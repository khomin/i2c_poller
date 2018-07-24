#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string.h>
#include <thread>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/signal.h>
#include <vector>
#include <thread>
#include <chrono>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/resource.h>
#include <sys/file.h>
#include <execinfo.h>
#include <wait.h>
#include <errno.h>

#include "../inc/I2cpoll.h"
#include "../inc/logger.h"

const std::string pidFileName = "/var/run/i2c_poller.pid";

static logger& loggerInstance = logger::Instance();
static I2cPoll *i2c_poller;
std::mutex mtx_thread_poll;
static std::vector<std::thread> threads;

const int CHILD_NEED_WORK = 1;
const int CHILD_NEED_TERMINATE = 2;
const int FD_LIMIT =  1024*10;

int monitorProc();
void setPidFile(char* Filename);
int workProc();
int setFdLimit(int maxFd);
int initWorkThread();
void destroyWorkThread();
static void signal_error(int sig, siginfo_t *si, void *ptr);


int main(int argc, char** argv)
{
	int status;
	int pid;

	loggerInstance.init();
	loggerInstance.appendToLog("[i2c_monitor] -Starting\r\n");

	// создаем потомка
	pid = fork();

	if (pid == -1) {
		// если не удалось запустить потомка
		// выведем на экран ошибку и её описание
		printf("Error: Start Daemon failed (%s)\n", strerror(errno));
		return -1;
	} else if (!pid) {
		// если это потомок
		// данный код уже выполняется в процессе потомка
		// разрешаем выставлять все  биты прав на создаваемые файлы,
		// иначе у нас могут быть проблемы с правами доступа
		umask(0);

		// создаём новый сеанс, чтобы не зависеть от родителя
		setsid();

		// переходим в корень диска, если мы этого не сделаем, то могут быть проблемы.
		// к примеру с размантированием дисков
		chdir("/");

		// Данная функция будет осуществлять слежение за процессом
		status = monitorProc();

		return status;
	} else {
		// если это родитель
		// завершим процес, т.к. основную свою задачу (запуск демона) мы выполнили
		return 0;
	}
}


int monitorProc() {
	int pid;
	int status;
	int is_need_start = 1;
	sigset_t sigset;
	siginfo_t siginfo;

	// настраиваем сигналы которые будем обрабатывать
	sigemptyset(&sigset);

	// сигнал остановки процесса пользователем
	sigaddset(&sigset, SIGQUIT);

	// сигнал для остановки процесса пользователем с терминала
	sigaddset(&sigset, SIGINT);

	// сигнал запроса завершения процесса
	sigaddset(&sigset, SIGTERM);

	// сигнал посылаемый при изменении статуса дочернего процесса
	sigaddset(&sigset, SIGCHLD);

	// пользовательский сигнал который мы будем использовать для обновления конфига
	sigaddset(&sigset, SIGUSR1);
	sigprocmask(SIG_BLOCK, &sigset, NULL);

	// данная функция создаст файл с нашим PID'ом
	setPidFile((char*)pidFileName.c_str());

	// бесконечный цикл работы
	for (;;)
	{
		// если необходимо создать потомка
		if (is_need_start) {
			// создаём потомка
			pid = fork();
		}

		is_need_start = 1;

		if (pid == -1) // если произошла ошибка
		{
			// запишем в лог сообщение об этом
			loggerInstance.appendToLog("[i2c_monitor] Fork failed (%s)\n", strerror(errno));
		}
		else if (!pid) // если мы потомок
		{
			// данный код выполняется в потомке
			loggerInstance.appendToLog("[i2c_monitor] Fork-children -OK\n");

			// запустим функцию отвечающую за работу демона
			status = workProc();

			// завершим процесс
			exit(status);
		}
		else // если мы родитель
		{
			// данный код выполняется в родителе
			loggerInstance.appendToLog("[i2c_monitor] Fork-parent -OK\n");

			// ожидаем поступление сигнала
			sigwaitinfo(&sigset, &siginfo);

			loggerInstance.appendToLog("[i2c_monitor] Fork-parent-signal[%d]!\n", siginfo.si_signo);

			// если пришел сигнал от потомка
			if (siginfo.si_signo == SIGCHLD)
			{
				// получаем статус завершение
				wait(&status);

				// преобразуем статус в нормальный вид
				status = WEXITSTATUS(status);

				// если потомок завершил работу с кодом говорящем о том, что нет нужды дальше работать
				if (status == CHILD_NEED_TERMINATE)
				{
					// запишем в лог сообщени об этом
					loggerInstance.appendToLog("[i2c_monitor] Child stopped\n");

					// прервем цикл
					break;
				}
				else if (status == CHILD_NEED_WORK) // если требуется перезапустить потомка
				{
					// запишем в лог данное событие
					loggerInstance.appendToLog("[i2c_monitor] Child restart\n");
				}
			}
			else if (siginfo.si_signo == SIGUSR1) // если пришел сигнал что необходимо перезагрузить конфиг
			{
				kill(pid, SIGUSR1); // перешлем его потомку
				is_need_start = 0; // установим флаг что нам не надо запускать потомка заново
			}
			else // если пришел какой-либо другой ожидаемый сигнал
			{
				// запишем в лог информацию о пришедшем сигнале
				loggerInstance.appendToLog("[i2c_monitor] Signal %s\n", strsignal(siginfo.si_signo));

				// убьем потомка
				kill(pid, SIGTERM);
				status = 0;
				break;
			}
		}
	}

	// запишем в лог, что мы остановились
	loggerInstance.appendToLog("[i2c_monitor] Stop\n");

	// удалим файл с PID'ом
	unlink(pidFileName.c_str());

	return status;
}

void setPidFile(char* Filename)
{
	FILE* f;

	f = fopen(Filename, "w+");
	if (f)
	{
		fprintf(f, "%u", getpid());
		fclose(f);
	}
}

int workProc()
{
	struct sigaction sigact;
	sigset_t         sigset;
	int             signo;
	int             status;

	// сигналы об ошибках в программе будут обрататывать более тщательно
	// указываем что хотим получать расширенную информацию об ошибках
	sigact.sa_flags = SA_SIGINFO;
	// задаем функцию обработчик сигналов
	sigact.sa_sigaction = signal_error;

	sigemptyset(&sigact.sa_mask);

	// установим наш обработчик на сигналы

	sigaction(SIGFPE, &sigact, 0); // ошибка FPU
	sigaction(SIGILL, &sigact, 0); // ошибочная инструкция
	sigaction(SIGSEGV, &sigact, 0); // ошибка доступа к памяти
	sigaction(SIGBUS, &sigact, 0); // ошибка шины, при обращении к физической памяти

	sigemptyset(&sigset);

	// блокируем сигналы которые будем ожидать
	// сигнал остановки процесса пользователем
	sigaddset(&sigset, SIGQUIT);

	// сигнал для остановки процесса пользователем с терминала
	sigaddset(&sigset, SIGINT);

	// сигнал запроса завершения процесса
	sigaddset(&sigset, SIGTERM);

	// пользовательский сигнал который мы будем использовать для обновления конфига
	sigaddset(&sigset, SIGUSR1);
	sigprocmask(SIG_BLOCK, &sigset, NULL);

	// Установим максимальное кол-во дискрипторов которое можно открыть
	setFdLimit(FD_LIMIT);

	// запишем в лог, что наш демон стартовал
	loggerInstance.appendToLog("[DAEMON] Started\n");

	// запускаем все рабочие потоки
	status = initWorkThread();
	if (!status) {
		// цикл ожижания сообщений
		for (;;)
		{
			// ждем указанных сообщений
			sigwait(&sigset, &signo);
			// если какой-либо другой сигнал, то выйдим из цикла
			loggerInstance.appendToLog("[DAEMON] sigwait-end[%s]\n", strsignal(signo));
			break;
		}

		// остановим все рабочеи потоки и корректно закроем всё что надо
		destroyWorkThread();
	}
	else
	{
		loggerInstance.appendToLog("[DAEMON] Create work thread failed\n");
	}

	loggerInstance.appendToLog("[DAEMON] Stopped\n");

	// вернем код не требующим перезапуска
	return CHILD_NEED_TERMINATE;
}

// функция для остановки потоков и освобождения ресурсов
void destroyWorkThread() {
	loggerInstance.appendToLog("[DAEMON] destroyd-OK\n");
	threads.~vector();
}

static void signal_error(int sig, siginfo_t *si, void *ptr)
{
	void* ErrorAddr;
	void* Trace[16];
	int    x;
	int    TraceSize;
	char** Messages;

	loggerInstance.appendToLog("[DAEMON] Stopped\n");

	// остановим все рабочие потоки и корректно закроем всё что надо
	destroyWorkThread();

	// завершим процесс с кодом требующим перезапуска
	exit(CHILD_NEED_WORK);
}

int setFdLimit(int maxFd) {
	struct rlimit lim;
	int           status;

	// зададим текущий лимит на кол-во открытых дискриптеров
	lim.rlim_cur = maxFd;
	// зададим максимальный лимит на кол-во открытых дискриптеров
	lim.rlim_max = maxFd;

	// установим указанное кол-во
	status = setrlimit(RLIMIT_NOFILE, &lim);

	return status;
}

void thread_proc(void) {
	for(;;)
	{
		{
			std::lock_guard<std::mutex> lock(mtx_thread_poll);
			i2c_poller->pollExect();
		}
		std::this_thread::sleep_for(std::chrono::milliseconds(100));
	}
}

// функция которая инициализирует рабочие потоки
int initWorkThread() {
	loggerInstance.appendToLog("[DAEMON] Init...\r\n");

	i2c_poller = new I2cPoll();

	std::thread thr(thread_proc);
	threads.emplace_back(std::move(thr));

	loggerInstance.appendToLog("[DAEMON] Init -Ok\r\n");
	return 0;
}
