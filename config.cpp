#include "config.h"
const int width = 160;
const int height = 40;
const int half_w = width/2, half_h = height/2;
const unsigned char CTRL_KEY = 0XE0;
const unsigned char KEY_LEFT = 0X4B;
const unsigned char KEY_RIGHT = 0X4D;
const unsigned char KEY_DOWN = 0X50;
const unsigned char KEY_UP = 0X48;
const unsigned char KEY_ESC = 0X1B;

const unsigned int MAX_PASSWORD_LEN = 20;//最长密码长度
const unsigned int MAX_USERNAME_LEN = 10;//最长用户名长度
const char SERVER_IP[] = "127.0.0.1";