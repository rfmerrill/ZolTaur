#ifndef SERIALCONTROLLER_H
#define SERIALCONTROLLER_H

#include <string.h>

class SerialParser {
    public:
        inline static const String COMMAND_NONE = "";
        inline static const String QUERY_IS_READY = "?R";
        inline static const String COMMAND_HAND_HOME = "!HH";
        inline static const String COMMAND_HAND_WAVE = "!HW";
        inline static const String COMMAND_MOUTH_HOME = "!MH";
        inline static const String COMMAND_MOUTH_TALK = "!MW";

        String parse(char c) {
            // Ignore optional carriage return
            if (c == '\r') return COMMAND_NONE;
            
            if (c == '\n') {
                String request = buffer;
                buffer = "";
                if (request == QUERY_IS_READY || 
                    request == COMMAND_HAND_HOME ||
                    request == COMMAND_HAND_WAVE ||
                    request == COMMAND_MOUTH_HOME ||
                    request == COMMAND_MOUTH_TALK) {
                        return request;
                } else {
                    return COMMAND_NONE;
                }
            }
            buffer += c;
            return COMMAND_NONE;
        };

    private:
        String buffer;
};

#endif /* SERIALCONTROLLER_H*/