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

extern std::queue<void(*)(void) > mainThreadFunctions;

#endif