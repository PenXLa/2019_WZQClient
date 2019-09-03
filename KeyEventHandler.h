#ifndef WZQCLIENT_KEYEVENTHANDLER_H
#define WZQCLIENT_KEYEVENTHANDLER_H


#include <functional>
#include <thread>

class KeyEventHandler {
public:
    volatile bool shouldStop = false;
    std::function<void(char)> *lambda = nullptr;
    KeyEventHandler(){}
    KeyEventHandler(std::function<void(char)>);
    void start();
    void start(std::function<void(char)>);
    void stop();
};


#endif //WZQCLIENT_KEYEVENTHANDLER_H
