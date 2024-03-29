#include "Server.h"
#include "commonHeads.h"
#include "UIUtils.h"
#include "config.h"
#include "./JSONLib/CJsonObject.hpp"
#include <iostream>
#include "Game.h"
int main() {
    char cmd[100];
    sprintf(cmd, "mode con cols=%d lines=%d", width, height);
    system(cmd);
    SetConsoleTitle("联机五子棋");
    setCursorVisible(false);
    //********↑设置控制台属性*********

    readConfig();//读入配置文件

    //********↓初始化Socket**********
    WSADATA Wsd;
    if (WSAStartup(MAKEWORD(2, 2), &Wsd) != 0) {
        printf("初始化Socket失败\n");
        return -1;
    }

    printf("正在连接服务器，请稍等...\n");
    if (connect2Server()) {
        startReceiving();
        welcome();
    }
    else
        printf("连接失败\n");
    //********↑初始化Socket**********

    while(1) {
        if (!mainThreadFunctions.empty()) {
            void (*func)(void) = mainThreadFunctions.front();
            mainThreadFunctions.pop();
            func();
        }
        Sleep(10);
    }

    return 0;
}