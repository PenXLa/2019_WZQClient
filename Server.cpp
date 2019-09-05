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

    // 要连接的基础信息
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
                //连接关闭
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
            MessageBoxA(NULL, "发送错误", "", MB_OK);
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
            mainThreadFunctions.emplace(showMainMenu);//主线程调用showMainMenu
        } else {
            std::string reason;
            json.Get("reason", reason);
            MessageBoxA(nullptr, reason.c_str(), "登录失败", MB_OK | MB_ICONWARNING);
            mainThreadFunctions.emplace(login);//主线程调用
        }
    } else if (type == "registerResult") {
        int result;
        json.Get("result", result);
        if (result) {
            MessageBox(nullptr, "注册成功", "联机五子棋", MB_OK|MB_ICONINFORMATION);
            mainThreadFunctions.emplace(showMainMenu);//主线程调用
        } else {
            std::string reason;
            json.Get("reason", reason);
            MessageBox(nullptr, reason.c_str(),"注册失败",MB_OK | MB_ICONWARNING);
            mainThreadFunctions.emplace(showRegister);//主线程调用
        }
    } else if (type == "showPersonalInfo") {
        playerInfo = json;
        mainThreadFunctions.emplace(printPersonalInfo);//主线程调用
    } else if (type == "changePersonalInfoResult") {
        int result;
        json.Get("result", result);
        if (result) {
            MessageBoxA(NULL, "修改个人信息成功", "联机五子棋", MB_OK|MB_ICONINFORMATION);
        } else {
            std::string reason;
            json.Get("reason", reason);
            MessageBoxA(NULL, reason.c_str(), "修改个人信息失败", MB_OK|MB_ICONWARNING);
        }
    } else if (type == "changePasswordResult") {
        int result;
        json.Get("result", result);
        if (result) {
            MessageBoxA(NULL, "修改密码成功", "联机五子棋", MB_OK|MB_ICONINFORMATION);
        } else {
            std::string reason;
            json.Get("reason", reason);
            MessageBoxA(NULL, reason.c_str(), "修改密码失败", MB_OK|MB_ICONWARNING);
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
        gameRunning = false;//gameRunning设为false，就会停止监听键盘，主线程停止阻塞
        MessageBoxA(NULL, "对方主动退出了游戏，此局您取得了胜利", "联机五子棋", MB_OK|MB_ICONINFORMATION);
        mainThreadFunctions.emplace(showMainMenu);
    } else if (type == "win") {
        int winr, winc, wind, who, winType;
        json.Get("who", who);
        json.Get("cenR", winr);
        json.Get("cenC", winc);
        json.Get("dir", wind);
        json.Get("winType", winType);
        gameRunning = false;

        std::string msg;

        if (winType==1) {
            if (chessColor == who) msg = "恭喜您取得了胜利！\n\n按确定返回主菜单";
            else msg = "很遗憾您输了，下次努力哦\n\n按确定返回主菜单";
            int dirs[4][5][2] = {{{-2,-2}, {-1,-1}, {0,0}, {1,1}, {2,2}},
                                    {{-2,2}, {-1,1}, {0,0}, {1,-1}, {2,-2}},
                                    {{0,-2}, {0,-1}, {0,0}, {0,1}, {0,2}},
                                    {{-2,0}, {-1,0}, {0,0}, {1,0}, {2,0}}};

            for (int i=0; i<5; ++i) {
                mat[winr+dirs[wind][i][0]][winc+dirs[wind][i][1]] = 'r';
                printGrid(CHESSBOARD_HEIGHT, CHESSBOARD_WIDTH, winr+dirs[wind][i][0], winc+dirs[wind][i][1]);
            }
        } else if (winType == 2) {
            if (chessColor == who) msg = "对方未能在规定时间内落子，此局您取得了胜利\n\n按确定返回主菜单";
            else msg = "您未能在规定时间内落子，您输了\n\n按确定返回主菜单";
        }

        MessageBox(NULL, msg.c_str(), "联机五子棋", MB_OK|MB_ICONINFORMATION);
        mainThreadFunctions.emplace(showMainMenu);
    } else if (type == "countdown") {
        if (gameRunning) {
            int timeLeft;
            json.Get("timeLeft", timeLeft);
            printGamingCountdown(timeLeft);
        }
    } else if (type == "offline") {
        std::string reason;
        std::string str = "您已被强制下线，因为\n";
        json.Get("reason", reason);
        str += reason;
        str += "\n点击确定退出游戏";
        MessageBox(NULL, str.c_str(), "联机五子棋", MB_OK | MB_ICONWARNING);
        exit(0);
    }
}

void onDisconnected() {
    MessageBox(NULL, "与服务器的连接断开，请确认网络连接", "联机五子棋", MB_OK|MB_ICONWARNING);
    exit(0);
}

