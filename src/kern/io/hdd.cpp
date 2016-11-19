#include <kern/io/HardDrive.h>
#include <kern/io/io.h>
#include <kern/system.hpp>
#include <kern/console.h>
#include <kern/io/Sata.h>
#include <SysCall.h>

#define    ATA_IDENT_DEVICETYPE   0
#define    ATA_IDENT_CYLINDERS   2/2
#define    ATA_IDENT_HEADS      6/2
#define    ATA_IDENT_SECTORS      12/2
#define    ATA_IDENT_SERIAL   20/2
#define    ATA_IDENT_MODEL      54/2
#define    ATA_IDENT_CAPABILITIES   98/2
#define    ATA_IDENT_FIELDVALID   106/2
#define    ATA_IDENT_MAX_LBA   120/2
#define   ATA_IDENT_COMMANDSETS   164/2
#define    ATA_IDENT_MAX_LBA_EXT   200/2

using namespace LunOS;
using namespace LunOS::IO;
using namespace LunOS::IO::DeviceTypes;

// Declare all of our static information
unsigned short BasicHDD::IdePIOLocations[4] = {0x1f0, 0x170, 0x1e8,0x168};
unsigned short BasicHDD::IdePIOControlRegisters[4] = {0x3f6, 0x376, 0x3E6, 0x366};
DiskInfo BasicHDD::DiskInformation[8];

void BasicHDD::Init()
{
	// If we can install the SATA driver first, we are the backup
	if(!LunOS::IO::Kern::Sata::ScanAndInitialise())
	{
		BasicHDD::ScanBuses();
		// Now install a VGA driver
		Device us;
		us.AWrite = BasicHDD::Write;
		us.Write = BasicHDD::Write;
		us.Read = BasicHDD::Read;
		us.ARead = BasicHDD::Read;
		us.Shutdown = NULL;
		us.DeviceThread = NULL;
		us.Close = BasicHDD::Close;
		us.Open = BasicHDD::Open;
		us.type = LunOS::IO::DeviceTypes::DEVICE_HDD;
	::System::InstallDevice(us);
	}
}

void BasicHDD::ScanDrives(int bus)
{
	// When we get here we know there is at least 1 drive on that bus
	int loops;
	// Now it is time to go and run the IDENTIFY command on the bus and grab our information
	for(int i = 0; i < 2; i++)
	{
		int drive = (i == 0 ? 0xa0 : 0xb0);
		unsigned char IDENTIFY = 0xec;
		outportb(BasicHDD::IdePIOLocations[bus] + 6, drive);
		outportb(BasicHDD::IdePIOLocations[bus] + 2, 0);
		outportb(BasicHDD::IdePIOLocations[bus] + 3, 0);
		outportb(BasicHDD::IdePIOLocations[bus] + 4, 0);
		outportb(BasicHDD::IdePIOLocations[bus] + 5, 0);
		outportb(BasicHDD::IdePIOLocations[bus] + 7, IDENTIFY);
		unsigned char status = inportb(BasicHDD::IdePIOLocations[bus] + 7);
		if(status == 0)
		{
			printf("%3Drive[%i:%i] does not exist.\n",bus, i);
			BasicHDD::DiskInformation[bus * 2 + i].Exists = false;
			continue;
		}
		// If we get here then we just need to wait for our information to be ready
		bool error = false;
		for(loops = 0; loops < 200; loops++)
		{
			unsigned char lbaMid = inportb(BasicHDD::IdePIOLocations[bus] + 4);
			unsigned char lbaHigh = inportb(BasicHDD::IdePIOLocations[bus] + 5);
			error = true;
			if(lbaMid != 0 || lbaHigh != 0)
			{
				printf("%3Drive[%i:%i] is not an ATA device.\n",bus, i);
				error = true;
				break;
			}
			status = inportb(BasicHDD::IdePIOLocations[bus] + 7);
			if(!(status & 0x80))
			{
				error = false;
				break;
			}
		}
		if(error)
		{
			BasicHDD::DiskInformation[bus * 2 + i].Exists = false;
			continue;
		}
		for(loops = 0; loops < 200; loops++ )
		{
			error = true;
			status = inportb(BasicHDD::IdePIOLocations[bus] + 7);
			if(((status & 8) || (status & 1)))
			{
				error = false;
				break;
			}
		}
		if(error)
		{
			BasicHDD::DiskInformation[bus * 2 + i].Exists = false;
			continue;
		}
		// Some devices don't set the error
		if((status & 1) || !(status & 8))
		{
			printf("%3Drive[%i:%i] had an error while getting the identity.\n",bus, i);
			BasicHDD::DiskInformation[bus * 2 + i].Exists = false;
			continue;
		}
		// If we get here we can start reading the information!
		for(int j = 0; j < 256; j++)
		{
			unsigned short data = inportw(BasicHDD::IdePIOLocations[bus]);
			switch(j)
			{
				case ATA_IDENT_DEVICETYPE:
				{
					//printf("Device found on bus[%i]:[%i] with Identity[0] == %i\n",bus,i,data);
					BasicHDD::DiskInformation[bus * 2 + i].Exists = true;
					break;
				}
				case ATA_IDENT_MAX_LBA:
				{
					unsigned int sectors = data + (inportw(BasicHDD::IdePIOLocations[bus]) << 16);
					j++;
					BasicHDD::DiskInformation[bus * 2 + i].Sectors = sectors;
					BasicHDD::DiskInformation[bus * 2 + i].LBA28 = sectors != 0;
					break;
				}
				case ATA_IDENT_MAX_LBA_EXT:
				{
					unsigned long long sectors = data
							+ ((unsigned long long)inportw(BasicHDD::IdePIOLocations[bus]) << 16)
							+ ((unsigned long long)inportw(BasicHDD::IdePIOLocations[bus]) << 32)
							+ ((unsigned long long)inportw(BasicHDD::IdePIOLocations[bus]) << 48);
					j += 4;
					if(sectors > BasicHDD::DiskInformation[bus * 2 + i].Sectors)
					{
						BasicHDD::DiskInformation[bus * 2 + i].Sectors = sectors;
						BasicHDD::DiskInformation[bus * 2 + i].LBA48 = true;
					}
					else
					{
						BasicHDD::DiskInformation[bus * 2 + i].LBA48 = false;
					}
					break;
				}
			}
		}
	}
}

void BasicHDD::ScanAndLoadFromPCI()
{
	int drivesFound = 0;
	unsigned int bus, slot, function;
	for(bus = 0; bus < 256; bus++)
	{
		// Don't bother scanning busses with no information
		for(slot = 0; slot < 32; slot++)
		{
			for(function = 0; function < 4; function++)
			{
				if(PCI::pciData[bus][slot][function].ClassCode == PCIClassCodes::MassStorageController)
				{
					bool load = false;
					switch(PCI::pciData[bus][slot][function].Subclass)
					{
						case PCIClassCodes::IDE:
						{
							load = true;
							break;
						}
						case PCIClassCodes::SATA:
						{
							load = true;
							break;
						}
						case PCIClassCodes::ATA:
						{
							load = true;
							break;
						}
					}
					if(load)
					{
						int j, k ;
						for(j = 0; j < 2; j++)
						{
							bool skip = false;
							for(k = 0; k < drivesFound; k++)
							{
								if(BasicHDD::IdePIOLocations[k] == PCI::pciData[bus][slot][function].BAR[0 + j])
								{
									skip = true;
									break;
								}
							}
							if(skip) continue;
							if(PCI::pciData[bus][slot][function].BAR[0 + j] != 0)
							{
								BasicHDD::IdePIOLocations[drivesFound] = PCI::pciData[bus][slot][function].BAR[0 + j];
								BasicHDD::IdePIOControlRegisters[drivesFound++] = PCI::pciData[bus][slot][function].BAR[2 + j];
								switch(PCI::pciData[bus][slot][function].Subclass)
								{
									case PCIClassCodes::IDE:
									{
										printf("Found %2IDE %1device at [%i, %i, %i]\n", bus, slot, function);
										break;
									}
									case PCIClassCodes::SATA:
									{
										printf("Found %2SATA %1device at [%i, %i, %i]\n", bus, slot, function);
										break;
									}
									case PCIClassCodes::ATA:
									{
										printf("Found %2ATA %1device at [%i, %i, %i]\n", bus, slot, function);
										break;
									}
								}
							}
						}
					}
				}
			}
		}
	}
}

void BasicHDD::ScanBuses()
{
	int i;
	ScanAndLoadFromPCI();
	// Do a flash reset on all of the hard drives to make sure they are in a normal state
	outportb(IdePIOControlRegisters[0], 4);
	outportb(IdePIOControlRegisters[1], 4);
	outportb(IdePIOControlRegisters[2], 4);
	outportb(IdePIOControlRegisters[3], 4);

	outportb(IdePIOControlRegisters[0], 0);
	outportb(IdePIOControlRegisters[1], 0);
	outportb(IdePIOControlRegisters[2], 0);
	outportb(IdePIOControlRegisters[3], 0);
	// You can wait by reading 5 times from the status register
	for(i = 0; i < 5; i++)
	{
		inportb(IdePIOControlRegisters[0] + 0x07);
	}
	// Now that we have waited for voltages to get back in order lets go look at what we have
	for(i = 0; i < 4; i++)
	{
		unsigned char status = inportb(IdePIOControlRegisters[i] + 0x07);
		// if we have a floating bus then there are no devices on it
		if(status == 255)
		{
			DiskInformation[i * 2].Exists = false;
			DiskInformation[i * 2 + 1].Exists = false;
		}
		else
		{
			ScanDrives(i);
		}
	}
}

bool BasicHDD::Open(FDTEntry* entry, unsigned char* buffer, size_t length)
{;
	return true;
}

bool BasicHDD::WaitForDrive()
{
	unsigned int timesWaiting;
	unsigned char value;
	bool didReset = false;
	do
	{
		for(timesWaiting = 0; timesWaiting < 50; timesWaiting++)
		{
			if(!((value = inportb(0x1F7)) & 0x80))
			{
				if(didReset)
				{
					printf("%2Successfully reset\n");
					// reset ftw
					return false;
				}
				else
				{
					if(value & 8)
					{
						return true;
					}
				}
			}
			LunOS::System::Sleep(32);
		}
		printf("%3Reseting the drives\n");
		// Reset the drive here
		outportb(0x3F6, 0x04);
		// wait over 400 ms to make sure it worked
		LunOS::System::Sleep(500);
		printf("%2Setting it back for working order\n");
		outportb(0x3F6, 0x00);
		didReset = true;
		LunOS::System::Sleep(10000);
	}while(true);
	return false;
}

bool BasicHDD::Read(FDTEntry* entry, unsigned char* buffer, size_t length)
{
	// Read Format all in Words
	// [0]: Read Data from Disk == [0] | Read Data from Registers  == [1]
	//      Read Disk Information [2]  |
	// If From Disk
	// [1]: Amount of sectors to read
	// [2]: Drive {0 or 1}
	// [3]: LBA
	// [4]: LBA Upper [32-bit]
	// The data is returned back in the buffer
	// If From Registers
	// [1]: Register offset
	if(length <= 0) return false;
	unsigned short* index = (unsigned short*)buffer;
	//printf("Selection %i:%i:%i:%i:%i\n", index[0], index[1], index[2], index[3], index[4]);
	// read data from disk
	switch(*index)
	{
	// read data from disk
	case 0:
	{
		// we need to make copies of this since we are going to be overwriting this
		unsigned short sectorCount = index[1];
		unsigned short drive = index[2];
		unsigned int lbaValue = *(unsigned int*)(index + 3);
		bool failedToRead = false;
		unsigned char burstSize = sectorCount > 32? 32 : sectorCount;
		// Use LBA mode
		outportb(0x1F6, (drive ? 0xF0 : 0xE0) | (((unsigned char)(lbaValue >> 24)) & 0x0F));
		outportb(0x1f2, burstSize); // Read 1 sector at a time
		while(sectorCount > 0)
		{
			burstSize = sectorCount > 32? 32 : sectorCount;
			if(failedToRead)
			{
				outportb(0x1F6, (drive ? 0xF0 : 0xE0) | (((unsigned char)(lbaValue >> 24)) & 0x0F));
				outportb(0x1f2, burstSize); // Read 1 sector at a time
				failedToRead = false;
			}
			outportb(0x1F3, (unsigned char) (lbaValue));
			outportb(0x1F4, (unsigned char) (lbaValue >> 8));
			outportb(0x1F5, (unsigned char) (lbaValue >> 16));
			outportb(0x1F7, 0x20);
			// Now do the bad stuff and wait here
			for(;burstSize > 0; burstSize--)
			{
				if(!BasicHDD::WaitForDrive())
				{
					failedToRead = true;
					continue;
				}
				// Now that the drives are ready read in the data
				for(unsigned int wordsRead = 0; wordsRead < 256; wordsRead++)
				{
					*index = inportw(0x1F0);
					index++;
				}
				lbaValue++;
				sectorCount--;
			}
		}
		break;
	}
		// Read data from registers
	case 1:
	{
		unsigned short offset = index[1];
		// Make sure it is a valid offset
		if(offset > 7)
		{
			return false;
		}
		index[0] = inportb(0x1f0 + offset);
		return true;
	}
	case 2:
	{
		int driveNumber = index[1];
		if(driveNumber > 8) return false;
		// give them a copy of the disk information in the buffer
		*(DiskInfo*)index = BasicHDD::DiskInformation[driveNumber];
		return true;
	}
	default:
		return false;
	}
	return true;
}

bool BasicHDD::Write(FDTEntry* entry, unsigned char* buffer, size_t length)
{
	// We do not support writing at this time
	return false;
}

bool BasicHDD::Close(FDTEntry* entry)
{
	return true;
}
