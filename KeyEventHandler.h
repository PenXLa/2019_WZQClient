#ifndef WZQCLIENT_KEYEVENTHANDLER_H
#define WZQCLIENT_KEYEVENTHANDLER_H


#include <functional>
#include <thread>
class KeyEventHandler {
public:
    volatile bool shouldStop = false;
    std::function<void(char)> lambda;
    KeyEventHandler(std::function<void(char)> lambda);
    void start();
    void stop();
};


#endif //WZQCLIENT_KEYEVENTHANDLER_H
