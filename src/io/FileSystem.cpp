/*
 * FileSystem.cpp
 *
 *  Created on: 2010-10-02
 *      Author: james
 */

#include <io/FileSystem.h>
#include <io/Fat32.h>
#include <kern/console.h>
using namespace LunOS;
using namespace LunOS::IO;
using namespace LunOS::IO::FileSystems;

FileSystem* FileSystem::GetFileSystem(DiskAccess* disk, int partitionNumber)
{
	PartitionInformation info = FileSystem::GetPartitionInformation(disk, partitionNumber);
	switch(info.FileSystemNumber)
	{
	// FAT32 CHS or no information is provided for partitions
		case 0:
		case 11:
		{
			Fat32* system = new Fat32(disk, partitionNumber, info);
			return system;
		}
		break;
		default:
		{
			printf("Failed to read File System, information dump\n");
			printf("Boot  :(%i)\n", info.BootIndicator);
			printf("ID    :(%i)\n", info.FileSystemNumber);
			printf("StartS:(%i)\n", info.StartingSector);
			printf("#Sect :(%i)\n", info.NumberOfSectors);
		}
		break;
	}
	return 0;
}

PartitionInformation FileSystem::GetPartitionInformation(DiskAccess* disk, int partitionNumber)
{
	unsigned char buff [512];
	// Read sector 1, I think this is the right 1
	disk->ReadDisk(buff,1,0);
	return *((PartitionInformation*)(buff + 0x1BE + (16 * (partitionNumber - 1))));
}

FileSystem::FileSystem(DiskAccess* disk)
{
	this->Disk = disk;
	this->NumberOfSectors = 0;
	this->PartitionNumber = 0;
}

FileSystem::~FileSystem()
{
}

void FileSystem::Close()
{
	if(this)
	{
		delete this;
	}
}

