#include "Game.h"
#include "conio2.h"
#include "config.h"
#include "UIUtils.h"
#include "Server.h"
#include "KeyEventHandler.h"
#include <windows.h>

volatile bool gameRunning = false;
volatile bool isMyTurn = false;
volatile char chessColor;
neb::CJsonObject myInfo, oppInfo;

//获取一个坐标是一个矩形的哪个边界，0代表不是边界
// 123
// 4 5
// 678
inline int getPosBoundary(int i, int j, int r, int c) {
    if (i==2*r) {
        if (j==4*c) return 1;
        else if (j==4*c+4) return 3;
        else if (j>4*c && j<4*c+4) return 2;
        else return 0;
    } else if (i==2*r+2) {
        if (j==4*c) return 6;
        else if (j==4*c+4) return 8;
        else if (j>4*c && j<4*c+4) return 7;
        else return 0;
    } else if (i>2*r && i<2*r+2) {
        if (j==4*c) return 4;
        else if (j==4*c+4) return 5;
        else return 0;
    } else return 0;
}



char mat[CHESSBOARD_HEIGHT][CHESSBOARD_WIDTH];
int curR = 5, curC = 5;
//画一个r行c列的网格，dirt参数若不为-1则表示仅更新dirt标识出的方格
void printGrid(int r, int c, int dirtR=-1, int dirtC=-1) {
    const char *chr;
    //基于5*3的cell大小
    for (int i=0; i<2*r+1; ++i) {
        for (int j=0; j<4*c+1; ++j) {
            if ((dirtR==-1 || i>=dirtR*2 && i<=dirtR*2+2) && (dirtC==-1 || j>=dirtC*4 && j<=dirtC*4+4)) {
                gotoxy(1+j,1+i);
                int b = getPosBoundary(i,j,curR, curC);
                if (i==0)
                    if (j==0) chr=(b==1?"┏":"┌");
                    else if (j==4*c) chr=(b==3?"┓":"┐");
                    else if (j%4==0) {
                        if (b==1) chr="┲";
                        else if (b==3) chr="┱";
                        else chr="┬";
                    }
                    else chr=(b==2||b==7?"━":"─");
                else if (i==2*r)
                    if (j==0) chr=(b==6?"┗":"└");
                    else if (j==4*c) chr=(b==8?"┛":"┘");
                    else if (j%4==0) {
                        if (b==6) chr="┺";
                        else if (b==8) chr="┹";
                        else chr="┴";
                    }
                    else chr=(b==2||b==7?"━":"─");
                else if (i%2==0)
                    if (j==0) {
                        if (b==1) chr="┢";
                        else if (b==6) chr="┡";
                        else chr="├";
                    }
                    else if (j==4*c) {
                        if (b==3) chr="┪";
                        else if (b==8) chr="┩";
                        else chr="┤";
                    }
                    else if (j%4==0) {
                        if (b==1) chr="╆";
                        else if (b==3) chr="╅";
                        else if (b==6) chr="╄";
                        else if (b==8) chr="╃";
                        else chr="┼";
                    }
                    else chr=(b==2||b==7?"━":"─");
                else
                    //以下代码是基于微软雅黑字体的对齐方式
                    if (j%4==0) chr=(b==4||b==5?"┃":"│");
                    else chr = " ";

                if (!strcmp(chr, "┃")) textcolor(LIGHTGRAY);
                else textcolor(DARKGRAY);
                printf(chr);
            }
        }
    }

    textcolor(WHITE);
    for (int i=0; i<r; ++i) {
        for (int j=0; j<c; ++j) {
            if ((dirtR==-1 || i==dirtR) && (dirtC==-1 || j==dirtC) && mat[i][j]) {
                gotoxy(j*4+3, i*2+2);
                if (mat[i][j]=='w') printf("●");
                else printf("○");
            }
        }
    }

    textcolor(LIGHTGRAY);
    gotoxy(width, height);
}




void prepareGame() {
    memset(mat, 0, sizeof mat);
    isMyTurn = false;
    gameRunning = true;

    clrscr();
    printGameHelp();
    printGamingPlayerInfo();
    printTurningInfo();
}


void startGame() {
    printGrid(CHESSBOARD_HEIGHT,CHESSBOARD_WIDTH);
    while(gameRunning) {
        if (kbhit()) {
            char ch = getch();

            if (ch == KEY_ESC) {
                if (MessageBoxA(NULL, "确认要退出这局游戏吗？如果您退出游戏，这局将视作您输。", "联机五子棋", MB_YESNO | MB_ICONQUESTION) == IDYES) {
                    neb::CJsonObject json;
                    json.Add("type", "exitRound");
                    sendPack(json);
                    gameRunning = false;
                    mainThreadFunctions.emplace(showMainMenu);
                    return;
                }
            } else if (isMyTurn) {
                if (ch == KEY_RIGHT || ch == KEY_LEFT || ch == KEY_UP || ch == KEY_DOWN) {
                    int lastR = curR, lastC = curC;
                    if (ch == KEY_UP) --curR;
                    else if (ch == KEY_DOWN) ++curR;
                    else if (ch == KEY_LEFT) --curC;
                    else if (ch == KEY_RIGHT) ++curC;

                    if (curR < 0) curR = CHESSBOARD_HEIGHT - 1;
                    else if (curR >= CHESSBOARD_HEIGHT) curR = 0;

                    if (curC < 0) curC = CHESSBOARD_WIDTH - 1;
                    else if (curC >= CHESSBOARD_WIDTH) curC = 0;


                    printGrid(CHESSBOARD_HEIGHT, CHESSBOARD_WIDTH, curR, curC);
                    printGrid(CHESSBOARD_HEIGHT, CHESSBOARD_WIDTH, lastR, lastC);
                } else if (ch == '\r') {
                    neb::CJsonObject json;
                    json.Add("type", "putChess");
                    json.Add("row", curR);
                    json.Add("col", curC);
                    sendPack(json);
                    //MessageBoxA(NULL, json.ToFormattedString().c_str(), "", MB_OK);
                }

            }
        }
        Sleep(10);
    }
}


void startMatching() {
    neb::CJsonObject json;
    json.Add("type", "match");
    sendPack(json);

    clrscr();
    printf("正在为您匹配玩家，请稍候...\n按ESC取消匹配\n");

    while(!gameRunning) {
        if (kbhit() && getch()==KEY_ESC) {
            neb::CJsonObject json;
            json.Add("type", "cancelMatching");
            sendPack(json);
            mainThreadFunctions.emplace(showMainMenu);
            break;
        }
        Sleep(10);
    }
}


void putChess(int r, int c, char color) {
    int oldR = curR, oldC = curC;
    curR = r; curC = c;
    mat[curR][curC] = color;
    printGrid(CHESSBOARD_HEIGHT, CHESSBOARD_WIDTH, oldR, oldC);
    printGrid(CHESSBOARD_HEIGHT, CHESSBOARD_WIDTH, curR, curC);
    if (color == chessColor) {
        isMyTurn = false;
        printTurningInfo();
    }
}