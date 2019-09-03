#ifndef WZQCLIENT_GAME_H
#define WZQCLIENT_GAME_H

#include <tuple>
#include "JSONLib/CJsonObject.hpp"

void startGame();
void startMatching(bool friendMatching);
void putChess(int r, int c, char color);
void prepareGame();

extern volatile bool gameRunning;
extern volatile bool isMyTurn;
extern volatile char chessColor;
extern neb::CJsonObject myInfo, oppInfo;

#endif //WZQCLIENT_GAME_H
