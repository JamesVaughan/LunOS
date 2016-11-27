#include <SysCall.h>
#include <kern/system.hpp>
#include <kern/io/mouse.h>
#include <io/Mouse.h>

using namespace LunOS::IO;
using namespace LunOS::IO::Kernel;

unsigned char MouseDriver::MessagePosition = 0;
unsigned char MouseDriver::Message[4];
unsigned int MouseDriver::LastMessage;
int MouseDriver::X = 0;
int MouseDriver::Y = 0;

bool MouseDriver::Button[5];

void TimeoutWait(bool data)
{
	int timeout = 10000;
	while (timeout > 0)
	{
		unsigned char status = inportb(0x64);
		if (data ? ((status & 0x1) == 1) : ((status & 0x2) == 0)) break;
		timeout--;
	}
}

void MouseDriver::Wait(bool data)
{
	TimeoutWait(data);
}

void MouseDriver::Write(unsigned char data)
{
	Wait(false);
	outportb(0x64, 0xD4);
	Wait(false);
	outportb(0x60, data);
}

unsigned char MouseDriver::Read()
{
	Wait(true);
	return inportb(0x60);
}

unsigned char ReadStatus()
{
	const unsigned short CommandPort = 0x64;
	outportb(CommandPort, 0x20);
	TimeoutWait(true);
	return inportb(CommandPort);
}

void MouseDriver::Init()
{
	const unsigned short DataPort = 0x60;
	const unsigned short CommandPort = 0x64;
	MouseDriver::MessagePosition = 0;
	MouseDriver::Message[0] = 0;
	MouseDriver::Message[1] = 0;
	MouseDriver::Message[2] = 0;
	MouseDriver::Button[0] = false;
	MouseDriver::Button[1] = false;
	MouseDriver::Button[2] = false;
	MouseDriver::Button[3] = false;
	MouseDriver::Button[4] = false;
	MouseDriver::LastMessage = 3;

	unsigned char status = 0x0;
	
	//Enable the second PS/2 channel
	/*outportb(CommandPort, 0xA8);
	Wait(false);
	// lets us handle IRQ12 for mouse operation
	irq_install_handler(12, InterruptHandler);
	// enable IRQ for both
	status = ReadStatus();
	printf("Initial Status = %20x%x\n", status);
	outportb(CommandPort, 0x60);
	Wait(false);
	status |= 0x3;
	outportb(DataPort, status);
	Wait(false);
	status = ReadStatus();
	printf("Post-Status = %20x%x\n", status);
	*/

	// Now install a Mouse Driver
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
	us.type = LunOS::IO::DeviceTypes::DEVICE_MOUSE;
::System::InstallDevice(us);
}

void MouseDriver::InterruptHandler(struct regs* r)
{
	printf("IRQ12\n");
	// This gets called when something triggers IRQ12 (the mouse IRQ)
	unsigned char input = inportb(0x60);
	//if(input > 0xF0) return;
	MouseDriver::Message[MouseDriver::MessagePosition++] = input;
	// now that we have all of the information save
	if(MouseDriver::MessagePosition == MouseDriver::LastMessage)
	{
		MouseDriver::MessagePosition = 0;
		// check for overflow
		if (Message[0] & 0xC0)
		{
			return;
		}
		MouseDriver::Button[0] = Message[0] & 0x1;
		MouseDriver::Button[1] = Message[0] & 0x2;
		MouseDriver::Button[2] = Message[0] & 0x4;
		int xdelta = (char)Message[1];
		int ydelta = -(char)Message[2];

		// Basic testing
		MouseDriver::X += xdelta;
		MouseDriver::Y += ydelta;
		if(MouseDriver::X < 0) MouseDriver::X = 0;
		else if(MouseDriver::X >= 800) MouseDriver::X = 799;

		if(MouseDriver::Y < 0) MouseDriver::Y = 0;
		else if(MouseDriver::Y >= 600) MouseDriver::Y = 599;

		if(MouseDriver::LastMessage == 4)
		{
			// Then we have the additional information packet here
		}
	}
}

bool MouseDriver::Read(FDTEntry* fdt, unsigned char* buffer, size_t length)
{
	int i;
	MouseStatus *status = (MouseStatus*)buffer;
	status->X = MouseDriver::X;
	status->Y = MouseDriver::Y;
	for(i = 0; i < 5; i++)
	{
		status->Buttons[i] = MouseDriver::Button[i];
	}
	return true;
}

bool MouseDriver::Write(FDTEntry* fdt, unsigned char* buffer, size_t length)
{
	return false;
}

bool MouseDriver::Init(Device* dev, unsigned char* buf, size_t size)
{
	return true;
}

bool MouseDriver::Open(FDTEntry* entry, unsigned char* buffer, size_t length)
{
	return true;
}

bool MouseDriver::Close(FDTEntry* entry)
{
	return true;
}
