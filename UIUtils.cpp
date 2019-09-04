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
void putsCenter(string str, bool xcen = true, bool ycen = true, int xoff = 0, int yoff=0, float zoomW=1) {
    int x = xoff, y = yoff;
    if (xcen) x += width/2 - str.length()*zoomW/2;
    if (ycen) y += height/2;
    gotoxy(x,y);
    puts(str.c_str());
}

//�����Զ����ȡ�ַ���
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
            //����3�����������жϣ��Ƿ��ǻس�����style�Ƿ��ʾ��д'\r'����style�Ƿ��ʾ��ֹ���ؿ��ַ���
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



void login() {
    const int xoff = 10;
    start:;
    clrscr();
    gotoxy(half_w-xoff, half_h-2);
    puts("���¼(��ESC����)");
    gotoxy(half_w-xoff, half_h);
    puts("�û�����");
    gotoxy(half_w-xoff, half_h+2);
    puts("���룺  ");

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
    //��ȡ����
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
                    if (i==0) printf("��");
                    else if (i==h-1) printf("��");
                    else printf("��");
                    textbackground(bgcolor);
                } else if (j==w-1) {
                    if (i==0) printf("��");
                    else if (i==h-1) printf("��");
                    else printf("��");
                } else {
                    if (i==0 || i==h-1) printf("��");
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
    printRect(20+xoff, 7+yoff, 26, 23, "1.���ƥ��", false, LIGHTBLUE,0, BLACK);
    printRect(47+xoff, 7+yoff, 26, 23, "2.���Ѷ�ս", false, LIGHTCYAN, 0, BLACK);
    printRect(74+xoff, 7+yoff, 26, 11, "3.��������", false, LIGHTMAGENTA, 0, BLACK);
    printRect(74+xoff, 19+yoff, 26, 11, "4.����", false, LIGHTGREEN, 0, BLACK);
    printRect(101+xoff, 7+yoff, 26, 11, "5.�����˺�", false, YELLOW, 0, BLACK);
    printRect(101+xoff, 19+yoff, 26, 11, "0.�˳�", false, LIGHTRED, 0, BLACK);

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

        } else if (ch == '5') {
            welcome();
            break;
        } else if (ch == '0') {
            exit(0);
        }
    }

}


void welcome() {
    clrscr();
    putsCenter("����������", 1, 1, 0, -10);
    putsCenter("����������������������������������������������������������������������������������������", 1, 1, 0, -8,0.5f);
    gotoxy(width/2-5, height/2-2);
    printf("1.  ��¼");
    gotoxy(width/2-5, height/2);
    printf("2.  ע��");
    gotoxy(width/2-5, height/2+2);
    printf("0.  �˳�");

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
    puts("ע���˺�(��ESC����)");
    gotoxy(half_w-xoff, half_h);
    puts("�û����� ");
    gotoxy(half_w-xoff, half_h+2);
    puts("���룺   ");
    gotoxy(half_w-xoff, half_h+4);
    puts("ȷ�����룺");

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
    //��ȡ����
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
    //��ȡ����
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
        MessageBox(NULL, "�����������벻ͬ������������", "����������", MB_OK|MB_ICONWARNING);
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
    printf("������...\n");
}

void printPersonalInfo() {
    clrscr();
    string name, des; int totalContest, winContest;
    playerInfo.Get("name", name); playerInfo.Get("description", des);
    playerInfo.Get("totalContest", totalContest);
    playerInfo.Get("winContest",winContest);
    if (des=="") des="����";
    if (totalContest==0) {
        printf("�û�����%s\n\n��ʷ��ֳ�����%d\n\nʤ�ʣ�����\n\n����ǩ����%s\n", name.c_str(), totalContest,  des.c_str());
    } else {
        printf("�û�����%s\n\n��ʷ��ֳ�����%d\n\nʤ�ʣ�%d%%\n\n����ǩ����%s\n", name.c_str(), totalContest, winContest*100/totalContest, des.c_str());
    }


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
    printf("������ʾ��");
    gotoxy(CHESSBOARD_WIDTH*4+10, 3);
    printf("ʹ�÷��������ѡ�еĸ���");
    gotoxy(CHESSBOARD_WIDTH*4+10, 4);
    printf("Enter����������");
}


void printGamingPlayerInfo() {
    std::string name, des;
    myInfo.Get("name", name);
    myInfo.Get("description", des);
    gotoxy(CHESSBOARD_WIDTH*4+10, 8);
    if (chessColor=='w') printf("������ң�");
    else printf("������ң�");
    printf("%s", name.c_str());
    gotoxy(CHESSBOARD_WIDTH*4+10, 9);
    printf("%s", des.c_str());


    oppInfo.Get("name", name);
    oppInfo.Get("description", des);
    gotoxy(CHESSBOARD_WIDTH*4+10, 11);
    if (chessColor=='b') printf("������ң�");
    else printf("������ң�");
    printf("%s", name.c_str());
    gotoxy(CHESSBOARD_WIDTH*4+10, 12);
    printf("%s", des.c_str());
}

void printTurningInfo() {
    gotoxy(CHESSBOARD_WIDTH*4+10, 15);
    clreol();
    if (isMyTurn) printf("��������");
    else printf("��ȴ��Է�����");
    gotoxy(width, height);
}

void setCursorVisible(bool visible) {
    HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_CURSOR_INFO CursorInfo;
    GetConsoleCursorInfo(handle, &CursorInfo);//��ȡ����̨�����Ϣ
    CursorInfo.bVisible = visible; //���ÿ���̨���ɼ���
    SetConsoleCursorInfo(handle, &CursorInfo);//���ÿ���̨���״̬
}

void printGamingCountdown(int leftTime) {
    gotoxy(CHESSBOARD_WIDTH*4+10, 16);
    clreol();
    if (isMyTurn) printf("���� %d ��������", leftTime);
    else printf("ʣ�� %d ��", leftTime);
}