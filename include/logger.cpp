#include "logger.h"
#include <iostream>
#include <map>
#include <ctime>
#include <iomanip>
#include <cstdio>
#include <experimental/filesystem>
#include <algorithm>
#include <thread>
#include <x86intrin.h>

namespace fs = std::experimental::filesystem;

std::string Logger::unsigned_int128_ToString(unsigned __int128 value){
    std::string res = "";
    if(value == 0){
        res = "0";
        return res;
    }

    while(value > 0){
        char digit = '0' + value % 10;
        res.insert(res.begin(), digit);
        value /= 10;
    }
    return res;
}

std::tm Logger::ns_to_tm(unsigned __int128 ns, unsigned __int128 &rem_ns){
    const unsigned __int128 NS_PER_SEC = 1000000000;
    std::time_t sec = ns / NS_PER_SEC;
    rem_ns = ns % NS_PER_SEC;

    std::tm tm = *std::localtime(&sec);

    return tm;
}

std::string Logger::format_tm(const std::tm &tm, unsigned __int128 rem_ns){
    std::ostringstream oss;
    oss << std::put_time(&tm, "%Y-%m-%d %H:%M:%S");
    oss << "." <<std::setw(9) << std::setfill('0') << unsigned_int128_ToString(rem_ns);
    return oss.str();
}

std::string Logger::logLevelToString(LogLevel level){
    switch (level) {
        case INFO: return "INFO";
        case WARNING: return "WARNING";
        case DEBUG: return "DEBUG";
        case ERROR: return "ERROR";
        default: return "UNKNOWN";;
    }
}

std::string Logger::functionStateToString(FunctionState state){
    switch (state) {
        case START_FUNCTION: return "START_FUNCTION";
        case END_FUNCTION: return "END_FUNCTION";
        default: return "UNKNOWN";;
    }
}

void Logger::checkMemStatus(){
    if(logs >= MAX_NUM_OF_LOGS || size >= MAX_AMOUNT_OF_MEM_USED){
        writeBinary();
    }
}

void Logger::log(LogLevel type, std::string message){

    std::lock_guard<std::mutex> lock_mtx(mtx);

    size_t dictSize = dictionary.size();

    if(dictionary.find(message) == dictionary.end()){
        dictionary[message] = dictSize;
    }

    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    unsigned __int128 ns = std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count();

    Log rec(ns,std::this_thread::get_id(), type, dictionary[message]);
    records.push_back(rec);

    logs++;
    size += sizeof(rec) + sizeof(int) + message.capacity();
    //size += 52 + message.capacity();

    checkMemStatus();
}

void Logger::log(FunctionState state, std::string message){
    std::lock_guard<std::mutex> lock_mtx(mtx);

//    auto start_ = std::chrono::high_resolution_clock::now();

    size_t dictSize = dictionary.size();

    if(dictionary.find(message) == dictionary.end()){
        dictionary[message] = dictSize;
    }

    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    unsigned __int128 ns = std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count();

    Log rec(ns, state, dictionary[message], std::this_thread::get_id());
    records.push_back(rec);

    logs++;
    size += sizeof(rec) + sizeof(int) + message.capacity();
    //size += 52 + message.capacity();

//    auto end_ = std::chrono::high_resolution_clock::now();

//    std::cout << std::chrono::duration_cast<std::chrono::nanoseconds>(end_ - start_).count() << std::endl;
    checkMemStatus();
}


//void Logger::log(LogLevel type, std::string message){
//    std::stringstream str;

//    std::lock_guard<std::mutex> lock_mtx(mtx);

//    size_t dictSize = dictionary.size();

//    auto start_ = std::chrono::high_resolution_clock::now();

//    if(!dictionary[message]){
//        dictionary[message] = dictSize;
//    }

//    auto end_ = std::chrono::high_resolution_clock::now();

//    str << "DICT: " << std::chrono::duration_cast<std::chrono::nanoseconds>(end_ - start_).count() << " ";

//    auto start_1 = std::chrono::high_resolution_clock::now();

//    std::chrono::time_point now = std::chrono::system_clock::now();
//    auto duration = now.time_since_epoch();
//    unsigned __int128 ns = std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count();

//    auto end_1 = std::chrono::high_resolution_clock::now();

//    str << " TIME: " << std::chrono::duration_cast<std::chrono::nanoseconds>(end_1 - start_1).count() << " ";

//    auto start_2 = std::chrono::high_resolution_clock::now();

//    Log rec(ns, type, dictSize);
//    records.push_back(rec);

//    auto end_2 = std::chrono::high_resolution_clock::now();
//    str << " REC: " << std::chrono::duration_cast<std::chrono::nanoseconds>(end_2 - start_2).count() << " ";


//    auto start_3 = std::chrono::high_resolution_clock::now();

//    logs++;
//    //size += sizeof(rec) + sizeof(int) + message.capacity();
//    size += 48 + 4 + message.capacity();

//    auto end_3 = std::chrono::high_resolution_clock::now();
//    str << " LOG&SIZE: " << std::chrono::duration_cast<std::chrono::nanoseconds>(end_3 - start_3).count() << " ";

//    checkMemStatus();

//    std::cout << str.str() << std::endl;

//}

void Logger::log(int val){
    auto start_ = std::chrono::high_resolution_clock::now();

    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    unsigned __int128 ns = std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count();

    Log rec(ns, val);
    std::lock_guard<std::mutex> lock(mtx);
    records.push_back(rec);

    logs++;
    size += sizeof(rec);

    auto end_ = std::chrono::high_resolution_clock::now();
    //std::cout << "I " << std::chrono::duration_cast<std::chrono::nanoseconds>(end_ - start_).count() << std::endl;

    checkMemStatus();
}

void Logger::log(float val){
    auto start_ = std::chrono::high_resolution_clock::now();

    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    unsigned __int128 ns = std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count();

    Log rec(ns, val);
    std::lock_guard<std::mutex> lock(mtx);
    records.push_back(rec);

    logs++;
    size += sizeof(rec);

    auto end_ = std::chrono::high_resolution_clock::now();
    //std::cout << "F " << std::chrono::duration_cast<std::chrono::nanoseconds>(end_ - start_).count() << std::endl;

    checkMemStatus();
}

void Logger::log(long val){
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    unsigned __int128 ns = std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count();

    Log rec(ns, val);
    std::lock_guard<std::mutex> lock(mtx);
    records.push_back(rec);

    logs++;
    size += sizeof(rec);

    checkMemStatus();
}

void Logger::readLog(std::string dir, QString fp, std::unordered_map<int, std::string> &dict,  std::vector<Log> &recs){
    QDir qdir(dir.c_str());

    static int st = 0, en = 0;

    std::ifstream file(dir + "/" + fp.toUtf8().constData(), std::ios::ate | std::ios::binary);

    if(!file){
        throw std::runtime_error("Failed to open file");
    }

    file.seekg(0, std::ios::beg);
    size_t dictSize;
    file.read(reinterpret_cast<char*>(&dictSize), sizeof(dictSize));

    for(size_t i = 0; i < dictSize; i++){

        int value;
        size_t strSize;
        file.read(reinterpret_cast<char*>(&strSize), sizeof(strSize));

        std::string key(strSize, ' ');

        file.read(&key[0], strSize);
        file.read(reinterpret_cast<char*>(&value), sizeof(value));

        dict[value] = key;
        //std::cout << strSize << " | " << key << " | " << value << std::endl;
    }

    size_t vecSize;
    file.read(reinterpret_cast<char*>(&vecSize), sizeof(vecSize));

    for(int i = 0; i < vecSize; i++){
        float val_f;
        unsigned __int128 a;
        long int b, c, val_l;
        int val_type, val_i;
        std::thread::id threadId;

        file.read(reinterpret_cast<char*>(&val_type), sizeof(long int));
        file.read(reinterpret_cast<char*>(&a), sizeof(a));
        unsigned __int128 rem_ns;
        std::tm tm = ns_to_tm(a, rem_ns);
        std::string form_time = format_tm(tm, rem_ns);

        if(val_type == 0){
            file.read(reinterpret_cast<char*>(&b), sizeof(b));
            file.read(reinterpret_cast<char*>(&c), sizeof(c));
            file.read(reinterpret_cast<char*>(&threadId), sizeof(std::thread::id));

            Log rec(a, threadId, b, c);
            rec.valueType = val_type;
            recs.push_back(rec);
        }
        else if(val_type == 1){
            file.read(reinterpret_cast<char*>(&val_i), sizeof(int));
            Log rec(a, val_i);
            rec.valueType = val_type;
            recs.push_back(rec);
        }
        else if(val_type == 2){
            file.read(reinterpret_cast<char*>(&val_f), sizeof(float));
            Log rec(a, val_f);
            rec.valueType = val_type;
            recs.push_back(rec);
        }
        else if(val_type == 3){
            file.read(reinterpret_cast<char*>(&val_l), sizeof(long int));
            Log rec(a, val_l);
            rec.valueType = val_type;
            recs.push_back(rec);
        }
        else if(val_type == 4){
            file.read(reinterpret_cast<char*>(&b), sizeof(b));
            file.read(reinterpret_cast<char*>(&c), sizeof(c));
            file.read(reinterpret_cast<char*>(&threadId), sizeof(std::thread::id));

            if(b == START_FUNCTION)
                st++;
            if(b == END_FUNCTION)
                en++;

            Log rec(a, b, c, threadId);
            rec.valueType = val_type;
            recs.push_back(rec);

            //std::cout << form_time << " # " << functionStateToString((FunctionState) b) << " # " << dict[c] << " # " << threadId << std::endl;
        }
    }

    if(!file){
        std::cerr << "Error: read data";
    }

    file.close();
}

void Logger::writeBinary(){
    std::lock_guard<std::mutex> lock_mtx(mtx);
    auto now = std::chrono::high_resolution_clock::now();
    auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(now.time_since_epoch()).count();
    out.open("logs/log_" + std::to_string(ns), std::ios::out | std::ios::binary);

    if(!out.is_open()){
        throw std::runtime_error("Failed to open file");
    }

    size_t mapSize = dictionary.size();
    out.write(reinterpret_cast<const char*>(&mapSize), sizeof(mapSize));

    for(const std::pair<std::string, int> &data : dictionary){
        size_t strLen = data.first.size();
        out.write(reinterpret_cast<const char*>(&strLen), sizeof(strLen));
        out.write(data.first.c_str(), data.first.size());
        out.write(reinterpret_cast<const char*>(&data.second), sizeof(data.second));
    }

    size_t vecSize = records.size();
    out.write(reinterpret_cast<const char*>(&vecSize), sizeof(vecSize));

    for(const auto &data : records){
        out.write(reinterpret_cast<const char*>(&data.valueType), sizeof(long int));
        if(data.valueType == 0){
            out.write(reinterpret_cast<const char*>(&data.timestamp), sizeof(unsigned __int128));
            out.write(reinterpret_cast<const char*>(&data.logType), sizeof(long int));
            out.write(reinterpret_cast<const char*>(&data.messageCode), sizeof(long int));
            out.write(reinterpret_cast<const char*>(&data.threadId), sizeof(std::thread::id));
        }
        else if(data.valueType == 1){
            out.write(reinterpret_cast<const char*>(&data.timestamp), sizeof(unsigned __int128));
            out.write(reinterpret_cast<const char*>(&data.value), sizeof(int));
        }
        else if(data.valueType == 2){
            out.write(reinterpret_cast<const char*>(&data.timestamp), sizeof(unsigned __int128));
            out.write(reinterpret_cast<const char*>(&data.value), sizeof(float));
        }
        else if(data.valueType == 3){
            out.write(reinterpret_cast<const char*>(&data.timestamp), sizeof(unsigned __int128));
            out.write(reinterpret_cast<const char*>(&data.value), sizeof(long int));
        }
        else if(data.valueType == 4){
            out.write(reinterpret_cast<const char*>(&data.timestamp), sizeof(unsigned __int128));
            out.write(reinterpret_cast<const char*>(&data.logType), sizeof(long int));
            out.write(reinterpret_cast<const char*>(&data.messageCode), sizeof(long int));
            out.write(reinterpret_cast<const char*>(&data.threadId), sizeof(std::thread::id));
        }
    }

    out.close();

    dictionary.clear();
    records.clear();

    size = sizeof(dictionary) + sizeof(records);
    logs = 0;
}

Logger::Logger(){
    dictionary.reserve(MAX_NUM_OF_LOGS);
    records.reserve(MAX_NUM_OF_LOGS);
    //dictionary.max_load_factor(0.25);
    QDir dir;
    dir.mkdir("./logs");

}

Logger::~Logger(){
    if(records.size() > 0){
        writeBinary();
    }
}
