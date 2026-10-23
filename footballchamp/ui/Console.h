// Console.h - робота з консоллю (уся платформо-залежна частина тут)
#pragma once

namespace ui {

void initConsole();   // кодування консолі (Windows: 1251)
void clearScreen();
void waitEnter();     // "Натисніть ENTER для продовження..."

}
