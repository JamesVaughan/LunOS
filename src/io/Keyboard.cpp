/*
 * Keyboard.cpp
 *
 *  Created on: 21-Jul-2009
 *      Author: james
 */

#include <io/Keyboard.h>
#include <kern/console.h>
#include <SysCall.h>

using namespace LunOS::IO;

Keyboard* Keyboard::GetKeyboardStream()
{
	unsigned int numberOfDevices = LunOS::System::GetNumberOfDevices();
	unsigned int i;
	for(i = 0; i < numberOfDevices; i++)
	{
		LunOS::IO::DeviceInfo info = LunOS::System::GetDeviceInfo(i);
		if(info.type == LunOS::IO::DeviceTypes::DEVICE_KBD)
		{
			return new Keyboard(i,sizeof(LunOS::IO::KeyEvent) * 4);
		}
	}
	return NULL;
}

Keyboard::Keyboard(unsigned int fd, unsigned int bufferSize) : Stream(fd,bufferSize)
{}

Keyboard::~Keyboard()
{
	this->Close();
}

LunOS::IO::KeyEvent Keyboard::ReadKeyEvent()
{
	this->buffer[0] = 0; // Get Char
	this->buffer[1] = 0; // Get Char
	this->buffer[2] = 0; // Get Char
	this->Read(sizeof(LunOS::IO::KeyEvent));
	return *((KeyEvent*)(this->buffer));
}

LunOS::IO::KeyEvent Keyboard::TryReadKey()
{
	this->buffer[0] = 1; // Try get char
	this->Read(sizeof(LunOS::IO::KeyEvent));
	return *((KeyEvent*)(this->buffer + 0));
}




