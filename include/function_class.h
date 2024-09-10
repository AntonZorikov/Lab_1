#ifndef FUNCTION_CLASS_H
#define FUNCTION_CLASS_H

#include<string>

class Function{
public:
    std::string name;
    int nestingLevel;
    unsigned __int128 startTime;
    unsigned __int128 endTime;

    Function(std::string name_, int nestingLevel_, unsigned __int128 startTime_, unsigned __int128 endTime_);
};

#endif // FUNCTION_CLASS_H
