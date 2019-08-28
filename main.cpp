#include "commonHeads.h"
#include "UIUtils.h"
#include "config.h"
#include "./JSONLib/CJsonObject.hpp"
#include <iostream>
#include "Server.h"

int main() {
    char cmd[100];
    sprintf(cmd, "mode con cols=%d lines=%d", width, height);
    system(cmd);
    //********↑设置控制台大小*********
    //********↓初始化Socket**********
    WSADATA Wsd;
    if (WSAStartup(MAKEWORD(2, 2), &Wsd) != 0) {
        printf("初始化Socket失败");
        return -1;
    }
    //********↑初始化Socket**********
    //login();
    if (connect2Server())
        startReceiving();
    else
        printf("连接失败\n");



    string s;
    while(1) {
        cin >> s;
        neb::CJsonObject json;
        json.Add("type", "chat");
        json.Add("content", s);

        sendPack(json);


    }

    return 0;
}