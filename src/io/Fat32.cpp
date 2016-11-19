#include <kern/system.hpp>
#include <io/DiskAccess.h>
#include <io/FileSystem.h>
#include <io/Fat32.h>
#include <kern/console.h>
#include <io/Keyboard.h>
#include <SysCall.h>

#ifndef NULL
#define NULL 0
#endif

using namespace LunOS;
using namespace LunOS::IO;
using namespace LunOS::IO::FileSystems;



Fat32::Fat32(DiskAccess* disk, int partitionNumber, PartitionInformation information) : FileSystem(disk)
{
	unsigned char buff [512];
	ExtendedFat32PartitionInformation* buffPointer = (ExtendedFat32PartitionInformation*)buff;
	this->Disk = disk;
	this->LBAMode = true;
	// Read sector 1, I think this is the right 1
	this->Disk->ReadDisk(buff,1,information.NumberOfSectorsBefore);
	this->Fat32Information = *buffPointer;
	this->BaseAddress = information.NumberOfSectorsBefore;
	unsigned int firstDataSector = this->Fat32Information.TableCount * this->Fat32Information.TableSize
			+ this->Fat32Information.ReservedSectors + this->BaseAddress;
	this->RootDirectorySector = firstDataSector;
}

Fat32::~Fat32()
{

}

int Fat32::ToLBA(unsigned int head, unsigned int cylinder, unsigned int sector)
{
	return 0;
}

namespace LunOS
{
	namespace IO
	{
		namespace FileSystems
		{
			namespace Fat32Methods
			{
				bool IsNonWhitespace(unsigned char c)
				{
					// everything above 32 is not white space
					return c > 32;
				}
			}
		}
	}
}

using namespace LunOS::IO::FileSystems::Fat32Methods;


void Fat32::PrintFile(char* fileName)
{
	File* file = this->LoadFile(fileName, 0);
	if(!file)
	{
		printf("%3File not found!\n");
		return;
	}
	unsigned int i,j;
	unsigned int read;
	unsigned char buff[512];
	for(i = 0; i < file->Length;)
	{
		if(!(read = this->ReadFile(file, buff, 512)))
		{
			break;
		}
		for(j = 0; j < read; j++)
		{
			printf("%c", buff[j]);
			i++;
		}
	}
	this->CloseFile(file);
}

unsigned int Fat32::ClusterToSector(unsigned int clusterNumber)
{
	return (clusterNumber - 2) * this->Fat32Information.SectorsPerCluster + this->RootDirectorySector;
}

void Fat32::PrintFat32Information()
{
	int i;
	Keyboard* keys = Keyboard::GetKeyboardStream();

	// Extended information
	printf("%4Disk Information\n");
	printf("%2OEM Name             %1:");
	for(i = 0; i < 8; i++)
	{
		printf("%2%c",this->Fat32Information.OEMName[i]);
	}
	printf("\n");
	printf("%2BackupCluster        %1:%2%i\n", this->Fat32Information.BackupCluster);
	printf("%2Drive Number         %1:%2%i\n", this->Fat32Information.DriveNumber);
	printf("%2FSInfoCluster        %1:%2%i\n", this->Fat32Information.FSInfoCluster);
	printf("%2Flags                %1:%2%i\n", this->Fat32Information.Flags);
	printf("%2NTFlags              %1:%2%i\n", this->Fat32Information.NTFlags);
	printf("%2Signature            %1:%2%i\n", this->Fat32Information.Signature);
	printf("%2Version              %1:%2%i\n", this->Fat32Information.Version);
	printf("%2Volume ID            %1:%2%i\n", this->Fat32Information.VolumeID);
	printf("%2VolumeName           %1:");
	for(i = 0; i < 11; i++)
	{
		printf("%2%c",this->Fat32Information.VolumeName[i]);
	}
	printf("\n");

	printf("Press Any Key to Continue... ");
	while(keys->TryReadKey().type != KEY_NO_EVENT);
	LunOS::System::Sleep(200);
	while(keys->ReadKeyEvent().type != KEY_DOWN);
	printf("\n");

	printf("%2NumberOfHeads        %1:%2%i\n", this->Fat32Information.NumberOfHeads);
	printf("%2SectorsPerCluster    %1:%2%i\n", this->Fat32Information.SectorsPerCluster);
	printf("%2SectorsPerTrack      %1:%2%i\n", this->Fat32Information.SectorsPerTrack);
	printf("%2ReservedSectors      %1:%2%i\n", this->Fat32Information.ReservedSectors);
	printf("%2HiddenSectors        %1:%2%i\n", this->Fat32Information.HiddenSectors);
	printf("%2RootCluster          %1:%2%i\n", this->Fat32Information.RootCluster);
	printf("%2TableSize            %1:%2%i\n", this->Fat32Information.TableSize);



	unsigned char buff [512];
	unsigned int pos = this->Fat32Information.BytesPerSector, currentSector = this->RootDirectorySector;
	int j;
	printf("%4Accessing Root Directories, Total of %i entries\nStarting at %i\n",
			this->Fat32Information.NumberOfDirectories, this->RootDirectorySector);

	printf("Press Any Key to Continue... ");
	while(	keys->TryReadKey().type != KEY_NO_EVENT);
	LunOS::System::Sleep(200);
	while(keys->ReadKeyEvent().type != KEY_DOWN);
	printf("\n");

	while(true)
	{
		if(pos >= this->Fat32Information.BytesPerSector)
		{
			pos = 0;
			this->Disk->ReadDisk(buff, 1, currentSector);
			currentSector++;
		}
		Fat32Directory dir = *((Fat32Directory*)(buff + pos));
		if(dir.Attributes == Fat32Attributes::LONGNAME)
		{
			// Long name entry
			//printf("%2Long Name Entry!\n");
		}
		else if(dir.ShortName[0] == 0x00)
		{
			// No more entries
			printf("%3End of Entries!\n");
			break;
		}
		else if(dir.ShortName[0] == 0xE5)
		{
			// Empty entry
			//printf("%2Empty Entry!\n");
		}
		else
		{
			printf("%2Name%1:");
			for(j = 0; j < 8; j++)
			{
				printf("%2%c", dir.ShortName[j]);
			}
			printf(".");
			for(; j < 11; j++)
			{
				printf("%2%c", dir.ShortName[j]);
			}
			printf("|%4Size%1:%2%ibytes", dir.FileLength);
			printf("\n");
		}
		pos += sizeof(Fat32Directory);
	}
	keys->Close();
}

bool Fat32::FindFileEntry(unsigned char* fileName, Fat32Directory* entry, unsigned long long startingSector)
{
	unsigned int currentSector = startingSector;
	unsigned int pos = this->Fat32Information.BytesPerSector;
	bool found = false;
	unsigned char buff[512];
	bool nextIsDir = false;
	int k = 0;
	while(fileName[k])
	{
		if(fileName[k] == '/' || fileName[k] == '\\')
		{
			nextIsDir = true;
			break;
		}
		k++;
	}
	while(true)
	{
		if(pos >= this->Fat32Information.BytesPerSector)
		{
			pos = 0;
			this->Disk->ReadDisk(buff, 1, currentSector);
			currentSector++;
		}
		Fat32Directory dir = *((Fat32Directory*)(buff + pos));
		if(dir.Attributes == Fat32Attributes::LONGNAME)
		{
			// Long name entry ( we don't deal with these
		}
		else if(dir.ShortName[0] == 0x00)
		{
			// No more entries
			break;
		}
		else if(dir.ShortName[0] != 0xE5)
		{
			int i = 0,j = 0;
			bool same = true;
			for(; fileName[j] && i < 11;i++)
			{
				// make sure to deal with the dots
				if(nextIsDir)
				{
					if(j >= 11)
					{
						same = false;
						break;
					}
					bool end = (fileName[j] == '/') | (fileName[j] == '\\');
					if(end)
					{
						if(IsNonWhitespace(dir.ShortName[j]))
						{
							same = false;
							break;
						}
						else
						{
							return this->FindFileEntry(fileName + j + 1, entry,
									this->ClusterToSector((unsigned long long)(dir.HighClusterClusterNumber << 16)
											| dir.LowClusterNumber));
						}
					}
					else
					{
						if(fileName[j] != dir.ShortName[j])
						{
							same = false;
							break;
						}
					}
					j++;
				}
				else
				{
					if(i == 8)
					{
						if(IsNonWhitespace(dir.ShortName[i]) && fileName[j++] != '.')
						{
							same = false;
							break;
						}
					}
					if(IsNonWhitespace(dir.ShortName[i]) == 0)
					{
						continue;
					}
					if(dir.ShortName[i] != fileName[j++])
					{
						same = false;
						break;
					}
				}
			}
			if(same)
			{
				*entry = dir;
				found = true;
				break;
			}
		}
		pos += sizeof(Fat32Directory);
	}
	return found;
}

File* Fat32::LoadFile(const char* fileName, unsigned int mode)
{
	Fat32File* file = NULL;
	Fat32Directory dir;
	if(this->FindFileEntry((unsigned char*)fileName,&dir, (unsigned long long)this->RootDirectorySector))
	{
		int i, pos = 0;
		// Allocate a new block of memory to represent the file
		file = new Fat32File;
		// copy the first part of the name
		for(i = 0; i < 8; i++)
		{
			if(Fat32Methods::IsNonWhitespace(dir.ShortName[i])) continue;
			file->FileInformation.Name[pos++] = dir.ShortName[i];
		}
		// Copy the extension for the file
		if(!Fat32Methods::IsNonWhitespace(dir.ShortName[i]))
		{
			file->FileInformation.Name[pos++] = '.';
			for(; i < 11; i++)
			{
				file->FileInformation.Name[pos++] = dir.ShortName[i];
			}
		}
		file->StartCluster = ((unsigned long long)dir.HighClusterClusterNumber << 16) + dir.LowClusterNumber;
		file->StartSector = this->ClusterToSector(file->StartCluster);
		file->SectorsLeftInCluster = this->Fat32Information.SectorsPerCluster;
		file->CurrentCluster = file->StartCluster;
		file->CurrentSector = file->StartSector;
		file->FileInformation.CurrentPosition = 0;
		file->FileInformation.Length = dir.FileLength;
	}
	else
	{
		//printf("%4FAT32%1:%3Unable to find File %2%s\n", fileName);
	}
	return (File*)file;
}

unsigned long long Fat32::GetNextCluster(unsigned long long currentCluster)
{
	unsigned char entrySector[512];
	unsigned int fatOffset = currentCluster * 4;
	unsigned int sector = this->Fat32Information.ReservedSectors + (fatOffset / 512);
	unsigned int entry = fatOffset % 512;
	// Load up the right table
	this->Disk->ReadDisk(entrySector,1,sector + this->BaseAddress);
	unsigned int* nextClustedPointer = (unsigned int*)(&entrySector[entry]);
	unsigned int nextCluster = (*nextClustedPointer) & 0x0FFFFFFF;
	// This covers both "BAD Cluster 0x0FFFFFF7
	if(nextCluster >= 0x0FFFFFF7) return 0;
	// The top 8 bits are not valid
	return nextCluster;
}

unsigned int Fat32::ReadFile(File* file, unsigned char* buffer, unsigned int length)
{
	unsigned char buff[512];
	Fat32File* f = (Fat32File*)file;
	unsigned int totalRead = 0;
	while(totalRead < length && f->FileInformation.CurrentPosition < f->FileInformation.Length && f->CurrentCluster)
	{
		unsigned int offset = f->FileInformation.CurrentPosition % 512;
		unsigned int delta = f->FileInformation.Length - f->FileInformation.CurrentPosition;
		unsigned int numberOfSectors = 1;
		if(delta > length - totalRead) delta = length - totalRead;
		if(offset != 0 && delta + offset > 512) delta = 512 - offset;

		if(delta > 512)
		{
			unsigned int bytesLeftInCluster = f->SectorsLeftInCluster * 512;
			if(delta > bytesLeftInCluster) delta = bytesLeftInCluster;
			numberOfSectors = delta / 512;
			delta = numberOfSectors * 512;
			this->Disk->ReadDisk(buffer + totalRead, numberOfSectors, f->CurrentSector);
		}
		else
		{
			this->Disk->ReadDisk(buff,1,f->CurrentSector);
			memcpy(buffer + totalRead, buff + offset, delta);
		}
		totalRead += delta;
		f->FileInformation.CurrentPosition += delta;
		// if what we read was over a sector boundary
		if(delta + offset >= 512)
		{
			f->CurrentSector += numberOfSectors;
			f->SectorsLeftInCluster -= numberOfSectors;
			if(f->SectorsLeftInCluster == 0)
			{
				int nextCluster = this->GetNextCluster(f->CurrentCluster);
				f->CurrentCluster = nextCluster;
				if(f->CurrentCluster == 0)
				{
					f->CurrentSector = f->StartSector;
					f->FileInformation.CurrentPosition = f->FileInformation.Length;
				}
				else
				{
					f->CurrentSector = this->ClusterToSector(f->CurrentCluster);
					f->SectorsLeftInCluster = this->Fat32Information.SectorsPerCluster;
				}
			}
		}
		else
		{
			break;
		}
	}
	return totalRead;
}

void Fat32::CloseFile(File* file)
{
	if(file)
	{
		delete ((Fat32File*)file);
	}
}

unsigned int Fat32::GetNextFileNameInDirectory(Directory* dir, unsigned char* buffer, unsigned int bufferSize)
{
	return 0;
}

Directory* Fat32::LoadDirectory(const char* fileName)
{
	return NULL;
}

void Fat32::CloseDirectory(Directory* dir)
{
	if(dir)
	{
		delete dir;
	}
}

