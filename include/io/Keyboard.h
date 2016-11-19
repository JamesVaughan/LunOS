/*
 * Keyboard.h
 *
 *  Created on: 21-Jul-2009
 *      Author: james
 */

#ifndef KEYBOARD_H_
#define KEYBOARD_H_
#include <io/Stream.h>
#include <io/Io.h>

//Key Event Types
#define KEY_NO_EVENT 0
#define KEY_DOWN 1
#define KEY_UP 2
#define KEY_MOD_DOWN 4
#define KEY_MOD_UP 5

//KeyCodes for the letters
#define VK_A 'a'
#define VK_B 'b'
#define VK_C 'c'
#define VK_D 'd'
#define VK_E 'e'
#define VK_F 'f'
#define VK_G 'g'
#define VK_H 'h'
#define VK_I 'i'
#define VK_J 'j'
#define VK_K 'k'
#define VK_L 'l'
#define VK_M 'm'
#define VK_N 'n'
#define VK_O 'o'
#define VK_P 'p'
#define VK_Q 'q'
#define VK_R 'r'
#define VK_S 's'
#define VK_T 't'
#define VK_U 'u'
#define VK_V 'v'
#define VK_W 'w'
#define VK_X 'x'
#define VK_Y 'y'
#define VK_Z 'z'

//KeyCodes for the numbers
#define VK_1 '1'
#define VK_2 '2'
#define VK_3 '3'
#define VK_4 '4'
#define VK_5 '5'
#define VK_6 '6'
#define VK_7 '7'
#define VK_8 '8'
#define VK_9 '9'
#define VK_0 '0'

//KeyCodes for the function keys
#define VK_F1 1
#define VK_F2 2
#define VK_F3 3
#define VK_F4 4
#define VK_F5 5
#define VK_F6 6
#define VK_F7 7
#define VK_F8 11
#define VK_F9 12
#define VK_F10 13
//NEWLINE 10
#define VK_F11 14
#define VK_F12 15

#define VK_Enter 10

//punctuation
#define VK_Period '.'
#define VK_Comma ','
#define VK_Tilda '~'
#define VK_Minus '-'
#define VK_UnderScore '-'
#define VK_Equals '='
#define VK_Plus '+'
#define VK_LessThan ','
#define VK_GreaterThan '.'
#define VK_Space 32
#define VK_Questionmark '/'
#define VK_FSlash '/'
#define VK_BSlash '\\'
#define VK_Quote '\''
#define VK_DQuote '\"'

//MISC Keys
#define VK_Up 16
#define VK_Down 17
#define VK_Left 18
#define VK_Right 19
#define VK_PageUp 20
#define VK_PageDown 21
#define VK_Home 22
#define VK_End 23
#define VK_Backspace '\b'
#define VK_Control 24
#define VK_ShiftL 25
#define VK_ShiftR 26
#define VK_Alt 28
#define VK_Escape 27
#define VK_Windows 29
#define VK_Tab '\t'
#define VK_Delete 127
#define VK_CapsLock 128
#define VK_ScrollLock 129
#define VK_Insert 130

#define KEYMOD_NONE 0
#define KEYMOD_SHIFT 1
#define KEYMOD_WINDOWS 2
#define KEYMOD_CTRL 4
#define KEYMOD_ALT 8


namespace LunOS
{
namespace IO
{

typedef struct KeyEvent
{
	unsigned char type;
	unsigned char key;
	unsigned short modifiers;
} KeyEvent;


class Keyboard : public LunOS::IO::Stream
{
public:
	static Keyboard* GetKeyboardStream();
	// Blocks until a key event is read
	LunOS::IO::KeyEvent ReadKeyEvent();
	// Tries to read a key event, if there is none returns the null event
	LunOS::IO::KeyEvent TryReadKey();
private:
	Keyboard(unsigned int fd, unsigned int bufferSize);
	~Keyboard();
};
}
}

#endif /* KEYBOARD_H_ */
