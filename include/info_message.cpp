#include "info_message.h"

InfoMessage::InfoMessage()
{

}

InfoMessage::InfoMessage(int _logLevel, unsigned __int128 _timestamp, std::string _message, int _nestingLevel, std::thread::id _threadId){
    this->logLevel = _logLevel;
    this->timestamp = _timestamp;
    this->message = _message;
    this->nestingLevel = _nestingLevel;
    this->threadId = _threadId;
}
