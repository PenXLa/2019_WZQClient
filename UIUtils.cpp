
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



bool login() {
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
        if (str.length() < MAX_USERNAME_LEN) {
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
        else if (str.length()<MAX_PASSWORD_LEN) {
            str.push_back(ch);
            putch('*');
        }
        return READSTR_CONTINUE;
    });
    if (pwd == "") goto start;


}


