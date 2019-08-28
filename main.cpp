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
    //********�����ÿ���̨��С*********
    //********����ʼ��Socket**********
    WSADATA Wsd;
    if (WSAStartup(MAKEWORD(2, 2), &Wsd) != 0) {
        printf("��ʼ��Socketʧ��");
        return -1;
    }
    //********����ʼ��Socket**********
    //login();
    if (connect2Server())
        startReceiving();
    else
        printf("����ʧ��\n");



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