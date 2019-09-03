#include "Server.h"
#include <string>
#include <windows.h>
#include "config.h"
#include <queue>
#include <iostream>
#include "UIUtils.h"
#include <thread>
#include "Game.h"


SOCKET server;
neb::CJsonObject playerInfo;

bool connect2Server() {
    server = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    SOCKADDR_IN addr;

    // Ҫ���ӵĻ�����Ϣ
    addr.sin_family = AF_INET;
    addr.sin_port = htons(6666);
    addr.sin_addr.S_un.S_addr = inet_addr(SERVER_IP);

    int ret = connect(server, (struct sockaddr*)&addr, sizeof(addr));

    if (ret == SOCKET_ERROR)
    {
        WSACleanup();
        return false;
    }

    return true;
}


void startReceiving() {
    std::thread receiver([](){
        std::queue<char> que;
        char buf[1024];
        int packSize = 0;
        while(true) {
            int size = recv(server,buf,sizeof buf,0);
            if(size > 0){
                for (int i=0; i<size; ++i) que.emplace(buf[i]);
            } else {
                //���ӹر�
                onDisconnected();
                return;
            }

            while(que.size()>=sizeof(int) && packSize==0 || packSize!=0 && que.size()>=packSize) {
                if (packSize == 0) {
                    if (que.size() >= sizeof(int)) {
                        for (int i=0; i< sizeof(int); ++i) {
                            packSize <<= 8;
                            packSize |= (unsigned char)que.front();
                            que.pop();
                        }
                    }
                }
                if (packSize != 0) {
                    if (que.size()>=packSize) {
                        std::string str;
                        for (int i=0; i<packSize; ++i) {
                            str.push_back(que.front());
                            que.pop();
                        }
                        str.push_back(0);
                        auto json = neb::CJsonObject(str);

                        onReceive(json);

                        packSize=0;
                    }
                }
            }

        }
    });
    receiver.detach();
}


void sendPack(neb::CJsonObject &json) {
    std::string str = json.ToString();
    int len = str.length(), sum;

    char *buf = new char[len+sizeof(int)];
    for (int i= sizeof(int)-1, size = len; i>=0; --i) {
        buf[i] = size & 0xFF;
        size >>= 8;
    }
    for (int i=sizeof(int); i<len+sizeof(int); ++i) {
        buf[i] = str[i-sizeof(int)];
    }

    len += sizeof(int);
    sum = len;

    while(sum > 0) {
        int res = send(server, buf+len-sum, sum, 0);
        if (res > 0) sum-=res;
        else {
            MessageBoxA(NULL, "���ʹ���", "", MB_OK);
        }
    }

    delete[] buf;
}



void onReceive(neb::CJsonObject& json) {
    std::string type;
    json.Get("type", type);
    //MessageBox(NULL, json.ToFormattedString().c_str(), "", MB_OK);
    if (type == "loginResult") {
        int result;
        json.Get("result", result);
        if (result) {
            mainThreadFunctions.emplace(showMainMenu);//���̵߳���showMainMenu
        } else {
            MessageBoxA(nullptr, "��¼ʧ�ܣ������û���������ƴд�Ƿ���ȷ", "����������", MB_OK | MB_ICONWARNING);
            mainThreadFunctions.emplace(login);//���̵߳���
        }
    } else if (type == "registerResult") {
        int result;
        json.Get("result", result);
        if (result) {
            MessageBox(nullptr, "ע��ɹ�", "����������", MB_OK|MB_ICONINFORMATION);
            mainThreadFunctions.emplace(showMainMenu);//���̵߳���
        } else {
            std::string reason;
            json.Get("reason", reason);
            MessageBox(nullptr, reason.c_str(),"ע��ʧ��",MB_OK | MB_ICONWARNING);
            mainThreadFunctions.emplace(showRegister);//���̵߳���
        }
    } else if (type == "showPersonalInfo") {
        playerInfo = json;
        mainThreadFunctions.emplace(printPersonalInfo);//���̵߳���
    } else if (type == "changePersonalInfoResult") {
        int result;
        json.Get("result", result);
        if (result) {
            MessageBoxA(NULL, "�޸ĸ�����Ϣ�ɹ�", "����������", MB_OK|MB_ICONINFORMATION);
        } else {
            std::string reason;
            json.Get("reason", reason);
            MessageBoxA(NULL, reason.c_str(), "�޸ĸ�����Ϣʧ��", MB_OK|MB_ICONWARNING);
        }
    } else if (type == "changePasswordResult") {
        int result;
        json.Get("result", result);
        if (result) {
            MessageBoxA(NULL, "�޸�����ɹ�", "����������", MB_OK|MB_ICONINFORMATION);
        } else {
            std::string reason;
            json.Get("reason", reason);
            MessageBoxA(NULL, reason.c_str(), "�޸�����ʧ��", MB_OK|MB_ICONWARNING);
        }
    } else if (type == "startGame") {
        int color;
        json.Get("color", color);
        chessColor = color;
        if (color == 'b') {
            json.Get("binfo", myInfo);
            json.Get("winfo", oppInfo);
        } else {
            json.Get("winfo", myInfo);
            json.Get("binfo", oppInfo);
        }
        prepareGame();
        mainThreadFunctions.emplace(startGame);
    } else if (type == "yourTurn") {
        isMyTurn = true;
        printTurningInfo();
    } else if (type == "putChess") {
        int r,c,color;
        json.Get("row", r);
        json.Get("col", c);
        json.Get("color", color);
        putChess(r,c,color);
    } else if (type == "oppExited") {
        gameRunning = false;//gameRunning��Ϊfalse���ͻ�ֹͣ�������̣����߳�ֹͣ����
        MessageBoxA(NULL, "�Է������˳�����Ϸ���˾���ȡ����ʤ��", "����������", MB_OK);
        mainThreadFunctions.emplace(showMainMenu);
    } else if (type == "win") {
        int winr, winc, wind, who;
        json.Get("who", who);
        json.Get("cenR", winr);
        json.Get("cenC", winc);
        json.Get("dir", wind);
        gameRunning = false;

        std::string msg;
        if (chessColor == who) {
            msg = "��ϲ��ȡ����ʤ����\n��ȷ���˳���Ϸ";
        } else {
            msg = "���ź������ˣ��´�Ŭ��Ŷ\n��ȷ���˳���Ϸ";
        }
        MessageBoxA(NULL, msg.c_str(), "����������", MB_OK|MB_ICONINFORMATION);
        mainThreadFunctions.emplace(showMainMenu);
    }
}

void onDisconnected() {
    std::cout << "Server shutdown.\n";
}

