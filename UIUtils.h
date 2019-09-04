#ifndef _UIUTILS_
#define _UIUTILS_

#include <queue>
#include <functional>

void login();
void showMainMenu();
void welcome();
void showRegister();
void showPersonalInfo();
void printPersonalInfo();
void showChangeUserName();
void showChangeDescription();
void showChangePassword();
void printGameHelp();
void printGamingPlayerInfo();
void printTurningInfo();
void setCursorVisible(bool);
void printGamingCountdown(int leftTime);

extern std::queue<void(*)(void) > mainThreadFunctions;

#endif