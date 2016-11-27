
#include <SysCall.h>
#include <kern/system.hpp>
#include <kern/keyboard.h>

using namespace LunOS::IO;

Lock* KeyboardLock;
volatile LunOS::IO::KeyEvent eventBuffer [128];
volatile unsigned int bufferPosition = 0;
volatile unsigned short modifiers = 0;
volatile unsigned short offset = 0;
volatile bool Shift = false;

unsigned char kbdus[128*2] =
{
    0, 0,  27, 27, '1', '!', '2', '@', '3', '#', '4', '$', '5', '%', '6', '^', '7', '&', '8', '*',	/* 9 */
  '9', '(', '0', ')', '-', '_', '=', '+', VK_Backspace, VK_Backspace,	/* Backspace */
  '\t', '\t',		/* Tab */
  'q', 'q', 'w', 'w', 'e', 'e', 'r', 'r',	/* 19 */
  't', 't', 'y', 'y', 'u', 'u', 'i', 'i', 'o', 'o', 'p', 'p', '[', '{', ']', '}', '\n', '\n',		/* Enter key */
    VK_Control, VK_Control,			/* 29   - Control */
  'a', 'a', 's', 's', 'd', 'd', 'f', 'f', 'g', 'g', 'h', 'h', 'j', 'j', 'k', 'k', 'l', 'l', ';', ':',	/* 39 */
 '\'', '"', '`', '~',   VK_ShiftL,VK_ShiftL,		/* Left shift */
 '\\', '|', 'z', 'z', 'x', 'x', 'c', 'c', 'v', 'v', 'b', 'b', 'n', 'n',			/* 49 */
  'm', 'm', ',', '<', '.', '>', '/', '?',   VK_ShiftR, VK_ShiftR,					/* Right shift */
  '*',
    VK_Alt,	/* Alt */
    VK_Alt,	/* Alt */
    VK_Space,	/* Space bar */
    VK_Space,	/* Space bar */
    VK_CapsLock,	/* Caps lock */
    VK_CapsLock,	/* Caps lock */
    0,	/* 59 - F1 key ... > */
    0,   0,   0,   0,   0,   0,   0,   0,
    0,	/* < ... F10 */
    0,	/* 69 - Num lock*/
    0,	/* 59 - F1 key ... > */
        0,   0,   0,   0,   0,   0,   0,   0,
        0,	/* < ... F10 */
        0,	/* 69 - Num lock*/
    VK_ScrollLock,	/* Scroll Lock */
    VK_ScrollLock,	/* Scroll Lock */
    VK_Home,	/* Home key */
    VK_Home,	/* Home key */
    VK_Up,	/* Up Arrow */
    VK_Up,	/* Up Arrow */
    VK_PageUp,	/* Page Up */
    VK_PageUp,	/* Page Up */
    VK_Minus,
    VK_Minus,
    VK_Left,	/* Left Arrow */
    VK_Left,	/* Left Arrow */
    0,
    0,
    VK_Right,	/* Right Arrow */
    VK_Right,	/* Right Arrow */
    VK_Plus,
    VK_Plus,
    VK_End,	/* 79 - End key*/
    VK_End,	/* 79 - End key*/
    VK_Down,	/* Down Arrow */
    VK_Down,	/* Down Arrow */
    VK_PageDown,	/* Page Down */
    VK_PageDown,	/* Page Down */
    VK_Insert,	/* Insert Key */
    VK_Insert,	/* Insert Key */
    VK_Delete,	/* Delete Key */
    VK_Delete,	/* Delete Key */
    0,   0,   0,
    0,   0,   0,
    VK_F11,	/* F11 Key */
    VK_F11,	/* F11 Key */
    VK_F12,	/* F12 Key */
    VK_F12,	/* F12 Key */
    0,	/* All other keys are undefined */
    0,	/* All other keys are undefined */
};

/* Handles the keyboard interrupt */
void keyboard_handler(struct regs *r)
{
    unsigned char scancode;
    scancode = inportb(0x60);
    unsigned char maskedCode = (scancode & ~0x80);
    if(bufferPosition < 128)
    {
		if(maskedCode < 128)
		{
			unsigned char key =  kbdus[(maskedCode * 2) + (Shift?1:0)];
			if((key == VK_ShiftR) | (key == VK_ShiftL))
			{
				Shift = ((scancode & 0x80) != 0) ? false : true;
				if(Shift)
				{
					modifiers |= KEYMOD_SHIFT;
				}
				else
				{
					modifiers &= ~KEYMOD_SHIFT;
				}
				return;
			}
			unsigned int pos = ((offset + bufferPosition) % 128);
			eventBuffer[pos].key = key;
			eventBuffer[pos].modifiers = modifiers;
			eventBuffer[pos].type = (scancode & 0x80) != 0 ? KEY_UP : KEY_DOWN;
			bufferPosition++;
		}
	}
}

/* Installs the keyboard handler into IRQ1 */
void keyboard_install()
{
	//zero out the buffer
	memset((void*)eventBuffer,0,sizeof(KeyEvent)*128);
	// set us up for success
	KeyboardLock = new Lock();
	bufferPosition = 0;
	//ok we are ready to handle key events now
    irq_install_handler(1, keyboard_handler);
}

namespace KEYBOARDSPACE
{
	bool Read(FDTEntry* fdt, unsigned char* buffer, size_t length)
	{
		//KeyboardLock->GetLock();
		switch(buffer[0])
		{
		// Get Key
		case 0:
			while(true)
			{
				*(KeyEvent*)buffer = getKeyEvent();
				if(((KeyEvent*)buffer)->type != KEY_NO_EVENT)
				{
					KeyboardLock->Release();
					return true;
				}
				KeyboardLock->Release();
				LunOS::System::Yield();
				KeyboardLock->GetLock();
			}
			break;
		// Try get key
		case 1:
			*(KeyEvent*)buffer = getKeyEvent();
			KeyboardLock->Release();
			return true;
		}
		KeyboardLock->Release();
		return true;
	}

	bool Open(FDTEntry* fdt, unsigned char* buffer, size_t length)
	{
		return true;
	}

	bool Init(struct Device* us, unsigned char* buffer, size_t length)
	{
		return true;
	}
	bool Close(FDTEntry* entry)
	{
		return true;
	}
}

using namespace KEYBOARDSPACE;

void keyboard_Init()
{
	// Now install a Keyboard
	Device us;
	us.AWrite = NULL;
	us.Write = NULL;
	us.Read = Read;
	us.ARead = Read;
	us.Shutdown = NULL;
	us.DeviceThread = NULL;
	us.Init = Init;
	us.Close = Close;
	us.Open = Open;
	us.type = LunOS::IO::DeviceTypes::DEVICE_KBD;
::System::InstallDevice(us);
}

//Gets a key event from the queue
LunOS::IO::KeyEvent getKeyEvent()
{
	LunOS::IO::KeyEvent temp = peekKeyEvent();
	if(bufferPosition > 0)
	{
		eventBuffer[offset].type = KEY_NO_EVENT;
		offset = (offset +1)%128;
		bufferPosition--;
	}
	return temp;
}

//Looks at the first key event on the queue
LunOS::IO::KeyEvent peekKeyEvent()
{
	LunOS::IO::KeyEvent temp;
	if(bufferPosition > 0)
	{
		temp = *((LunOS::IO::KeyEvent*)&eventBuffer[offset]);
	}
	else
	{
		temp.type = KEY_NO_EVENT;
		temp.key = NULL;
		temp.modifiers = modifiers;
	}
	return temp;
}

int nonCommand(LunOS::IO::KeyEvent* event){
	return ((event->key >= 32) & (event->key <= 126)) | (event->key == 10);
}

int alphaNumeric(LunOS::IO::KeyEvent* event){
	if((event->key >= 'a') & (event->key <= 'z')) return true;
	if((event->key >= 'A') & (event->key <= 'Z')) return true;
	if((event->key >= '0') & (event->key <= '9')) return true;
	return false;
}

char GetCharacter(LunOS::IO::KeyEvent* event)
{
	if(event->key < 32) return 0;
	if(event->key > 128) return 0;
	if((event->modifiers & KEYMOD_SHIFT) && ((event->key >= 'a') & (event->key <= 'z'))) return (event->key - 32);
	return event->key;
}
