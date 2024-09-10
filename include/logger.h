#ifndef LOGGER_H
#define LOGGER_H

#include <fstream>
#include <mutex>
#include <sstream>
#include <fstream>
#include <vector>
#include <map>
#include <unordered_map>
#include "variant.hpp"
#include <thread>
#include <iostream>
#include "flat_hash_map.hpp"

#include <QDir>
#include <QListWidget>


#define MAX_AMOUNT_OF_MEM_USED 10000000
#define MAX_NUM_OF_LOGS 10000

enum LogLevel{
    INFO,
    WARNING,
    DEBUG,
    ERROR,
};

enum FunctionState{
    START_FUNCTION,
    END_FUNCTION
};

struct alignas (32) Log{
    unsigned __int128 timestamp;

    int logType;

    int messageCode;

    int valueType;

    mpark::variant<int, float, long> value;

    std::thread::id threadId;

    Log(size_t _timestamp, std::thread::id threadId_, int _type, int _message){
        this->timestamp = _timestamp;
        this->logType = _type;
        this->messageCode = _message;
        this->threadId = threadId_;

        valueType = 0;
    }

    Log(size_t _timestamp, int _functionState ,int _message, std::thread::id threadId_){
        this->logType = _functionState;
        this->timestamp = _timestamp;
        this->messageCode = _message;
        this->threadId = threadId_;
        valueType = 4;
    }

    Log(size_t _timestamp, int _val){
        this->timestamp = _timestamp;
        valueType = 1;
        value = _val;
    }

    Log(size_t _timestamp, float _val){
        this->timestamp = _timestamp;
        valueType = 2;
        value = _val;
    }

    Log(size_t _timestamp, long _val){
        this->timestamp = _timestamp;
        valueType = 3;
        value = _val;
    }
};

std::string unsigned_int128_ToString(unsigned __int128 value);

class Logger{
private:
    std::string logfile;

    int logs = 0;

    std::mutex mtx;

    size_t size = sizeof(dictionary) + sizeof(records);

    std::ofstream out;

    void openFile();

    std::vector<Log> records;

    ska::flat_hash_map<std::string, int> dictionary;

    void checkMemStatus();

public:

    void log(LogLevel type, std::string message);

    void log(FunctionState state, std::string message);

    void log(int val);

    void log(float val);

    void log(long val);

    static void readLog(std::string dir, QString fp, std::unordered_map<int, std::string> &dict,  std::vector<Log> &recs);

    static std::string logLevelToString(LogLevel level);

    static std::string functionStateToString(FunctionState state);

    static std::string format_tm(const std::tm &tm, unsigned __int128 rem_ns);

    static std::tm ns_to_tm(unsigned __int128 ns, unsigned __int128 &rem_ns);

    static std::string unsigned_int128_ToString(unsigned __int128 value);

    void writeBinary();

    Logger();

    ~Logger();

};

#endif // LOGGER_H
