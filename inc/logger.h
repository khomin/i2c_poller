#ifndef LOGGER_H_
#define LOGGER_H_

#include <string>

class logger {
public:
    static logger& Instance() {
            static logger instance;
            return instance;
    };
    void init();
    void appendToLog(std::string format, ...);
private:
    //конструктор и деструктор недоступны
    logger();
    virtual ~logger();

    //запрещаем копирование
    logger(logger const&) = delete;
    logger& operator= (logger const&) = delete;

    bool mkpath( std::string path );

    FILE* file;
    const std::string logFileDir    = "/var/log/i2c_poller/";
    std::string logFileName          = "i2c_poller_exe.log";
    std::string logPath;
};

#endif /* OGGER_H_ */
