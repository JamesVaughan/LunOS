#ifndef CONSOLE_H_
#define CONSOLE_H_

//Called by the time to get the console to update the current time
void updateClock();
//CLS
void clearConsole();
//write text to the console
void writeConsoleText(const char* text);
//write information
void writeConsoleInformation(const char* text);
//write a warning
void writeConsoleWarning(const char* text);

//write text to the console
void putchConsoleText(const char text);
//write information
void putchConsoleInformation(const char text);
//write a warning
void putchConsoleWarning(const char text);

//Change the colour of text
void setConsoleTextColour(const char colour);
//Change the colour of Information
void setConsoleInfoColour(const char colour);
//change the colour of a warning
void setConsoleWarningColour(const char colour);
//sets up default values
void initConsole();

void updateCursor(unsigned char x, unsigned char y);
void getCursorPosition(unsigned char* mx, unsigned char* my);

//It is just printf... %1 = text %2 = info %3= warning
void printf(const char* format, ...);

void initConsolePhase2();

#endif /*CONSOLE_H_*/
