#include "Server.h"
#include <iomanip>
#include <functional>
#include "UIUtils.h"
#include "commonHeads.h"
#include "config.h"
#include "KeyEventHandler.h"
#include "Game.h"

std::queue<void(*)(void)> mainThreadFunctions;



//用于在屏幕中心输出文字
void putsCenter(string str, bool xcen = true, bool ycen = true, int xoff = 0, int yoff=0, float zoomW=1) {
    int x = xoff, y = yoff;
    if (xcen) x += width/2 - str.length()*zoomW/2;
    if (ycen) y += height/2;
    gotoxy(x,y);
    puts(str.c_str());
}

//用于自定义读取字符串
const string acceptedChars = "`1234567890-=~!@#$%^&*(_+qwertyuiopasdfghjklzxcvbnmQWERTYUIOPASDFGHJKLZXCVBNM[]\\{}|;':\",./<>?";
const int READSTR_SUCCESS = 1, READSTR_CANCELED = 2, READSTR_CONTINUE = 0;
const int READSTR_OVERRIDE_ENTER = 1, READSTR_FORBID_EMPTY = 1<<1, READSTR_OVERRIDE_ESC = 1<<2;
int readString(std::function<int(char&, string&)> onGotChar, string &str, int style = 0) {
    setCursorVisible(true);
    while(true) {
        char ch = getch();
        if (ch == '\b') {
            if (str.length()>0) {
                printf("\b \b");
                str.pop_back();
            }
        } else if (ch == '\r' && !(style & READSTR_OVERRIDE_ENTER) && !(str.length()==0 && (style&READSTR_FORBID_EMPTY))) {
            //上面3个条件依次判断：是否是回车键，style是否表示覆写'\r'键，style是否表示禁止返回空字符串
            setCursorVisible(false);
            return READSTR_SUCCESS;
        } else if (ch == KEY_ESC && !(style&READSTR_OVERRIDE_ESC)) {
            setCursorVisible(false);
            return READSTR_CANCELED;
        } else {
            int res = onGotChar(ch, str);
            if (res == READSTR_SUCCESS || res == READSTR_CANCELED) {
                setCursorVisible(false);
                return res;
            }
        }

    }
}



void login() {
    const int xoff = 10;
    start:;
    clrscr();
    gotoxy(half_w-xoff, half_h-2);
    puts("请登录(按ESC返回)");
    gotoxy(half_w-xoff, half_h);
    puts("用户名：");
    gotoxy(half_w-xoff, half_h+2);
    puts("密码：  ");

    string username, pwd;
    gotoxy(half_w-xoff+8, half_h);
    int res = readString([](char &ch, string &str){
        if (str.length() < MAX_USERNAME_LEN && acceptedChars.find(ch)!=acceptedChars.npos) {
            str.push_back(ch);
            putch(ch);
        }
        return READSTR_CONTINUE;
    }, username, READSTR_FORBID_EMPTY);
    if (res == READSTR_CANCELED) {
        mainThreadFunctions.emplace(welcome);
        return;
    }


    gotoxy(half_w-xoff+8, half_h+2);
    //读取密码
    res = readString([](char &ch, string &str){
        if (str.length()<MAX_PASSWORD_LEN && acceptedChars.find(ch)!=acceptedChars.npos) {
            str.push_back(ch);
            putch('*');
        }
        return READSTR_CONTINUE;
    }, pwd, READSTR_FORBID_EMPTY);
    if (res == READSTR_CANCELED) goto start;

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
            startMatching(false);
            break;
        } else if (ch == '2') {
            startMatching(true);
            break;
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
    putsCenter("联机五子棋", 1, 1, 0, -10);
    putsCenter("────────────────────────────────────────────", 1, 1, 0, -8,0.5f);
    gotoxy(width/2-5, height/2-2);
    printf("1.  登录");
    gotoxy(width/2-5, height/2);
    printf("2.  注册");
    gotoxy(width/2-5, height/2+2);
    printf("0.  退出");

    while(1) {
        char ch = getch();
        if (ch == '1') {
            login();
            break;
        } else if (ch == '2') {
            showRegister();
            break;
        } else if (ch == '0') {
            exit(0);
        }
    }
}


void showRegister() {
    const int xoff = 10;
    start:
    clrscr();
    gotoxy(half_w-xoff, half_h-2);
    puts("注册账号(按ESC返回)");
    gotoxy(half_w-xoff, half_h);
    puts("用户名： ");
    gotoxy(half_w-xoff, half_h+2);
    puts("密码：   ");
    gotoxy(half_w-xoff, half_h+4);
    puts("确认密码：");

    string username, pwd, pwd2;

    gotoxy(half_w-xoff+10, half_h);
    clreol();
    int res = readString([](char &ch, string &str){
        if (str.length() < MAX_USERNAME_LEN && acceptedChars.find(ch)!=acceptedChars.npos) {
            str.push_back(ch);
            putch(ch);
        }
        return READSTR_CONTINUE;
    }, username, READSTR_FORBID_EMPTY);
    if (res == READSTR_CANCELED) {
        mainThreadFunctions.emplace(welcome);
        return;
    }

    pwd_1:
    gotoxy(half_w-xoff+10, half_h+2);
    clreol();
    //读取密码
    res = readString([](char &ch, string &str){
        if (str.length()<MAX_PASSWORD_LEN && acceptedChars.find(ch)!=acceptedChars.npos) {
            str.push_back(ch);
            putch('*');
        }
        return READSTR_CONTINUE;
    }, pwd, READSTR_FORBID_EMPTY);
    if (res == READSTR_CANCELED) goto start;


    gotoxy(half_w-xoff+10, half_h+4);
    clreol();
    //读取密码
    res = readString([](char &ch, string &str){
        if (str.length()<MAX_PASSWORD_LEN && acceptedChars.find(ch)!=acceptedChars.npos) {
            str.push_back(ch);
            putch('*');
        }
        return READSTR_CONTINUE;
    }, pwd2, READSTR_FORBID_EMPTY);
    if (res == READSTR_CANCELED) {
        gotoxy(half_w-xoff+10, half_h+4);
        clreol();
        goto pwd_1;
    }

    if (pwd!=pwd2) {
        MessageBox(NULL, "两次输入密码不同，请重新输入", "联机五子棋", MB_OK|MB_ICONWARNING);
        gotoxy(half_w-xoff+10, half_h+4);
        clreol();
        gotoxy(half_w-xoff+10, half_h+2);
        clreol();
        goto pwd_1;
    }


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

void printPersonalInfo() {
    clrscr();
    string name, des; int totalContest, winContest;
    playerInfo.Get("name", name); playerInfo.Get("description", des);
    playerInfo.Get("totalContest", totalContest);
    playerInfo.Get("winContest",winContest);
    if (totalContest==0) {
        printf("%s\n历史棋局场数:%d\n胜率:暂无\n个性签名:%s\n", name.c_str(), totalContest,  des.c_str());
    } else {
        printf("%s\n历史棋局场数:%d\n胜率:%d%%\n个性签名:%s\n", name.c_str(), totalContest, winContest*100/totalContest, des.c_str());
    }


    printf("\n\n1.更改用户名\n2.更改个性签名\n3.更改密码\n0.返回\n");

    while(1) {
        char ch = getch();

        if (ch == '1') {
            showChangeUserName();
            break;
        } else if (ch == '2') {
            showChangeDescription();
            break;
        } else if (ch == '3') {
            showChangePassword();
            break;
        } else if (ch == '0') {
            showMainMenu();
            break;
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
    showPersonalInfo();
}

void showChangeDescription() {
    clrscr();
    printf("输入个性签名：");
    std::string des;
    getline(cin,des);
    if (MessageBoxA(NULL, "确认要修改吗?", "修改个性签名", MB_YESNO | MB_ICONQUESTION) == IDYES) {
        neb::CJsonObject json;
        json.Add("type", "changeDescription");
        json.Add("des", des);
        sendPack(json);
    }
    showPersonalInfo();
}


void showChangePassword() {
    clrscr();

    string oldpwd, newpwd;
    cout << "输入旧密码:\n";
    cin >> oldpwd;
    cout << "输入新密码:\n";
    cin >> newpwd;

    neb::CJsonObject json;
    json.Add("type", "changePassword");
    json.Add("old", oldpwd);
    json.Add("new", newpwd);
    sendPack(json);

    showPersonalInfo();
}


void printGameHelp() {
    gotoxy(CHESSBOARD_WIDTH*4+10, 2);
    printf("使用方向键控制选中的格子");
    gotoxy(CHESSBOARD_WIDTH*4+10, 3);
    printf("Enter键放下棋子");
}


void printGamingPlayerInfo() {
    std::string name, des;
    myInfo.Get("name", name);
    myInfo.Get("description", des);
    gotoxy(CHESSBOARD_WIDTH*4+10, 5);
    if (chessColor=='w') printf("白棋");
    else printf("黑棋");
    gotoxy(CHESSBOARD_WIDTH*4+10, 6);
    printf("%s", name.c_str());
    gotoxy(CHESSBOARD_WIDTH*4+10, 7);
    printf("%s", des.c_str());


    oppInfo.Get("name", name);
    oppInfo.Get("description", des);
    gotoxy(CHESSBOARD_WIDTH*4+10, 9);
    if (chessColor=='b') printf("白棋");
    else printf("黑棋");
    gotoxy(CHESSBOARD_WIDTH*4+10, 10);
    printf("%s", name.c_str());
    gotoxy(CHESSBOARD_WIDTH*4+10, 11);
    printf("%s", des.c_str());
}

void printTurningInfo() {
    gotoxy(CHESSBOARD_WIDTH*4+10, 15);
    clreol();
    if (isMyTurn) printf("该您下了");
    else printf("请等待对方下棋");
    gotoxy(width, height);
}

void setCursorVisible(bool visible) {
    HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO CursorInfo;
    GetConsoleCursorInfo(handle, &CursorInfo);//获取控制台光标信息
    CursorInfo.bVisible = visible; //设置控制台光标可见性
    SetConsoleCursorInfo(handle, &CursorInfo);//设置控制台光标状态
}