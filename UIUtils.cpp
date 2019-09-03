#include "Server.h"
#include <iomanip>
#include <functional>
#include "UIUtils.h"
#include "commonHeads.h"
#include "config.h"
#include "KeyEventHandler.h"
#include "Game.h"

std::queue<void(*)(void)> mainThreadFunctions;



//��������Ļ�����������
void putsCenter(string str, bool xcen = true, bool ycen = true, int xoff = 0, int yoff=0) {
    int x = xoff, y = yoff;
    if (xcen) x += width/2 - str.length()/2;
    if (ycen) y += height/2;
    gotoxy(x,y);
    puts(str.c_str());
}

//�����Զ����ȡ�ַ���
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
    puts("���¼");
    gotoxy(half_w-10, half_h);
    puts("�û�����");
    gotoxy(half_w-10, half_h+2);
    puts("���룺  ");

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
    //��ȡ����
    pwd = readString([](char &ch, string &str){
        if (ch == KEY_UP) {
            //�����Ϸ�����������û�������
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
    printf("1.���ƥ��\n2.���Ѷ�ս\n3.��������\n4.�����˺�\n0.�˳�\n");

    while(1) {
        char ch = getch();
        if (ch == '1') {
            startMatching();
            break;
        } else if (ch == '2') {
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
    printf("����������\n1.��¼\n2.ע��\n3.������Ϸ\n");

    while(1) {
        char ch = getch();
        if (ch == '1') {
            login();
            break;
        } else if (ch == '2') {
            showRegister();
            break;
        } else if (ch == '3') {
            startGame();
            break;
        }
    }
}


void showRegister() {
    clrscr();

    std::string username, pwd;
    printf("�����û�����");
    std::cin >> username;
    printf("�������룺");
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
    printf("������...\n");
}

void printPersonalInfo() {
    clrscr();
    string name, des; int rating;
    playerInfo.Get("name", name); playerInfo.Get("description", des);
    playerInfo.Get("rating", rating);
    printf("%s\n�ȼ�:%d\n����ǩ��:%s\n", name.c_str(), rating, des.c_str());

    printf("\n\n1.�����û���\n2.���ĸ���ǩ��\n3.��������\n0.����\n");

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
    printf("�����û�����");
    std::string userName;
    cin >> userName;
    if (MessageBoxA(NULL, "ȷ��Ҫ�޸���?", "�޸��û���", MB_YESNO | MB_ICONQUESTION) == IDYES) {
        neb::CJsonObject json;
        json.Add("type", "changeUserName");
        json.Add("name", userName);
        sendPack(json);
    }
    showPersonalInfo();
}

void showChangeDescription() {
    clrscr();
    printf("�������ǩ����");
    std::string des;
    getline(cin,des);
    if (MessageBoxA(NULL, "ȷ��Ҫ�޸���?", "�޸ĸ���ǩ��", MB_YESNO | MB_ICONQUESTION) == IDYES) {
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
    cout << "���������:\n";
    cin >> oldpwd;
    cout << "����������:\n";
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
    printf("ʹ�÷��������ѡ�еĸ���");
    gotoxy(CHESSBOARD_WIDTH*4+10, 3);
    printf("Enter����������");
}


void printGamingPlayerInfo() {
    std::string name, des;
    myInfo.Get("name", name);
    myInfo.Get("description", des);
    gotoxy(CHESSBOARD_WIDTH*4+10, 5);
    if (chessColor=='w') printf("����");
    else printf("����");
    gotoxy(CHESSBOARD_WIDTH*4+10, 6);
    printf("%s", name.c_str());
    gotoxy(CHESSBOARD_WIDTH*4+10, 7);
    printf("%s", des.c_str());


    oppInfo.Get("name", name);
    oppInfo.Get("description", des);
    gotoxy(CHESSBOARD_WIDTH*4+10, 9);
    if (chessColor=='b') printf("����");
    else printf("����");
    gotoxy(CHESSBOARD_WIDTH*4+10, 10);
    printf("%s", name.c_str());
    gotoxy(CHESSBOARD_WIDTH*4+10, 11);
    printf("%s", des.c_str());
}

void printTurningInfo() {
    gotoxy(CHESSBOARD_WIDTH*4+10, 15);
    clreol();
    if (isMyTurn) printf("��������");
    else printf("��ȴ��Է�����");
}