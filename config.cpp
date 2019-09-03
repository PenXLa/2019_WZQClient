#include <cstdio>
#include "config.h"
#include "JSONLib/CJsonObject.hpp"
#include <string>
const int width = 160;
const int height = 39;
const int half_w = width/2, half_h = height/2;
const unsigned char CTRL_KEY = 0XE0;
const unsigned char KEY_LEFT = 0X4B;
const unsigned char KEY_RIGHT = 0X4D;
const unsigned char KEY_DOWN = 0X50;
const unsigned char KEY_UP = 0X48;
const unsigned char KEY_ESC = 0X1B;

const unsigned int MAX_PASSWORD_LEN = 20;//最长密码长度
const unsigned int MAX_USERNAME_LEN = 10;//最长用户名长度
char SERVER_IP[20] = "127.0.0.1";



void readConfig() {
    FILE* fp = fopen("config.json", "r");
    char buf[1025];
    std::string json_str;
    if (fp) {
        while(1) {
            int res = fread(buf, 1, sizeof(buf)-1, fp);
            buf[res] = 0;
            if (res == 0) break;
            else json_str += buf;
        }
        fclose(fp);

        neb::CJsonObject json(json_str);
        if (!json.IsNull("serverIP")) {
            std::string ip;
            json.Get("serverIP", ip);
            strcpy(SERVER_IP, ip.c_str());
        }
    }


}