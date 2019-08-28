#include "Server.h"
#include <string>
#include <windows.h>
#include "config.h"
#include <thread>
#include <queue>
#include <iostream>

SOCKET server;
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
        closesocket(server);
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
                closesocket(server);
                onDisconnected();
                return;
            }

            while(que.size() >= sizeof(int) || packSize!=0) {
                if (packSize == 0) {
                    if (que.size() >= sizeof(int)) {
                        for (int i=0; i< sizeof(int); ++i) {
                            packSize <<= 8;
                            packSize |= que.front();
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
        else if (res == 0) {
            //断开
        } else {
            //错误
        }
    }

    delete[] buf;
}



void onReceive(neb::CJsonObject& json) {
    std::cout << json["content"].ToString() << '\n';
}

void onDisconnected() {

}

