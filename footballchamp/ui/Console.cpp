// Console.cpp
#include "Console.h"
#include <cstdlib>
#include <iostream>
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

namespace ui {

void initConsole() {
#ifdef _WIN32
    SetConsoleOutputCP(1251);
    SetConsoleCP(1251);
#endif
}

void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void waitEnter() {
    std::cout << "\nНатисніть ENTER для продовження...";
    std::cin.ignore(10000, '\n');
    std::cin.get();
}

}
