#ifndef WZQCLIENT_GAME_H
#define WZQCLIENT_GAME_H

#include <tuple>
#include "JSONLib/CJsonObject.hpp"
#include "config.h"

void startGame();
void startMatching(bool friendMatching);
void putChess(int r, int c, char color);
void prepareGame();

extern volatile bool gameRunning;
extern volatile bool isMyTurn;
extern volatile char chessColor;
extern neb::CJsonObject myInfo, oppInfo;
extern char mat[CHESSBOARD_HEIGHT][CHESSBOARD_WIDTH];
void printGrid(int r, int c, int dirtR=-1, int dirtC=-1);

#endif //WZQCLIENT_GAME_H
