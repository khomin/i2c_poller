#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <string.h>
#include <libpq-fe.h>
#include <thread>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/signal.h>

#include "I2cpoll.h"
//#include "DbConnection.h"
//#include "I2cpoll.h"
#include "logger.h"

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

//#include <pthread.h>

#define USE_DEBUG       1

// лимит для установки максимально кол-во открытых дискрипторов
const int FD_LIMIT =  1024*10;
//// константы для кодов завершения процесса
const int CHILD_NEED_WORK = 1;
const int CHILD_NEED_TERMINATE = 2;

const std::string pidFileName = "/var/run/sernsorI2cPoll.pid";
//static PGConnection *pgConnection;
static std::vector<std::thread> threads {};
//static std::vector<Sensor*> sensors;
static logger& loggerInstance = logger::Instance();
//static GpsMonitor *gpsMonitor;
//static Wialon *wialonPorotocol;
//static Settings *settings;

// функция для остановки потоков и освобождения ресурсов
void destroyWorkThread();
// функция которая инициализирует рабочие потоки
int initWorkThread();
// функция обработки сигналов
static void signal_error(int sig, siginfo_t *si, void *ptr);
// функция установки максимального кол-во дескрипторов которое может быть открыто
int setFdLimit(int maxFd);
int workProc();
void setPidFile(std::string filename);
int i2cProc();


#ifdef USE_DEBUG

void thread_func(){
    I2c_poll *i2cPoll = new I2c_poll();
    while(1) {
        i2cPoll->pollExect();
        sleep(1);
        fprintf(stdout, "threat_run\r\n");
    };
}

int main(int argc, char** argv) {
    fprintf(stdout, "Threat create\r\n");
    std::thread t1(thread_func);
    t1.join();
    return 0;
    
//    DbConnection::S_insertData device_data;    
//    DbConnection * db = new DbConnection();
//    
//    if(db->isConnecting()) {
//        fprintf(stdout, "is connected\r\n");
//        
//        device_data.device_type = i2cDevices::dev_i2c_tmp112;
//        device_data.parameter.temp.temp = 25.6;        
//        db->insertData(device_data);
//        device_data.parameter.temp.temp += 1;
//        
//        device_data.device_type = i2cDevices::dev_i2c_lis3dh;
//        device_data.parameter.accel.isInited = true;
//        device_data.parameter.accel.x = 1;
//        device_data.parameter.accel.y = 2;
//        device_data.parameter.accel.z = 3;
//        device_data.parameter.accel.velocityX = 4;
//        device_data.parameter.accel.velocityY = 5;
//        device_data.parameter.accel.velocityZ = 6;
//        db->insertData(device_data);
//        
//        device_data.device_type = i2cDevices::dev_i2c_txs02324;
//        device_data.parameter.sim_switcher.cardPowerIsUp = true;
//        device_data.parameter.sim_switcher.currentSimSlot = 1;
//        db->insertData(device_data);
//        
//        device_data.device_type = i2cDevices::dev_i2c_ina260;
//        device_data.parameter.power_monitor.currentPower = 10;
//        device_data.parameter.power_monitor.currnetFlowing = 20;
//        device_data.parameter.power_monitor.voltage = 24;        
//        db->insertData(device_data);
//    } else {
//        fprintf(stdout, "no connected\r\n");
//    }
}
#else
int main(int argc, char **argv)
{
    int pid;
    int status;
    
    // при не правильном запускке покажем как использовать демона
    if (argc != 1) {
        printf("Usage: ./i2c\r\n");
        return -1;
    }
    //Инициализация логгера
    loggerInstance.init("i2c task.log");
    loggerInstance.appendToLog("[I2C] Starting I2C\r\n");
    // создаем потомка
    pid = fork();

    if (pid == -1) { // если не удалось запустить потомка
        // выведем на экран ошибку и её описание
        printf("Start Daemon Error: %s\r\n", strerror(errno));
        return -1;
    } else if (!pid) {// если это потомок
        printf("Daemon started\r\n");
        
        // данный код уже выполняется в процессе потомка
        // разрешаем выставлять все биты прав на создаваемые файлы,
        // иначе у нас могут быть проблемы с правами доступа
        umask(0);

        // создаём новый сеанс, чтобы не зависеть от родителя
        setsid();

        // переходим в корень диска, если мы этого не сделаем, то могут быть проблемы.
        // к примеру с размантированием дисков
        chdir("/");

        // закрываем дискрипторы ввода/вывода/ошибок, так как нам они больше не понадобятся
        close(STDIN_FILENO);
        close(STDOUT_FILENO);
        close(STDERR_FILENO);

        // Данная функция будет осуществлять слежение за процессом
        status = i2cProc();

        return status;
    } else {// если это родитель
        // завершим процес, т.к. основную свою задачу (запуск демона) мы выполнили
        return 0;
    }
}
#endif

//    DbConnection::S_insertData device_data;
//    
//    DbConnection * db = new DbConnection();
//    
//    while(1) {
//        fprintf(stdout, "while: -");
//        
//        fork();
////        sleep(1);
//        
//        if(db->isConnecting()) {
//            fprintf(stdout, "while: is connecting");
//        } else {
//            fprintf(stdout, "while: -");
//        }
//    }
//    return 0;


//Функция опроса датчика
//void sensorMonitorThreadFunction(Sensor* sensor, PGConnection *pgConnection) {
//	bool result;
//
//	while (1){
//
//		result = sensor->requestData();
//		if (result) {
//			//sensor->print();
//			std::string sensorData = sensor->getSensorData();
//			pgConnection->insertSensorData(sensor->getId(), sensorData);
//		}
//		std::this_thread::sleep_for(std::chrono::milliseconds(settings->getSensorsRequestDelay()));
//	}
//}


// функция для остановки потоков и освобождения ресурсов
void destroyWorkThread() {

//    if (settings->getGpsIsEnabled()) {
//        delete gpsMonitor;
//    }
//    delete settings;
//    delete pgConnection;
}

// функция которая инициализирует рабочие потоки
int initWorkThread() {
//	pgConnection = new PGConnection();
//
//	settings = pgConnection->readSettings();
//
//	if (settings->getGpsIsEnabled()) {
//		gpsMonitor = new GpsMonitor(pgConnection, settings->getGpsRequestDelay());
//	}
	return 0;
}


// функция обработки сигналов
static void signal_error(int sig, siginfo_t *si, void *ptr) {
//	void*  ErrorAddr;
//	void*  Trace[16];
//	int    x;
//	int    TraceSize;
//	char** Messages;

	// запишем в лог что за сигнал пришел
	loggerInstance.appendToLog("[DAEMON] Signal: %s, Addr: %p\r\n", strsignal(sig), si->si_addr);

    //TODO ErrorAddr для arm
//	#if __WORDSIZE == 64 // если дело имеем с 64 битной ОС
//			// получим адрес инструкции которая вызвала ошибку
//			ErrorAddr = (void*)((ucontext_t*)ptr)->uc_mcontext.gregs[REG_RIP];
//	#else
//			// получим адрес инструкции которая вызвала ошибку
//			ErrorAddr = (void*)((ucontext_t*)ptr)->uc_mcontext.gregs[REG_EIP];
//	#endif
//
//	// произведем backtrace чтобы получить весь стек вызовов
//	TraceSize = backtrace(Trace, 16);
//	Trace[1] = ErrorAddr;
//
//	// получим расшифровку трасировки
//	Messages = backtrace_symbols(Trace, TraceSize);
//	if (Messages) {
//		loggerInstance.appendToLog("== Backtrace ==\r\n");
//
//			// запишем в лог
//			for (x = 1; x < TraceSize; x++)
//			{
//				loggerInstance.appendToLog("%s\r\n", Messages[x]);
//			}
//
//			loggerInstance.appendToLog("== End Backtrace ==\r\n");
//			free(Messages);
//	}

	loggerInstance.appendToLog("[DAEMON] Stopped\r\n");

	// остановим все рабочие потоки и корректно закроем всё что надо
	destroyWorkThread();

	// завершим процесс с кодом требующим перезапуска
	exit(CHILD_NEED_WORK);
}


// функция установки максимального кол-во дескрипторов которое может быть открыто
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

int workProc() {
	struct sigaction sigact;
	sigset_t         sigset;
	int              signo;
	int              status;

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

	sigprocmask(SIG_BLOCK, &sigset, NULL);

	// Установим максимальное кол-во дискрипторов которое можно открыть
	setFdLimit(FD_LIMIT);

	// запишем в лог, что наш демон стартовал
	loggerInstance.appendToLog("[DAEMON] Started\r\n");

	// запускаем все рабочие потоки
	status = initWorkThread();
	if (!status) {
		// цикл ожижания сообщений
		for (;;) {
			// ждем указанных сообщений
			sigwait(&sigset, &signo);

			// если какой-сигнал, то выйдим из цикла
			break;
		}

		// остановим все рабочеи потоки и корректно закроем всё что надо
		destroyWorkThread();
	} else {
		loggerInstance.appendToLog("[DAEMON] Create work thread failed\r\n");
	}

	loggerInstance.appendToLog("[DAEMON] Stopped\r\n");

	// вернем код не требующим перезапуска
	return CHILD_NEED_TERMINATE;
}


void setPidFile(std::string filename) {
	FILE* f;

	f = fopen(filename.c_str(), "w+");
	if (f) {
			fprintf(f, "%u", getpid());
			fclose(f);
	}
}


int i2cProc() {
	int       pid;
	int       status;
	int       need_start = 1;
	sigset_t  sigset;
	siginfo_t siginfo;

	// настраиваем сигналы которые будем обрабатывать
	sigemptyset(&sigset);

	// сигнал остановки процесса пользователем
	sigaddset(&sigset, SIGQUIT);

	// сигнал для остановки процесса пользователем с терминала
	sigaddset(&sigset, SIGINT);

	// сигнал запроса завершения процесса
	sigaddset(&sigset, SIGTERM);

	// сигнал посылаемый при изменении статуса дочернего процесс
	sigaddset(&sigset, SIGCHLD);

	// сигнал посылаемый при изменении статуса дочернего процесс
	sigaddset(&sigset, SIGCHLD);

	sigprocmask(SIG_BLOCK, &sigset, NULL);

	// данная функция создат файл с нашим PID'ом
	setPidFile(pidFileName.c_str());

	// бесконечный цикл работы
	for (;;) {
		// если необходимо создать потомка
		if (need_start) {
                    // создаём потомка
                    pid = fork();
		}

		need_start = 1;

		if (pid == -1) { // если произошла ошибка
			// запишем в лог сообщение об этом
			loggerInstance.appendToLog("[I2C_POLLER] Fork failed (%s)\r\n", strerror(errno));
		} else if (!pid) { // если мы потомок
			// данный код выполняется в потомке

			// запустим функцию отвечающую за работу демона
			status = workProc();

			// завершим процесс
			exit(status);
		} else { // если мы родитель
			// данный код выполняется в родителе

			// ожидаем поступление сигнала
			sigwaitinfo(&sigset, &siginfo);

			// если пришел сигнал от потомка
			if (siginfo.si_signo == SIGCHLD) {
				// получаем статус завершение
				wait(&status);

				// преобразуем статус в нормальный вид
				status = WEXITSTATUS(status);

				 // если потомок завершил работу с кодом говорящем о том, что нет нужны дальше работать
				if (status == CHILD_NEED_TERMINATE) {
					// запишем в лог сообщени об этом
					loggerInstance.appendToLog("[I2C_POLLER] Childer stopped\r\n");

					// прервем цикл
					break;
				} else if (status == CHILD_NEED_WORK) { // если требуется перезапустить потомка
					// запишем в лог данное событие
					loggerInstance.appendToLog("[I2C_POLLER] Childer restart\r\n");
				}
			} else if (siginfo.si_signo == SIGUSR1) { // если пришел сигнал что необходимо перезагрузить конфиг
				kill(pid, SIGUSR1); // перешлем его потомку
				need_start = 0; // установим флаг что нам не надо запускать потомка заново
			} else { // если пришел какой-либо другой ожидаемый сигнал
				// запишем в лог информацию о пришедшем сигнале
				loggerInstance.appendToLog("[I2C_POLLER] Signal %s\r\n", strsignal(siginfo.si_signo));

				// убьем потомка
				kill(pid, SIGTERM);
				status = 0;
				break;
			}
		}
	}

	// запишем в лог, что мы остановились
	loggerInstance.appendToLog("[I2C_POLLER] Stopped\r\n");

	// удалим файл с PID'ом
	unlink(pidFileName.c_str());

	return status;
}
