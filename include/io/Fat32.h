/*
 * Fat32.h
 *
 *  Created on: 2010-10-11
 *      Author: james
 */

#ifndef FAT32_H_
#define FAT32_H_

namespace LunOS
{
	namespace IO
	{
		namespace FileSystems
		{
			class Fat32;
		}
	}
}

#include <io/FileSystem.h>

namespace LunOS
{
namespace IO
{
namespace FileSystems
{
	namespace Fat32Attributes
	{
		enum
		{
			NONE     = 0x00,
			READONLY = 0x01,
			HIDDEN   = 0x02,
			SYSTEM   = 0x04,
			VOLUMEID = 0x08,
			DIRECTORY= 0x10,
			ARCHIVE  = 0x20,
			LONGNAME = 0x0F
		};
	}
	struct ExtendedFat32PartitionInformation
	{
		unsigned int JumpCode : 24;
		unsigned char OEMName[8];
		unsigned short BytesPerSector;
		unsigned char SectorsPerCluster;
		unsigned short ReservedSectors;
		unsigned char TableCount;
		unsigned short NumberOfDirectories;
		unsigned short TotalSectorsSmall;
		unsigned char MediaType;
		unsigned short SectorsPerFatFAT16ONLY;
		unsigned short SectorsPerTrack;
		unsigned short NumberOfHeads;
		unsigned int HiddenSectors;
		unsigned int TotalSectors;

		//Extended part for FAT32
		unsigned int TableSize;
		unsigned short Flags;
		unsigned short Version;
		unsigned int RootCluster;
		unsigned short FSInfoCluster;
		unsigned short BackupCluster;
		unsigned char RESERVED[12];
		unsigned char DriveNumber;
		unsigned char NTFlags; // RESERVED
		unsigned char Signature;
		unsigned int VolumeID;
		unsigned char VolumeName[11];
		unsigned char FatTypeLabel[8];
	}__attribute__((packed));

	struct Fat32Directory
	{
		unsigned char ShortName[11];
		unsigned char Attributes;
		unsigned char ReservedNT;
		unsigned char CreationTimeTenthsOfSecond;
		unsigned char CreationTimeSecond : 5;
		unsigned char CreationTimeMinute : 6;
		unsigned char CreationTimeHour : 5;
		unsigned char CreationDay : 5;
		unsigned char CreationMonth : 4;
		unsigned char CreationYear : 7;
		unsigned char LastAccessedDay : 5;
		unsigned char LastAccessedMonth : 4;
		unsigned char LastAccessedYear : 7;
		unsigned short HighClusterClusterNumber;
		unsigned char ModifiedTimeSecond : 5;
		unsigned char ModifiedTimeMinute : 6;
		unsigned char ModifiedTimeHour : 5;
		unsigned char ModifiedDay : 5;
		unsigned char ModifiedMonth : 4;
		unsigned char ModifiedYear : 7;
		unsigned short LowClusterNumber;
		unsigned int FileLength;
	}__attribute__((packed));

	struct Fat32File
	{
		File FileInformation;
		unsigned long long CurrentPosition;
		unsigned long long StartSector;
		unsigned long long StartCluster;
		unsigned long long CurrentCluster;
		unsigned long long CurrentSector;
		unsigned int SectorsLeftInCluster;
	}__attribute__((packed));

	// The class that implements the Fat32 standard
	class Fat32 : public FileSystem
	{
		friend class LunOS::IO::FileSystem;
	public:
		void PrintFat32Information();
		// Used for testing
		void PrintFile(char* fileName);
		//
		virtual File* LoadFile(const char* fileName, unsigned int mode);
		// Load a directory
		virtual Directory* LoadDirectory(const char* fileName);
		// Get the Files contained in the given directory
		virtual unsigned int GetNextFileNameInDirectory(Directory* dir,	unsigned char* buffer, unsigned int bufferSize);
		// Read the given file
		virtual unsigned int ReadFile(File* file, unsigned char* buffer, unsigned int length);
		// Close the file
		virtual void CloseFile(File* file);
		// Close the directory
		virtual void CloseDirectory(Directory* dir);
	private:
		// Find a file in the file system with the given name
		bool FindFileEntry(unsigned char* fileName, Fat32Directory* entry, unsigned long long startingSector);
		int ToLBA(unsigned int head, unsigned int cylinder, unsigned int sector);
		unsigned long long GetNextCluster(unsigned long long currentCluster);
		Fat32(DiskAccess* disk, int partitionNumber, PartitionInformation information);
		~Fat32();
		unsigned int ClusterToSector(unsigned int clusterNumber);
		ExtendedFat32PartitionInformation Fat32Information;
		DiskAccess* Disk;
		unsigned long long BaseAddress;
		bool LBAMode;
		unsigned long long RootDirectorySector;
	};
}
}
}

#endif /* FAT32_H_ */
