#include "Server.h"
#include <iomanip>
#include <functional>
#include "UIUtils.h"
#include "commonHeads.h"
#include "config.h"

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

void printPersonalInfo(neb::CJsonObject &json) {
    clrscr();
    string name, des; int rating;
    json.Get("name", name); json.Get("description", des);
    json.Get("rating", rating);
    printf("%s\n�ȼ�:%d\n����ǩ��:%s\n", name.c_str(), rating, des.c_str());

    printf("\n\n1.�����û���\n2.���ĸ���ǩ��\n3.��������\n0.����\n");
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
    printf("�����û�����");
    std::string userName;
    cin >> userName;
    if (MessageBoxA(NULL, "ȷ��Ҫ�޸���?", "�޸��û���", MB_YESNO | MB_ICONQUESTION) == IDYES) {
        neb::CJsonObject json;
        json.Add("type", "changeUserName");
        json.Add("name", userName);
        sendPack(json);
    }
    showMainMenu();
}