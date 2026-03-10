#ifndef MESSAGE_PARSER_H
#define MESSAGE_PARSER_H

#include <Arduino.h>
class MessageParser
{
public:
    bool parse(const char *data, size_t len);
    String getCommand();
    int getParamCount();
    String getParam(int index);
    int getIntParam(int index);

private:
    String command;
    String params[10];
    int paramCount;
};
#endif // MESSAGE_PARSER_H