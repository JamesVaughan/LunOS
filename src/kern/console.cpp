#include <kern/system.hpp>
#include <String.h>

//The colour we are writing in
unsigned char colour;
//stored values for types of text
unsigned char textColour, infoColour, warningColour, alternateColour;
//The colour to use for rendering the top bar
unsigned char titleColour;

const unsigned char* consoleName = (const unsigned char*)"LunOS-Console ";

unsigned char* consoleScreenStart = (unsigned char*) 0xb8000 + 160;
unsigned char* consoleScreenEnd = (unsigned char*) (0xb8000 + 80*50); //25x2

volatile unsigned char x,y;

Lock* printfLock;

void renderCursor()
{
    unsigned temp;
    temp = y * 80 + x + 80; // x and y are in offset to a 80x24 window
    outportb(0x3D4, 14);
    outportb(0x3D5, temp >> 8);
    outportb(0x3D4, 15);
    outportb(0x3D5, temp);
}

void scrollConsole()
{
	memcpy(consoleScreenStart,consoleScreenStart+160,160*23);
	memsetw((unsigned short*)(consoleScreenEnd-160),0x0F00,80);
	y--;
}

void renderTopBar()
{
	int i = 0;
	const unsigned char* c = consoleName;
	unsigned char seconds, minutes, hours;
	char temp [3];
	//char convert[12];
	for(;*c;c++)
	{
		*((char*)(0xb8000 + (i<<1))) = *c;
		*(char*)(0xb8000 + (i<<1) + 1) = titleColour;
		i++;
	}
	/*uintToChar(y,convert);
	c = (const unsigned char*)convert;
	for(;*c;c++)
		{
			*((char*)(0xb8000 + (i<<1))) = *c;
			*(char*)(0xb8000 + (i<<1) + 1) = titleColour;
			i++;
		}
		*/
	for(;i < 80 - 8;i++)
	{
		*(char*)(0xb8000 + (i<<1)) = 0;
		*(char*)(0xb8000 + (i<<1) + 1) = titleColour;
	}

	outportb(0x70,0x04);
	hours = inportb(0x71);
	ucharDEHDecode(hours,temp);
	*(char*)(0xb8000 + (i<<1)) = temp[0];
	*(char*)(0xb8000 + (i++<<1) + 1) = titleColour;
	*(char*)(0xb8000 + (i<<1)) = temp[1];
	*(char*)(0xb8000 + (i++<<1) + 1) = titleColour;

	*(char*)(0xb8000 + (i<<1)) = ':';
	*(char*)(0xb8000 + (i++<<1) + 1) = titleColour;

	outportb(0x70,0x02);
	minutes = inportb(0x71);
	ucharDEHDecode(minutes,temp);
	*(char*)(0xb8000 + (i<<1)) = temp[0];
	*(char*)(0xb8000 + (i++<<1) + 1) = titleColour;
	*(char*)(0xb8000 + (i<<1)) = temp[1];
	*(char*)(0xb8000 + (i++<<1) + 1) = titleColour;

	*(char*)(0xb8000 + (i<<1)) = ':';
	*(char*)(0xb8000 + (i++<<1) + 1) = titleColour;

	outportb(0x70,0x00);
	seconds = inportb(0x71);
	ucharDEHDecode(seconds,temp);
	*(char*)(0xb8000 + (i<<1)) = temp[0];
	*(char*)(0xb8000 + (i++<<1) + 1) = titleColour;
	*(char*)(0xb8000 + (i<<1)) = temp[1];
	*(char*)(0xb8000 + (i++<<1) + 1) = titleColour;
}

void updateClock()
{
	renderTopBar();
}

void clearConsole()
{
	unsigned char* i;
	printfLock->GetLock();
	for(i = (unsigned char*) consoleScreenStart; i < consoleScreenEnd; i+=2)
	{
		*i = 0;
		*(i+1) = 0x7;
	}
	x = y = 0;
	renderCursor();
	printfLock->Release();
}

void writeConsoleText(const char* text)
{
	while(*text)
	{
		putchConsoleText(*(text++));
	}
}

void writeConsoleInformation(const char* text)
{
	while(*text)
	{
		putchConsoleInformation(*(text++));
	}
}

void writeConsoleWarning(const char* text)
{
	while(*text)
	{
		putchConsoleWarning(*(text++));
	}
}

void putch(const char text){
	switch(text)
	{
		case '\n':
			if(++y >= 24)
			{
				scrollConsole();
			}
			x = 0;
			break;
		// fall through
		case '\r':
			x = 0;
			break;
		case '\t':
			x += 4;
			break;
		default:
		*(unsigned short *)(consoleScreenStart + (y * 80 + x)*2) = text;
		*(unsigned short *)(consoleScreenStart + (y * 80 + x)*2 + 1) = colour;
		x++;
		if(x >= 80)
		{
			x = 0;
			if(++y >= 24)
			{
				scrollConsole();
			}
		}
		break;
	}
	renderCursor();
}

void litteralputch(const char text)
{
	*(unsigned short *)(consoleScreenStart + (y * 80 + x)*2) = text;
	*(unsigned short *)(consoleScreenStart + (y * 80 + x)*2 + 1) = colour;
	x++;
	if(x >= 80)
	{
		x = 0;
		if(++y >= 24)
		{
			scrollConsole();
		}
	}
	renderCursor();
}

//write text to the console
void putchConsoleText(const char text){
	colour = textColour;
	putch(text);
}
//write information
void putchConsoleInformation(const char text){
	colour = infoColour;
	putch(text);
}
//write a warning
void putchConsoleWarning(const char text)
{
	colour = warningColour;
	putch(text);
}

void setConsoleTextColour(const char colourz)
{
	textColour = colourz;
}

void setConsoleInfoColour(const char colourz)
{
	infoColour = colourz;
}

void setConsoleWarningColour(const char colourz)
{
	warningColour = colourz;
}

void initConsole(){
	colour = textColour = 0x0F;
	alternateColour = 0x0E;
	infoColour = 0x0A;
	warningColour = 0x0C;
	titleColour = 0x2F;
	renderTopBar();
	clearConsole();
}

void initConsolePhase2()
{
	printfLock = new Lock();
}


//It is just printf... %1 = text %2 = info %3= warning
void printf(const char* s, ...)
{
	char** arg = (char**)&s;
	char p;
	//Used for doing integer to text conversions
	char buff[12];
	colour = textColour;
	//ignore the format, just pointer to the ellipsis
	arg++;
	printfLock->GetLock();
	//while the format has more data
	while((p = *s++))
	{
		if(p == '%')
		{
			char* temp;
			switch((p = *s++))
			{
				case '1':
					colour = textColour;
					break;
				case '2':
					colour = infoColour;
					break;
				case '3':
					colour = warningColour;
					break;
				case '4':
					colour = alternateColour;
					break;
				case 'i':
					uintToChar((unsigned int)*(arg++),buff);
					temp = buff;
					while(*temp)
					{
						putch(*temp);
						temp++;
					}
					break;
				case 'x':
					uintToCharX((unsigned int)*(arg++),buff);
					temp = buff;
					while(*temp)
					{
						putch(*temp);
						temp++;
					}
				break;
				case 's':
					temp = *(arg++);
					while(*temp)
					{
						putch(*temp);
						temp++;
					}
					break;
				case 'c':
				{
					unsigned char val = *((unsigned char*)arg);
					litteralputch(val);
					arg++;
					break;

				}
				default:
				putch(p);
				break;
			}
		}
		else
		{
			putch(p);
		}
	}
	printfLock->Release();
}


void updateCursor(unsigned char mx, unsigned char my){
	printfLock->GetLock();
	x = mx;
	y = my;
	renderCursor();
	printfLock->Release();
}

void getCursorPosition(unsigned char* mx, unsigned char* my){
	printfLock->GetLock();
	*mx = x;
	*my = y;
	printfLock->Release();
}

