#ifndef _CONFIG_
#define _CONFIG_

extern const int width;
extern const int height;
extern const int half_w, half_h;
extern const unsigned char CTRL_KEY;
extern const unsigned char KEY_LEFT;
extern const unsigned char KEY_RIGHT;
extern const unsigned char KEY_DOWN;
extern const unsigned char KEY_UP;
extern const unsigned char KEY_ESC;

extern const unsigned int MAX_PASSWORD_LEN;//最长密码长度
extern const unsigned int MAX_USERNAME_LEN;//最长用户名长度
extern char SERVER_IP[20];
extern const int MAX_DESCRIPTION_LEN;//最长个性签名长度
extern const int MAX_FRIENDCODE_LEN;//匹配暗号最大长度

void readConfig();

#define CHESSBOARD_WIDTH 25
#define CHESSBOARD_HEIGHT 19

#endif