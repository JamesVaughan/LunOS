/*
 * sata.cpp
 *
 *  Created on: 2011-03-29
 *      Author: james
 */

#include <kern/io/Sata.h>
#include <kern/io/pci.h>
#include <kern/memory.h>
#include <kern/console.h>
#include <SystemCalls.h>
#include <kern/system.hpp>

using namespace LunOS;
using namespace LunOS::IO;
using namespace LunOS::IO::Kern;

unsigned int Sata::AHCIVersion = 0;
AhciDevice* Sata::DeviceList = NULL;
unsigned int Sata::NumberOfDevices = 0;
unsigned int Sata::NumberOfDrives = 0;
SataDiskInfo* Sata::DriveInformation = NULL;

bool Sata::ScanAndInitialise()
{
	/*
	 * Ok, the plan here is to scan the PCI bus, only looking at active busses in order to look for an ahci device.
	 * That is Subcode 6 of Device Type 1 of the PCI spec.
	 *
	 * Once we find one we add it to the list of devices we have found.
	 */
	LinkedList* deviceList = NULL;

	for(int bus = 0; bus < 256; bus++)
	{
		// no point scanning an inactive bus
		if(!PCI::ActivePCIBus[bus]) continue;
		for(int slot = 0; slot < 32; slot++)
		{
			for(int function = 0; function < 4; function++)
			{
				PCIDevice current = PCI::pciData[bus][slot][function];
				if(current.ClassCode == 0x1 && current.Subclass == 0x6)
				{
					// success we have found an ahci controller
					if(deviceList == NULL)
					{
						deviceList = new LinkedList();
					}
					AhciDevice* device = new AhciDevice;
					device->Registers = (AhciRegisters*)PCI::GetMemoryAddress(current.BAR[5]);
					device->InterruptVector = current.InterruptLine;
					deviceList->AddLast(device);
					irq_install_handler(device->InterruptVector, IRQHandler);
				}
			}
		}
	}
	if(!deviceList)
	{
		return false;
	}
	LinkedList* driveList = new LinkedList();
	Sata::DeviceList = new AhciDevice[deviceList->Length];
	int i = 0;
	Node* current = deviceList->Root;
	while(current)
	{
		Sata::DeviceList[i] = *((AhciDevice*)current->data);
		Sata::InitialiseDevice(driveList, i);
		delete ((AhciDevice*)current->data);
		current = current->next;
		i++;
	}
	Sata::NumberOfDrives = driveList->Length;
	Sata::DriveInformation = new SataDiskInfo[Sata::NumberOfDrives];
	i = 0;
	current = driveList->Root;
	while(current)
	{
		Sata::DriveInformation[i] = *((SataDiskInfo*)current->data);
		delete (SataDiskInfo*)current->data; // release the temp version
		current = current->next;
		i++;
	}
	delete driveList;
	delete deviceList;
	// ok we are now ready to be loaded and accessed by userspace
	Device us;
	us.AWrite = Sata::Write;
	us.Write = Sata::Write;
	us.Read = Sata::Read;
	us.ARead = Sata::Read;
	us.Shutdown = NULL;
	us.DeviceThread = NULL;
	us.Close = Sata::Close;
	us.Open = Sata::Open;
	us.type = LunOS::IO::DeviceTypes::DEVICE_HDD;
::System::InstallDevice(us);
	return true;
}

void Sata::IRQHandler(struct regs *r)
{
	printf("In the IRQ handler!\n");
}

void Sata::StartPort(void* port)
{
	// Wait until CR (bit15) is cleared
	while (((AhciPort*)port)->CommandAndStatus & 0x8000);

	// Set FRE (bit4) and ST (bit0)
	((AhciPort*)port)->CommandAndStatus |= 0x10;
	((AhciPort*)port)->CommandAndStatus |= 0x01;

}

void Sata::StopPort(void* port)
{
	// Clear ST (bit0)
	((AhciPort*)port)->CommandAndStatus &= ~0x1;
	// Wait until ST (bit0) and CR (bit15) are cleared
	while(true)
	{
		if (((AhciPort*)port)->CommandAndStatus & 0x8001)
		{
			continue;
		}
		break;
	}
	// Clear FRE (bit4)
	((AhciPort*)port)->CommandAndStatus &= ~0x10;
}

#define	SATA_SIG_ATA	0x00000101	// SATA drive
#define	SATA_SIG_ATAPI	0xEB140101	// SATAPI drive
#define	SATA_SIG_SEMB	0xC33C0101	// Enclosure management bridge
#define	SATA_SIG_PM	0x96690101	// Port multiplier

void Sata::InitialiseDevice(LinkedList* driveList, unsigned int deviceNumber)
{
	unsigned int* memorymap = (unsigned int*)Sata::DeviceList[deviceNumber].Registers;
	Memory::RemoveFromFreeStack(memorymap);
	Memory::linkPage(memorymap, memorymap);
	Memory::MarkUncacheable(memorymap);
	// now we can directly write to their addresses
	AhciRegisters* registers = Sata::DeviceList[deviceNumber].Registers;
	unsigned int portMask = 0x1;
	for(int i = 0; i < 32; i++)
	{
		if(!(registers->PortImplemented & portMask))
		{
			portMask = portMask << 1;
			continue;
		}
		switch(registers->ports[i].Signature)
		{
			case SATA_SIG_ATA:
			{
				printf("%2Found a BASIC SATA device\n");
				Sata::StopPort((void*)&registers->ports[i]);
				SataDiskInfo* drive = new SataDiskInfo;
				drive->Controller = (Sata::DeviceList + deviceNumber);
				drive->DeviceNumber = i;
				// load in some pages for it's buffer
				for(int j = 0; j < 32; j++)
				{
					drive->BufferMemoryVirtual[j] = Memory::requestPages(1);
					if(!drive->BufferMemoryVirtual[j])
					{
						printf("%3SATA: UNABLE TO GET STRUCTURE MEMORY!\n");
						return;
					}
					drive->BufferMemoryPhysical[j] = (unsigned int*)Memory::GetLocalAddress(drive->BufferMemoryVirtual[j]);
					//printf("Buffer V@0x%x | P@0x%x\n",drive->BufferMemoryVirtual[j],
					//		drive->BufferMemoryPhysical[j]);
					Memory::MarkUncacheable(drive->BufferMemoryVirtual[j]);
				}
				drive->BasicInformation.Exists = true;
				drive->CommandHeader = (HBA_CMD_HEADER*)Memory::requestPages(1);
				if(!drive->CommandHeader)
				{
					printf("%3SATA: UNABLE TO GET COMMAND HEADER MEMORY!\n");
					return;
				}
				Memory::MarkUncacheable((unsigned int*)drive->CommandHeader);
				registers->ports[i].CommandListLow = (unsigned int)Memory::GetLocalAddress(drive->CommandHeader);
				// just put the FIS right below the CommandList since we know that the header is 0x1000 aligned and only goes for
				// 1kb (0x400 bytes)
				registers->ports[i].FrameLow = (unsigned int)registers->ports[i].CommandListLow + 0x400;
				registers->ports[i].FrameHigh = 0;
				drive->CommandTable = (HBA_CMD_TBL*)Memory::requestPages(1);
				drive->CommandHeader->ctba = (unsigned int)Memory::GetLocalAddress(drive->CommandTable);
				drive->CommandHeader->ctbau = 0;
				Memory::MarkUncacheable((unsigned int*)drive->CommandTable);
				drive->CommandHeader->prdtl = 32;
				registers->ports[i].InterruptEnable = 0; // for now we don't want any interrupts
				Sata::StartPort((void*)&registers->ports[i]);
				driveList->AddLast(drive);
				break;
			}
			case SATA_SIG_ATAPI:
			{
				printf("Found a SATA ATAPI Device!\n");
				break;
			}
			case SATA_SIG_SEMB:
			{
				printf("Found an external sata encloser\n");
				break;
			}
			case SATA_SIG_PM:
			{
				printf("Found a Port Multiplier device\n");
				break;
			}
		}
		portMask = portMask << 1;
	}
}


 bool Sata::Open(FDTEntry* fdt, unsigned char* buffer, size_t length)
 {
	 return true;
 }

 bool Sata::Read(FDTEntry* fdt, unsigned char* buffer, size_t length)
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
			//bool failedToRead = false;
			if(drive >= Sata::NumberOfDrives)
			{
				return false;
			}
			AhciDevice* device = Sata::DriveInformation[drive].Controller;
			AhciPort* port = &device->Registers->ports[Sata::DriveInformation[drive].DeviceNumber];
			//port->CommandAndStatus = port->CommandAndStatus | 0x13000001;
			while(sectorCount > 0)
			{
				unsigned char burstSize = sectorCount > 32 ? 32 : sectorCount;
				unsigned int freeSlot = 0;
				// right now I don't want any interrupts, and clear out all previous error status
				port->InterruptStatus = 0;
				port->InterruptEnable = ~0x0;
				port->SataError = 0;
				// for now we are only going to be using the first command header
				HBA_CMD_HEADER* cmdHeader = (HBA_CMD_HEADER*)(&Sata::DriveInformation[drive].CommandHeader[freeSlot]);
				cmdHeader->cfl = sizeof(AhciHostToDevice);
				cmdHeader->a = 0;
				cmdHeader->w = 0; // read
				cmdHeader->p = 0;
				cmdHeader->r = 0;
				cmdHeader->b = 0;
				cmdHeader->prdtl = (burstSize / 8) + (burstSize % 8 > 0? 1 : 0);
				HBA_CMD_TBL* cmdTable = &Sata::DriveInformation[drive].CommandTable[freeSlot];
				memset(cmdTable, 0, sizeof(HBA_CMD_TBL));
				int i;
				// 4K bytes (8 sectors) per PRDT
				for (i = 0; i<cmdHeader->prdtl - 1; i++)
				{
					cmdTable->prdt_entry[i].dba = (unsigned int)(Sata::DriveInformation[drive].BufferMemoryPhysical[i]);
					cmdTable->prdt_entry[i].dbau = 0;
					cmdTable->prdt_entry[i].dbc = 4*1024;	// 4K bytes (1 page)
					cmdTable->prdt_entry[i].i = 1;
					cmdTable->prdt_entry[i].rsv0 = 0;
					cmdTable->prdt_entry[i].rsv1 = 0;
					sectorCount -= 8;
				}
				cmdTable->prdt_entry[i].dba = (unsigned int)(Sata::DriveInformation[drive].BufferMemoryPhysical[i]);
				cmdTable->prdt_entry[i].dbau = 0;
				cmdTable->prdt_entry[i].dbc = (sectorCount < 8 ? sectorCount * 512 : 4*1024); //load up to 1 more page
				cmdTable->prdt_entry[i].i = 1; // interrupt when it is done
				cmdTable->prdt_entry[i].rsv0 = 0;
				cmdTable->prdt_entry[i].rsv1 = 0;
				sectorCount -= (sectorCount < 8? sectorCount : 8);

				AhciHostToDevice* cmdfis = (AhciHostToDevice*)(cmdTable->cfis);
				cmdfis->Type = (unsigned char)LunOS::IO::Kern::AchiFrame::REG_H2D;
				cmdfis->CommandOrControl = 1;	// Command
				//cmdfis->Command = 0xC8; // Read LBA24 DMA
				cmdfis->Command = 0x25; // Read LBA48 DMA

				cmdfis->LBA0 = (unsigned char)lbaValue;
				cmdfis->LBA1 = (unsigned char)(lbaValue>>8);
				cmdfis->LBA2 = (unsigned char)(lbaValue>>16);
				cmdfis->Device = 1<<6;	// LBA mode


				cmdfis->LBA3 = (unsigned char)(lbaValue>>24);
				cmdfis->LBA4 = 0;
				cmdfis->LBA5 = 0;

				cmdfis->CountLow = (unsigned char)burstSize;
				cmdfis->CountHigh = (unsigned char)(burstSize >> 8);

				port->CommandIssue = 1<<freeSlot;	// Issue command

				// Wait for completion
				while (true)
				{
					if ((port->CommandIssue & (1<<freeSlot)) == 0)
					{
						break;
					}
					if (port->InterruptStatus & 0x40000000)	// Task file error
					{
						printf("%3Read disk error 0x%x\n", port->TaskFileData);
						return false;
					}
					else if(port->InterruptStatus)
					{
						port->InterruptStatus = 0;
						//printf("IS:%20x%x\n", port->InterruptStatus);
						//printf("DIS:%20x%x\n", device->Registers->InterruptStatus);
						// we finished servicing the device
						device->Registers->InterruptStatus =
								device->Registers->InterruptStatus &
								~(1<<Sata::DriveInformation[drive].DeviceNumber);
					}
					// printf("CI:%20x%x\n", port->CommandIssue);
					// printf("CS:%20x%x\n", port->CommandAndStatus);
					// printf("SE:%20x%x\n", port->SataError);
					// printf("TF:%20x%x\n", port->TaskFileData);
					// LunOS::System::Sleep(1000);
				}

				// Check again
				if (port->InterruptStatus & 0x40000000)
				{
					printf("%4Read disk error 0x%x\n", port->TaskFileData);
					return false;
				}
				// copy the data to the user's buffer
				for(int k = 0; k < cmdHeader->prdtl; k++)
				{
					unsigned int sizeOfData = cmdTable->prdt_entry[i].dbc;
					memcpy(buffer, Sata::DriveInformation[drive].BufferMemoryVirtual[k], sizeOfData);
					buffer += sizeOfData;
				}
			}
			break;
		}
			// Read data from registers
		case 1:
		{
			return false;
		}
		case 2:
		{
			unsigned short driveNumber = index[1];
			if(driveNumber >= Sata::NumberOfDrives) return false;
			// give them a copy of the disk information in the buffer
			*((DiskInfo*)buffer) = *((DiskInfo*)(&Sata::DriveInformation[driveNumber]));
			return true;
		}
		default:
			return false;
		}
		return true;
 }

 bool Sata::Write(FDTEntry* fdt, unsigned char* buffer, size_t length)
 {
	 return false;
 }

 bool Sata::Close(FDTEntry* entry)
 {
	 return true;
 }
