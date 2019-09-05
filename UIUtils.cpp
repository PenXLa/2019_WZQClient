#include "Server.h"
#include <iomanip>
#include <functional>
#include "UIUtils.h"
#include "commonHeads.h"
#include "config.h"
#include "KeyEventHandler.h"
#include "Game.h"

std::queue<void(*)(void)> mainThreadFunctions;
std::mutex uimux;


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
int readString(std::function<int(char&, string&)> onGotChar, string &str, int style) {
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
        } else if (!(ch<=31 || ch==127)) {
            int res = onGotChar(ch, str);
            if (res == READSTR_SUCCESS || res == READSTR_CANCELED) {
                setCursorVisible(false);
                return res;
            }
        }

    }
}

int readWString(std::function<int(wchar_t&, std::wstring&)> onGotChar, std::wstring &str, int style = 0) {
    setCursorVisible(true);
    while(true) {
        wchar_t ch = _getwch();
        if (ch == '\b') {
            if (str.length()>0) {
                if (*str.rbegin()<255) printf("\b \b");
                else printf("\b\b  \b\b");
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

std::string wch2bytes(wchar_t wch) {
    int bytes = ::WideCharToMultiByte(CP_ACP, 0, &wch, 1, NULL, 0, NULL, NULL);
    std::string ans;
    ans.resize(bytes);
    bytes = ::WideCharToMultiByte(CP_ACP, 0, &wch, 1, const_cast<char *>(ans.data()), ans.size(), NULL, NULL);
    return ans;
}

std::string wstr2str(const std::wstring &wstr) {
    int bytes = ::WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), wstr.size(), NULL, 0, NULL, NULL);
    std::string str;
    str.resize(bytes);
    bytes = ::WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), wstr.size(),const_cast<char *>(str.data()),str.size(), NULL, NULL);
    return str;
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



void printRect(int x, int y, int w, int h, const string &txt, bool withBoard = true, int bgcolor = BLACK, int fgcolor = LIGHTGRAY, int txtcolor = LIGHTGRAY) {
    textcolor(fgcolor);
    gotoxy(x,y);
    for (int i=0; i<h; ++i) {
        gotoxy(x, y+i);
        for (int j=0; j<w; ++j) {
            if (withBoard) {
                if (j==0) {
                    textbackground(BLACK);
                    if (i==0) printf("┌");
                    else if (i==h-1) printf("└");
                    else printf("│");
                    textbackground(bgcolor);
                } else if (j==w-1) {
                    if (i==0) printf("┐");
                    else if (i==h-1) printf("┘");
                    else printf("│");
                } else {
                    if (i==0 || i==h-1) printf("─");
                    else printf(" ");
                }
            } else {
                textbackground(j==0?BLACK:bgcolor);
                printf(" ");
            }

        }
    }
    textcolor(txtcolor);
    gotoxy((w-txt.length())/2+x, y+h/2);
    printf(txt.c_str());
    textcolor(LIGHTGRAY);
    textbackground(BLACK);
}



void showMainMenu() {
    const int xoff = 5, yoff = 2;
    clrscr();
    printRect(20+xoff, 7+yoff, 26, 23, "1.随机匹配", false, LIGHTBLUE,0, BLACK);
    printRect(47+xoff, 7+yoff, 26, 23, "2.好友对战", false, LIGHTCYAN, 0, BLACK);
    printRect(74+xoff, 7+yoff, 26, 11, "3.个人资料", false, LIGHTMAGENTA, 0, BLACK);
    printRect(74+xoff, 19+yoff, 26, 11, "4.帮助", false, LIGHTGREEN, 0, BLACK);
    printRect(101+xoff, 7+yoff, 26, 11, "5.更换账号", false, YELLOW, 0, BLACK);
    printRect(101+xoff, 19+yoff, 26, 11, "ESC.退出", false, LIGHTRED, 0, BLACK);

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
            showHelp();
            break;
        } else if (ch == '5') {
            welcome();
            break;
        } else if (ch == KEY_ESC) {
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
        } else if (ch == '0' || ch == KEY_ESC) {
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

    string username;
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
    string pwd, pwd2;
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



const int pioffx = 57, pioffy = 10;
int desoffx, desoffy;
void printPersonalInfo() {
    clrscr();
    string name, des; int totalContest, winContest;
    playerInfo.Get("name", name); playerInfo.Get("description", des);
    playerInfo.Get("totalContest", totalContest);
    playerInfo.Get("winContest",winContest);
    if (des=="") des="暂无";


    int xoff = pioffx, yoff = pioffy;
    //打印用户名
    gotoxy(xoff, yoff);
    textcolor(LIGHTBLUE);
    printf(name.c_str());
    textcolor(LIGHTGRAY);
    //打印历史战绩
    gotoxy(xoff, yoff+=2);
    if (totalContest>0) {
        printf("共玩过 %d 局，其中赢了 %d 局，胜率为 %d%%", totalContest, winContest, winContest*100/totalContest);

        gotoxy(xoff, yoff+=2);
        const int barLen = 40;
        for (int i=0; i<barLen; ++i) {
            textbackground(i*totalContest < barLen*winContest?LIGHTGREEN:LIGHTRED);
            putchar(' ');
        }
        textbackground(BLACK);
    }
    else printf("共玩过 %d 局，其中赢了 %d 局，暂无胜率", totalContest, winContest);

    //打印个性签名
    gotoxy(xoff, yoff+=2);
    printf("个性签名：%s", des.c_str());
    desoffx = xoff+10, desoffy = yoff;

    textcolor(DARKGRAY);
    gotoxy(xoff+=14, yoff+=4);
    printf("1.更改用户名");
    gotoxy(xoff, yoff+=2);
    printf("2.更改个性签名");
    gotoxy(xoff, yoff+=2);
    printf("3.更改密码");
    gotoxy(xoff, yoff+=2);
    printf("ESC.返回");
    textcolor(LIGHTGRAY);

    while(1) {
        char ch = getch();

        if (ch == '1') {
            mainThreadFunctions.emplace(showChangeUserName);
            break;
        } else if (ch == '2') {
            mainThreadFunctions.emplace(showChangeDescription);
            break;
        } else if (ch == '3') {
            mainThreadFunctions.emplace(showChangePassword);
            break;
        } else if (ch==KEY_ESC) {
            mainThreadFunctions.emplace(showMainMenu);
            break;
        }
    }

}



void showChangeUserName() {
    gotoxy(pioffx, pioffy);
    clreol();
    std::string userName;

    int res = readString([](char &ch, string &str){
        if (str.length() < MAX_USERNAME_LEN && acceptedChars.find(ch)!=acceptedChars.npos) {
            str.push_back(ch);
            putch(ch);
        }
        return READSTR_CONTINUE;
    }, userName, READSTR_FORBID_EMPTY);

    if (res == READSTR_SUCCESS) {
        if (MessageBoxA(NULL, "确认要修改吗?", "修改用户名", MB_YESNO | MB_ICONQUESTION) == IDYES) {
            neb::CJsonObject json;
            json.Add("type", "changeUserName");
            json.Add("name", userName);
            sendPack(json);
        }
    }

    showPersonalInfo();
}

void showChangeDescription() {
    gotoxy(desoffx, desoffy);
    clreol();

    std::wstring wdes;
    int res = readWString([](wchar_t& ch, std::wstring& str){
        if (str.length()<MAX_DESCRIPTION_LEN) {
            str+=ch;
            _putwch(ch);
        }
        return READSTR_CONTINUE;
    }, wdes);

    if (res == READSTR_SUCCESS) {
        std::string des = wstr2str(wdes);
        if (MessageBoxA(NULL, "确认要修改吗?", "修改个性签名", MB_YESNO | MB_ICONQUESTION) == IDYES) {
            neb::CJsonObject json;
            json.Add("type", "changeDescription");
            json.Add("des", des);
            sendPack(json);
        }
    }
    showPersonalInfo();
}


void showChangePassword() {
    start:;
    clrscr();
    gotoxy(half_w-10, half_h-3);
    cout << "修改密码(按ESC返回)";
    gotoxy(half_w-10, half_h-1);
    cout << "输入旧密码：";
    gotoxy(half_w-10, half_h+1);
    cout << "输入新密码：";

    string oldpwd, newpwd;
    gotoxy(half_w+2, half_h-1);
    int res = readString([](char &ch, string &str){
        if (str.length() < MAX_PASSWORD_LEN && acceptedChars.find(ch)!=acceptedChars.npos) {
            str.push_back(ch);
            putch('*');
        }
        return READSTR_CONTINUE;
    }, oldpwd, READSTR_FORBID_EMPTY);
    if (res == READSTR_CANCELED) {
        mainThreadFunctions.emplace(showPersonalInfo);
        return;
    }

    gotoxy(half_w+2, half_h+1);
    res = readString([](char &ch, string &str){
        if (str.length() < MAX_PASSWORD_LEN && acceptedChars.find(ch)!=acceptedChars.npos) {
            str.push_back(ch);
            putch('*');
        }
        return READSTR_CONTINUE;
    }, newpwd, READSTR_FORBID_EMPTY);

    if (res == READSTR_CANCELED) goto start;

    neb::CJsonObject json;
    json.Add("type", "changePassword");
    json.Add("old", oldpwd);
    json.Add("new", newpwd);
    sendPack(json);

    showPersonalInfo();
}


void printGameHelp() {
    uimux.lock();
    gotoxy(CHESSBOARD_WIDTH*4+22, 36);
    printf("操作提示：");
    gotoxy(CHESSBOARD_WIDTH*4+22, 37);
    printf("使用方向键控制选中的格子");
    gotoxy(CHESSBOARD_WIDTH*4+22, 38);
    printf("Enter键放下棋子");
    uimux.unlock();
}


void printGamingPlayerInfo() {
    uimux.lock();
    std::string name, des;
    myInfo.Get("name", name);
    myInfo.Get("description", des);
    gotoxy(CHESSBOARD_WIDTH*4+10, 13);
    textcolor(LIGHTGREEN);
    if (chessColor=='w') printf("白棋玩家：");
    else printf("黑棋玩家：");
    printf("%s", name.c_str());
    gotoxy(CHESSBOARD_WIDTH*4+10, 14);
    printf("%s", des.c_str());


    oppInfo.Get("name", name);
    oppInfo.Get("description", des);
    gotoxy(CHESSBOARD_WIDTH*4+10, 18);
    textcolor(LIGHTRED);
    if (chessColor=='b') printf("白棋玩家：");
    else printf("黑棋玩家：");
    printf("%s", name.c_str());
    gotoxy(CHESSBOARD_WIDTH*4+10, 19);
    printf("%s", des.c_str());

    textcolor(LIGHTGRAY);

    uimux.unlock();
}

void printTurningInfo() {
    uimux.lock();
    gotoxy(CHESSBOARD_WIDTH*4+24, 3);
    clreol();
    if (isMyTurn) printf("该您下了");
    else printf("请等待对方落子");
    gotoxy(width, height);
    uimux.unlock();
}

void setCursorVisible(bool visible) {
    HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO CursorInfo;
    GetConsoleCursorInfo(handle, &CursorInfo);//获取控制台光标信息
    CursorInfo.bVisible = visible; //设置控制台光标可见性
    SetConsoleCursorInfo(handle, &CursorInfo);//设置控制台光标状态
}

void printGamingCountdown(int leftTime) {
    uimux.lock();
    gotoxy(CHESSBOARD_WIDTH*4+24, 5);
    clreol();
    if (isMyTurn) printf("请在 %d 秒内落子", leftTime);
    else printf("剩余 %d 秒", leftTime);
    uimux.unlock();
}

void showHelp() {
    clrscr();
    putsCenter("联机五子棋", 1, 1, 0, -13);
    putsCenter("────────────────────────────────────────────", 1, 1, 0, -11,0.5f);


    int xoff=half_w-22, yoff=half_h-8;
    gotoxy(xoff, yoff);
    printf("作者：赵龙宇");

    gotoxy(xoff, yoff+=2);
    printf("联机五子棋功能介绍：");

    gotoxy(xoff, yoff+=2);
    printf("1.随机匹配");
    gotoxy(xoff, yoff+=1);
    printf("    随机匹配可以帮您随机找到一位玩家与您下棋。");

    gotoxy(xoff, yoff+=2);
    printf("2.好友对战");
    gotoxy(xoff, yoff+=1);
    printf("    通过输入和朋友约好的口令(暗号)，系统可以将");
    gotoxy(xoff, yoff+=1);
    printf("你们分配到一局，从而进行好友对战。");

    gotoxy(xoff, yoff+=2);
    printf("3.个人资料");
    gotoxy(xoff, yoff+=1);
    printf("    这里有您的用户名、历史战绩、个性签名等内容，");
    gotoxy(xoff, yoff+=1);
    printf("除此之外，您还可以在这里修改您的用户名和密码等");
    gotoxy(xoff, yoff+=1);
    printf("个人信息。");

    gotoxy(xoff, yoff+=2);
    printf("4.更换账号");
    gotoxy(xoff, yoff+=1);
    printf("    退出当前账号，切换账号登录。");

    putsCenter("按ESC返回", true, false, 0, yoff+=4);

    while(getch()!=KEY_ESC);
    mainThreadFunctions.emplace(showMainMenu);
}