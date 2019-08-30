#include "Server.h"
#include <iomanip>
#include <functional>
#include "UIUtils.h"
#include "commonHeads.h"
#include "config.h"

//用于在屏幕中心输出文字
void putsCenter(string str, bool xcen = true, bool ycen = true, int xoff = 0, int yoff=0) {
    int x = xoff, y = yoff;
    if (xcen) x += width/2 - str.length()/2;
    if (ycen) y += height/2;
    gotoxy(x,y);
    puts(str.c_str());
}

//用于自定义读取字符串
const string acceptedChars = "`1234567890-=~!@#$%^&*(_+qwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM[]\\{}|;':\",./<>?";
const int READSTR_SUCCESS = 1, READSTR_CANCELED = 2, READSTR_CONTINUE = 0;
string readString(std::function<int(char&, string&)> onGotChar) {
    string str;
    while(true) {
        char ch = getch();
        if (ch == '\b') {
            if (str.length()>0) {
                printf("\b \b");
                str.pop_back();
            }
        } else if (ch == '\r') {
            return str;
        } else if (ch == KEY_ESC) {
            int len = str.length();
            for (int i=0; i<len; ++i) printf("\b \b");
            str.clear();
        } else {
            int res = onGotChar(ch, str);
            if (res == READSTR_SUCCESS) return str;
            else if (res == READSTR_CANCELED) return "";
        }

    }
}



void login() {
    start:;
    clrscr();
    gotoxy(half_w-10, half_h-2);
    puts("请登录");
    gotoxy(half_w-10, half_h);
    puts("用户名：");
    gotoxy(half_w-10, half_h+2);
    puts("密码：  ");

    string username, pwd;
    gotoxy(half_w-10+8, half_h);
    username = readString([](char &ch, string &str){
        if (str.length() < MAX_USERNAME_LEN && acceptedChars.find(ch)!=acceptedChars.npos) {
            str.push_back(ch);
            putch(ch);
        }
        return READSTR_CONTINUE;
    });
    if (username == "") goto start;


    gotoxy(half_w-10+8, half_h+2);
    //读取密码
    pwd = readString([](char &ch, string &str){
        if (ch == KEY_UP) {
            //按下上方向键，返回用户名输入
            return READSTR_CANCELED;
        }
        else if (str.length()<MAX_PASSWORD_LEN && acceptedChars.find(ch)!=acceptedChars.npos) {
            str.push_back(ch);
            putch('*');
        }
        return READSTR_CONTINUE;
    });
    if (pwd == "") goto start;

    neb::CJsonObject json;
    json.Add("type", "login");
    json.Add("userName", username);
    json.Add("pwd", pwd);
    sendPack(json);
}

void showMainMenu() {
    clrscr();
    printf("1.随机匹配\n2.好友对战\n3.个人资料\n4.更换账号\n0.退出\n");
    while(1) {
        char ch = getch();
        if (ch == '1') {

        } else if (ch == '2') {

        } else if (ch == '3') {
            showPersonalInfo();
            break;
        } else if (ch == '4') {
            welcome();
            break;
        } else if (ch == '0') {
            exit(0);
        }
    }

}


void welcome() {
    clrscr();
    printf("联机五子棋\n1.登录\n2.注册\n3.离线游戏\n");
    while(1) {
        char ch = getch();
        if (ch == '1') {
            login();
            break;
        } else if (ch == '2') {
            showRegister();
            break;
        } else if (ch == '3') {

        }
    }

}


void showRegister() {
    clrscr();

    std::string username, pwd;
    printf("输入用户名：");
    std::cin >> username;
    printf("输入密码：");
    std::cin >> pwd;

    neb::CJsonObject json;
    json.Add("type", "register");
    json.Add("userName", username);
    json.Add("pwd", pwd);
    sendPack(json);
}

void showPersonalInfo() {
    neb::CJsonObject json;
    json.Add("type", "getPersonalInfo");
    sendPack(json);

    clrscr();
    printf("加载中...\n");
}

void printPersonalInfo(neb::CJsonObject &json) {
    clrscr();
    string name, des; int rating;
    json.Get("name", name); json.Get("description", des);
    json.Get("rating", rating);
    printf("%s\n等级:%d\n个性签名:%s\n", name.c_str(), rating, des.c_str());

    printf("\n\n1.更改用户名\n2.更改个性签名\n3.更改密码\n0.返回\n");
    while(1) {
        char ch = getch();
        if (ch == '1') {
            showChangeUserName();
        } else if (ch == '2') {

        } else if (ch == '3') {

        } else if (ch == '0') {

        }
    }
}



void showChangeUserName() {
    clrscr();
    printf("输入用户名：");
    std::string userName;
    cin >> userName;
    if (MessageBoxA(NULL, "确认要修改吗?", "修改用户名", MB_YESNO | MB_ICONQUESTION) == IDYES) {
        neb::CJsonObject json;
        json.Add("type", "changeUserName");
        json.Add("name", userName);
        sendPack(json);
    }
    showMainMenu();
}