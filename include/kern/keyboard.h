#ifndef Kern_KEYBOARD_H_
#define Kern_KEYBOARD_H_
#include "system.hpp"
#include <io/Keyboard.h>

void keyboard_install();

//Gets a key event from the queue
LunOS::IO::KeyEvent getKeyEvent();
//Looks at the first key event on the queue
LunOS::IO::KeyEvent peekKeyEvent();

int alphaNumeric(LunOS::IO::KeyEvent* event);
int nonCommand(LunOS::IO::KeyEvent* event);
char GetCharacter(LunOS::IO::KeyEvent* event);

void keyboard_Init();


#endif /*KEYBOARD_H_*/
