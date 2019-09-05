#ifndef _UIUTILS_
#define _UIUTILS_

#include <queue>
#include <functional>
#include <mutex>
#include <string>

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
int readString(std::function<int(char&, std::string&)> onGotChar, std::string &str, int style = 0);
void showHelp();

extern const std::string acceptedChars;
extern const int READSTR_SUCCESS, READSTR_CANCELED, READSTR_CONTINUE;
extern const int READSTR_OVERRIDE_ENTER, READSTR_FORBID_EMPTY, READSTR_OVERRIDE_ESC;

extern std::queue<void(*)(void) > mainThreadFunctions;
extern std::mutex uimux;//保证打印的原子性，防止多线程打印造成混乱

#endif