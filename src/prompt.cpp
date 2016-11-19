#include <prompt.h>
#include <io/Keyboard.h>
#include <kern/system.hpp>
#include <SysCall.h>
#include <io/FileSystem.h>
using namespace LunOS;
typedef struct{
	unsigned int size;
	unsigned int position;
	unsigned char buffer[160];
} InputBuffer;


CommandList commands;

unsigned char starty;
InputBuffer buffer;
void CommandPrompt::Init()
{
	//might do something here later
	int i = 0;
	for(;i < 160; i++){
		buffer.buffer[i] = 0;
	}
	unsigned char x,y;
	getCursorPosition(&x,&y);
	starty = y;
}

void CommandPrompt::ResetPosition()
{
	unsigned char x,y;
	getCursorPosition(&x,&y);
	starty = y;
}


void CommandPrompt::AddCommand(Command* c)
{
	if(commands.ammount < 80)
	{
		commands.list[commands.ammount].params = c->params;
		if(c->params)
		{
			commands.list[commands.ammount].paramCommand = c->paramCommand;
		}
		else
		{
			commands.list[commands.ammount].command = c->command;
		}
		commands.list[commands.ammount++].name = c->name;

	}
}

int getParamNumber(String* s)
{
	unsigned int i,total = 0;
	bool isWhitespace = false;
	unsigned char* c = s->toCharArray();
	unsigned int length = s->GetLength();
	unsigned int useless = 0;
	for(i = 0; i < length; i++)
	{
		if(c[i] == ' ' && !isWhitespace)
		{
			total++;
		}
		else
		{
		    useless++;
		}
		isWhitespace = (c[i] == ' ');
	}
	if(!isWhitespace)
	{
	    total++;
	}
	return total - 1;
}

void AssignPlaces(unsigned char** &argv, String* s)
{
	unsigned int i,pos = 0;
	bool isWhitespace = false;
	unsigned char* c = s->toCharArray();
	for(i = 0; i < s->GetLength(); i++){
		// if last time we had whitespace and now we don't
		if((isWhitespace) & (c[i] != ' ')){
			//then this is the start of a new argument
			argv[pos++] = (unsigned char*)(c + i);
			isWhitespace = false;
		}
		else if(c[i] == ' ' && !isWhitespace && i > 0){
			c[i] = 0;
			isWhitespace = true;
		}
		else
		{
			isWhitespace = (c[i] == ' ');
		}
	}
}

void PrintCommands()
{
	printf("\nCommands:");
	unsigned int i;
	for(i = 0; i < commands.ammount; i++)
	{
		printf("\n%2%s",commands.list[i].name.toCharArray());
	}
}

void launchCommand(){
	unsigned int i;
	String buf;
	buf = (unsigned char*)buffer.buffer;
	if(buf == (unsigned char*)"help")
	{
		PrintCommands();
		return;
	}
	if(!(buf == (unsigned char*)""))
	{
		for(i = 0; i < commands.ammount; i++)
		{
			//printf("%s == %s?\n",buf.toCharArray(),commands.list[i].name.toCharArray());
			if(buf.StartsWith(&commands.list[i].name))
			{
				printf("\n");
				//run the command
				if(commands.list[i].params)
				{
					int numParams = getParamNumber(&buf);
					unsigned char** argv = new unsigned char*[numParams];
					AssignPlaces(argv,&buf);
					commands.list[i].paramCommand(numParams, argv);
					delete argv;
				}
				else{
					commands.list[i].command();
				}
				return;
			}
		}
		printf("%3\nCommand not found.");
	}
}

LunOS::IO::Keyboard* keyboard = NULL;
void CommandPrompt::Run(){
	if(keyboard == NULL)
	{
		keyboard = keyboard->GetKeyboardStream();
	}
	LunOS::IO::KeyEvent k = keyboard->ReadKeyEvent();
	//only do stuff if it was from a key down
	if(k.type == KEY_DOWN)
	{
		unsigned char x,y;
		unsigned int i;
		switch(k.key){
		case VK_Backspace:
			if(buffer.size > 0)
			{
				buffer.position--;
				for(i = buffer.position; i < buffer.size; i++)
				{
					buffer.buffer[i] = buffer.buffer[i+1];
				}
				buffer.size--;
				if(buffer.position == 0){
					buffer.buffer[buffer.position] = ' ';
					updateCursor(0,starty);
					printf("%s", buffer.buffer);
					buffer.buffer[buffer.size] = 0;
					getCursorPosition(&x,&y);
					updateCursor(x-1,y);
					buffer.buffer[buffer.position] = 0;
				}
				else
				{
					updateCursor(0,starty);
					printf("%s", buffer.buffer);
					x = buffer.position%80;
					y = starty + buffer.position/80;
					updateCursor(x,y);
				}
			}
			break;
		case VK_Delete:
			if(buffer.position < buffer.size){
				for(i = buffer.position; i < buffer.size; i++){
				buffer.buffer[i] = buffer.buffer[i+1];
				}
				buffer.size--;
				if(buffer.position == buffer.size){
					buffer.buffer[buffer.position] = ' ';
					updateCursor(0,starty);
					printf("%s", buffer.buffer);
					buffer.buffer[buffer.size] = 0;
					getCursorPosition(&x,&y);
					updateCursor(x-1,y);
					buffer.buffer[buffer.position] = 0;
				}
				else{
					updateCursor(0,starty);
					printf("%s", buffer.buffer);
					x = buffer.position%80;
					y = starty + buffer.position/80;
					updateCursor(x,y);
				}
			}
			break;
		case VK_Enter:
			buffer.buffer[buffer.size] = 0;
			launchCommand();
			getCursorPosition(&x,&y);
			starty = y+1;
			for(i = 0; i < 160; i++){
				buffer.buffer[i] = 0;
			}
			buffer.size = buffer.position = 0;
			while(starty + buffer.size/80 >= 23){
				starty--;
				putchConsoleText('\n');
			}
			updateCursor(0,starty);
			break;
		case VK_Left:
			if(buffer.position > 0){
					buffer.position--;
					x = buffer.position%80;
					y = starty + (buffer.position/80);
					updateCursor(x,y);
			}
			break;
		case VK_Right:
			if(buffer.position < buffer.size){
				buffer.position++;
				x = buffer.position%80;
				y = starty + (buffer.position/80);
				updateCursor(x,y);
			}
			break;
		default:
			if(nonCommand(&k) && buffer.size < 159){

				buffer.buffer[buffer.position] = GetCharacter(&k);
				buffer.position = (buffer.position == buffer.size)?
						++buffer.size : buffer.position + 1;
				if(starty + buffer.size/80 >= 23){
					updateCursor(0,starty+1);
					for(i = 0; i < buffer.size - 1; i++){
						putchConsoleText(' ');
					}
					do{
						starty--;
						putchConsoleText('\n');
					}while(starty + buffer.size/80 > 23);
				}
				updateCursor(0,starty);
				printf("%s",buffer.buffer);
				x = buffer.position%80;
				y = starty + (buffer.position/80);
				updateCursor(x,y);
			}
			break;
		}
	}
}

