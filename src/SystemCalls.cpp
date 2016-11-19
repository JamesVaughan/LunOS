/*
 * SystemCalls.cpp
 *
 *  Created on: 11-Nov-2008
 *      Author: phibred
 */
#include <SysCall.h>
#include <kern/system.hpp>
namespace LunOS
{
	void System::Yield()
	{
		System::Call(0, LunOS::Calls::Yield, NULL, NULL);
	}

	void System::KillThread()
	{
		System::Call(0,LunOS::Calls::KillThread,NULL,0);
	}

	unsigned int System::GetUserID()
	{
		unsigned int temp = 0;
		System::Call(0,LunOS::Calls::GetUserID,(unsigned char*)&temp,sizeof(unsigned int));
		return temp;
	}

	// User must delete the reference given
	const char* System::GetUserName(unsigned int uid)
	{
		const char* temp = (const char*)new char[80];
		*(unsigned int*)temp = uid;
		System::Call(0,LunOS::Calls::GetUserName,(unsigned char*)temp,80);
		return temp;
	}

	const char* System::GetUserName()
	{
		return GetUserName(GetUserID());
	}

	unsigned int* System::GetActiveUsers()
	{
		unsigned int* temp = new unsigned int[1025];
		System::Call(0,LunOS::Calls::GetUsers,(unsigned char*)temp,1025);
		return temp;
	}

	unsigned int System::CreateNewUser(const char* name, const char* password, const char* yourPassword)
	{
		int i;
		int sizeName = strlen(name) + 1, sizepass = strlen(password) + 1, sizeOurPass = strlen(yourPassword) + 1;
		char* temp = new char[sizeName + sizepass + sizeOurPass];
		memcpy(temp, name ,sizeName);
		memcpy(temp + sizeName,password,sizepass);
		memcpy(temp + (sizeName + sizepass),yourPassword,sizeOurPass);
		Call(0,Calls::CreateUser,(unsigned char*)temp,sizeName + sizepass + sizeOurPass);
		i = *(unsigned int*)temp;
		delete[] temp;
		return i;
	}

	unsigned int System::CreateThread(unsigned char* name, unsigned int(*startingAddress)(void*), void* params)
	{
		unsigned int amountOfData = 64 + (sizeof(char*) * 2);
		unsigned char buff[amountOfData];
		unsigned int* intBuff = ((unsigned int*)buff);
		int i;
		for(i = 0; i < 63; i++)
		{
			if(name[i] == 0)
				break;
			buff[i] = name[i];
		}
		// now fill the rest with 0's
		for(; i < 64; i++)
		{
			buff[i] = 0;
		}
		*((unsigned int*)(buff + 64)) = (unsigned int)startingAddress;
		*((unsigned int*)(buff + 64) + 1) = (unsigned int)params;
		Call(0,Calls::CreateThread,(unsigned char*)buff, amountOfData);
		return intBuff[0];
	}

	unsigned int System::GetNumberOfDevices()
	{
		unsigned int buff;
		Call(0,Calls::GetNumberOfDevices,(unsigned char*)&buff,4);
		return buff;
	}

	LunOS::IO::DeviceInfo System::GetDeviceInfo(unsigned int deviceNumber)
	{
		LunOS::IO::DeviceInfo dev;
		*((unsigned int*)&dev) = deviceNumber;
		Call(0,Calls::GetDeviceInfo,(unsigned char*)&dev,sizeof(LunOS::IO::DeviceInfo));
		return dev;
	}

	void System::Sleep(unsigned int milliseconds)
	{
		Call(0,Calls::Sleep,NULL,milliseconds);
	}
}
