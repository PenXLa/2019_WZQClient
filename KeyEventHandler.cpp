#include "conio2.h"
#include "KeyEventHandler.h"
#include <windows.h>


KeyEventHandler::KeyEventHandler(std::function<void(char)> lambda):lambda(&lambda){}

void KeyEventHandler::start() {
    std::thread listener([this](){
        while(!this->shouldStop) {
            if (kbhit()) (*this->lambda)(getch());
            Sleep(10);
        }
        shouldStop = false;
    });
    listener.detach();
}

void KeyEventHandler::start(std::function<void(char)> lambda) {
    this->lambda = &lambda;
    start();
}

void KeyEventHandler::stop() {
    this->shouldStop = true;
}
