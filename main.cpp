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
    //********�����ÿ���̨��С*********
    //********����ʼ��Socket**********
    WSADATA Wsd;
    if (WSAStartup(MAKEWORD(2, 2), &Wsd) != 0) {
        printf("��ʼ��Socketʧ��\n");
        return -1;
    }

    printf("�������ӷ����������Ե�...\n");
    if (connect2Server()) {
        startReceiving();
        welcome();
    }
    else
        printf("����ʧ��\n");
    //********����ʼ��Socket**********

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