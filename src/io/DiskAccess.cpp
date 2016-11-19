#include <io/DiskAccess.h>
#include <SysCall.h>
#include <kern/system.hpp>

using namespace LunOS::IO;

DiskInfo LunOS::IO::DiskAccess::GetDiskInformation()
{
	unsigned short* sBuffer = (unsigned short*)this->buffer;
	sBuffer[0] = 2;
	sBuffer[1] = (unsigned short)this->StreamedDisk;
	this->Read(sizeof(DiskInfo));
	return *((DiskInfo*)this->buffer);
}

DiskAccess* LunOS::IO::DiskAccess::GetDiskStream(unsigned int disk)
{
	unsigned int numberOfDevices = LunOS::System::GetNumberOfDevices();
	unsigned int i;
	for(i = 0; i < numberOfDevices; i++)
	{
		LunOS::IO::DeviceInfo info = LunOS::System::GetDeviceInfo(i);
		if(info.type == LunOS::IO::DeviceTypes::DEVICE_HDD)
		{
			// Create a 4k buffer
			return new DiskAccess(disk, i, 512 * 8);
		}
	}
	printf("%3FAILED TO FIND A HDD DEVICE!\n");
	return NULL;
}

LunOS::IO::DiskAccess::DiskAccess(unsigned int disk, unsigned int fd, unsigned int bufferSize) : Stream(fd,bufferSize)
{
	this->StreamedDisk = disk;
	memset(this->buffer, 0, this->bufferLength);
}

LunOS::IO::DiskAccess::~DiskAccess()
{
	this->Close();
}

void LunOS::IO::DiskAccess::ReadDisk(unsigned char* buffer, unsigned int sectors, unsigned int LBA)
{
	unsigned short* streamInput = (unsigned short*)this->buffer;
	unsigned int ammountOfData = (sectors * 512);
	while(ammountOfData > 0)
	{
		unsigned int ammountThisItteration = (ammountOfData < this->bufferLength ? ammountOfData : this->bufferLength);
		unsigned int sectorsToRead = (ammountThisItteration / 512) + (ammountThisItteration % 512 ? 1 : 0);
		// do a read command
		streamInput[0] = 0;
		// the number of sectors
		streamInput[1] = sectorsToRead;
		// target the given disk
		streamInput[2] = this->StreamedDisk; // which disk to read from
		*(unsigned int*)(streamInput + 3) = LBA; // the location in LBA to read from
		this->Read(ammountThisItteration);
		// Copy the data to the user's buffer
		memcpy(buffer, this->buffer, ammountThisItteration);
		buffer += ammountThisItteration;
		LBA += sectorsToRead;
		ammountOfData -= ammountThisItteration;
	}
}

