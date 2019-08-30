#include "conio2.h"
#include "KeyEventHandler.h"


KeyEventHandler::KeyEventHandler(std::function<void(char)> lambda):lambda(lambda){}
void KeyEventHandler::start() {
    std::thread listener([this](){
        while(!this->shouldStop) {
            this->lambda(getch());
        }
    });
    listener.detach();
}

void KeyEventHandler::stop() {
    this->shouldStop = true;
}