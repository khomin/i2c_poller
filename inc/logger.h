#ifndef LOGGER_H_
#define LOGGER_H_

#include <string>
#include <mutex>

class logger {
public:
    static logger& Instance(){
            // согласно стандарту, этот код ленивый и потокобезопасный
            static logger instance;
            return instance;
    };
    void init(const std::string fileName);
    void appendToLog(const std::string format, ...);
private:
    //конструктор и деструктор недоступны
    logger();
    virtual ~logger();

    //запрещаем копирование
    logger(logger const&) = delete;
    logger& operator= (logger const&) = delete;

    bool mkpath( std::string path );

    std::mutex lockMutex;
    FILE* file;
    const std::string logFileDir    = "/var/log/i2c/";
    std::string logFileName          = "i2c.log";
    std::string logPath;
};

#endif /* OGGER_H_ */