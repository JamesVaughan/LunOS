/*
 * Mouse.cpp
 *
 *  Created on: 21-Jul-2009
 *      Author: james
 */

#include <io/Mouse.h>
#include <SysCall.h>

using namespace LunOS::IO;

Mouse* Mouse::GetMouseStream()
{
	unsigned int numberOfDevices = LunOS::System::GetNumberOfDevices();
	unsigned int i;
	for(i = 0; i < numberOfDevices; i++)
	{
		LunOS::IO::DeviceInfo info = LunOS::System::GetDeviceInfo(i);
		if(info.type == LunOS::IO::DeviceTypes::DEVICE_MOUSE)
		{
			return new Mouse(i,sizeof(LunOS::IO::MouseStatus));
		}
	}
	return NULL;
}

Mouse::Mouse(unsigned int fd, unsigned int bufferSize) : Stream(fd,bufferSize)
{}

MouseStatus Mouse::GetStatus()
{
	this->Read(sizeof(MouseStatus));
	return *((MouseStatus*)this->buffer);
}

Mouse::~Mouse()
{
	this->Close();
}
