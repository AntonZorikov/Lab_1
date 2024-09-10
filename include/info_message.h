#ifndef INFO_MESSAGE_H
#define INFO_MESSAGE_H

#include <logger.h>
#include <thread>

class InfoMessage
{
public:
    int logLevel;
    unsigned __int128 timestamp;
    std::string message;
    int nestingLevel;
    std::thread::id threadId;

    InfoMessage();

    InfoMessage(int _logLevel, unsigned __int128 _timestamp, std::string _message, int _nestingLevel, std::thread::id _threadId);
};

#endif // INFO_MESSAGE_H
