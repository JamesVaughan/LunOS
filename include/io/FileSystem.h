/*
 * FileSystem.h
 *
 *  Created on: 2010-10-02
 *      Author: james
 */

#ifndef FILESYSTEM_H_
#define FILESYSTEM_H_

#include <io/Stream.h>
#include <io/DiskAccess.h>
#include <io/Io.h>

namespace LunOS
{
	namespace IO
	{
		struct PartitionInformation
		{
			unsigned char BootIndicator;
			unsigned char StartingHeadNumber;
			unsigned char StartingSector : 6;
			unsigned short StartingCylinderNumber : 10;
			unsigned char FileSystemNumber;
			unsigned char EndHeadNumber;
			unsigned char EndSector : 6;
			unsigned short EndCylinderNumber : 10;
			unsigned int  NumberOfSectorsBefore;
			unsigned int NumberOfSectors;
		}__attribute__((packed));

		struct Directory
		{
			unsigned long long Files;
			unsigned char Name[128];
		};

		struct File
		{
			unsigned long long Length;
			unsigned long long CurrentPosition;
			unsigned char Name[128];
		}__attribute__((packed));

		class FileSystem
		{
		public:
			static FileSystem* GetFileSystem(DiskAccess* disk, int partitionNumber);
			static PartitionInformation GetPartitionInformation(DiskAccess* disk, int partitionNumber);
			// Read from the connected disk
			void ReadDisk(unsigned char* buffer, unsigned int sectors, unsigned int LBA);
			// Returns a file
			virtual File* LoadFile(const char* fileName, unsigned int mode) = 0;
			// Load a directory
			virtual Directory* LoadDirectory(const char* fileName) = 0;
			// Read data from the given file
			virtual unsigned int ReadFile(File* file, unsigned char* buffer, unsigned int length) = 0;
			// Get the Files contained in the given directory
			virtual unsigned int GetNextFileNameInDirectory(Directory* dir,	unsigned char* buffer, unsigned int bufferSize) = 0;
			// Close the file
			virtual void CloseFile(File* file) = 0;
			// Close the directory
			virtual void CloseDirectory(Directory* dir) = 0;
			// Close the connection to the file system (Do not call after this is called!)
			virtual void Close();
		protected:
			DiskAccess* Disk;
			unsigned int PartitionNumber;
			unsigned long long NumberOfSectors;
			FileSystem(DiskAccess* disk);
			virtual ~FileSystem();
		};
	}
}


#endif /* FILESYSTEM_H_ */
