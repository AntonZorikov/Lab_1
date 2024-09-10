#include "function_class.h"

Function::Function(std::string name_, int nestingLevel_, unsigned __int128 startTime_, unsigned __int128 endTime_){
    this->name = name_;
    this->nestingLevel = nestingLevel_;
    this->startTime = startTime_;
    this->endTime = endTime_;
}
