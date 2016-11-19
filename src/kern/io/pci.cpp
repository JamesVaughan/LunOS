#include <kern/io/pci.h>
#include <kern/system.hpp>
#include <SysCall.h>
using namespace LunOS::IO;

#define PCIWRITEPORT 0xCF8
#define PCIREADPORT 0xCFC


namespace LunOS
{
	namespace IO
	{
		PCIDevice PCI::pciData[256][32][4];
		bool PCI::ActivePCIBus[256];

		unsigned short PCI::ReadWord(unsigned char bus, unsigned char device, unsigned char function, unsigned char offset)
		{
			unsigned long address;
			    unsigned long lbus = (unsigned long)bus;
			    unsigned long lslot = (unsigned long)device;
			    unsigned long lfunc = (unsigned long)function;
			    unsigned short tmp = 0;

			    /* create configuration address as per Figure 1 */
			    address = (unsigned long)((lbus << 16) | (lslot << 11) |
			              (lfunc << 8) | (offset & 0xfc) | ((unsigned int)0x80000000));

			    /* write out the address */
			    outportd(PCIWRITEPORT, address);
			    /* read in the data */
			    tmp = (unsigned short)((inportd(PCIREADPORT) >> ((offset & 2) * 8)) & 0xffff);
			    return (tmp);
		}


		void PCI::WriteWord(unsigned char bus, unsigned char device, unsigned char function, unsigned char offset, unsigned short value)
		{
			unsigned long address;
				unsigned long lbus = (unsigned long)bus;
				unsigned long lslot = (unsigned long)device;
				unsigned long lfunc = (unsigned long)function;

				/* create configuration address as per Figure 1 */
				address = (unsigned long)((lbus << 16) | (lslot << 11) |
						  (lfunc << 8) | (offset & 0xfc) | ((unsigned int)0x80000000));

				/* write out the address */
				outportd(PCIWRITEPORT, address);
				/* read in the data */
				outportw(PCIWRITEPORT, value);
		}

		void PCI::WriteDWord(unsigned char bus, unsigned char device, unsigned char function, unsigned char offset, unsigned int value)
		{
			unsigned long address;
				unsigned long lbus = (unsigned long)bus;
				unsigned long lslot = (unsigned long)device;
				unsigned long lfunc = (unsigned long)function;
				/* create configuration address as per Figure 1 */
				address = (unsigned long)((lbus << 16) | (lslot << 11) |
						  (lfunc << 8) | (offset & 0xfc) | ((unsigned int)0x80000000));

				/* write out the address */
				outportd(PCIWRITEPORT, address);
				/* write the data */
				outportd(PCIWRITEPORT, value);
		}


		unsigned short PCI::GetVendor(unsigned char bus, unsigned char device, unsigned char function)
		{
		    return ReadWord(bus,device,function,0);
		}

		void PCI::StorePCIDeviceData(unsigned char bus, unsigned char device, unsigned char function, unsigned short vendorID)
		{
			unsigned short temp,i;
			pciData[bus][device][function].VendorNumber = vendorID;
			pciData[bus][device][function].DeviceNumber = ReadWord(bus,device,function,0x2);
			temp = ReadWord(bus,device,function,0x8);
			pciData[bus][device][function].RevisionID = (unsigned char)temp;
			pciData[bus][device][function].ProgramIF = (unsigned char)(temp >> 8);
			temp = ReadWord(bus,device,function,0xa);
			pciData[bus][device][function].Subclass = (unsigned char)temp;
			pciData[bus][device][function].ClassCode = (unsigned char)(temp >> 8);
			temp = ReadWord(bus,device,function,0xc);
			pciData[bus][device][function].CacheLineSize = (unsigned char)temp;
			pciData[bus][device][function].LatencyTimer = (unsigned char)(temp >> 8);
			temp = ReadWord(bus,device,function,0xe);
			pciData[bus][device][function].HeaderType = (unsigned char)temp;
			pciData[bus][device][function].BIST = (unsigned char)(temp >> 8);
			for(i = 0; i < 6; i++)
			{
				pciData[bus][device][function].BAR[i] = ReadWord(bus,device,function,0x10+(i<<2))
						+ (((unsigned int)ReadWord(bus,device,function,0x12+(i<<2)))<<16);
			}
			pciData[bus][device][function].CardbusCSIPointer = (ReadWord(bus,device,function,0x28))+
					((ReadWord(bus,device,function,0x30))<<16);
			pciData[bus][device][function].ExpansionROMBaseAddress = (ReadWord(bus,device,function,0x30))+
					((ReadWord(bus,device,function,0x32))<<16);
			pciData[bus][device][function].CapabilityPointer = (unsigned char)ReadWord(bus,device,function,0x34);
			temp = ReadWord(bus,device,function,0x3c);
			pciData[bus][device][function].InterruptLine = (unsigned char)temp;
			pciData[bus][device][function].InterruptPin = (unsigned char)(temp >> 8);
			pciData[bus][device][function].RESERVED = 0;
		}

		void PCI::Update(unsigned char bus, unsigned char slot, unsigned char function)
		{
			unsigned int vendorID;
			if((vendorID = GetVendor(bus,slot, function)) != 0xFFFF)
			{
				StorePCIDeviceData(bus,slot, function, vendorID);
				PCI::ActivePCIBus[bus] = true;
			}
			else
			{
				memset((unsigned short*)&pciData[bus][slot][function],0,sizeof(PCIDevice));
			}
		}

		void PCI::Probe()
		{
			//
			unsigned int numberOfBusses = 256;
			unsigned int numberOfDevices = 32;
			unsigned int numberOfFunctions = 4;
			unsigned int bus,dev, func;
			for(bus = 0; bus < numberOfBusses; bus++)
			{
				PCI::ActivePCIBus[bus] = false;
				for(dev = 0; dev < numberOfDevices; dev++)
				{
					for(func = 0; func < numberOfFunctions; func++)
					{
						PCI::Update(bus,dev,func);
					}
				}
			}
		}

		bool PCI::Write(FDTEntry* entry, unsigned char* buffer, size_t length)
		{
			return true;
		}

		bool PCI::Read(FDTEntry* entry, unsigned char* buffer, size_t length)
		{
			// First we need to figure out what we want to do
			// in regards to what we are trying to read from the PCI
			unsigned int bus, slot, function;
			slot = buffer[1];
			bus = buffer[2];
			//printf("%4%i\n", (unsigned int)buffer[3]);
			function = buffer[3];
			// if the 0th bit is 1 then we want to read the device information
			if((buffer[0] & 0x01) == 1)
			{
				// ok lets store some device information
				*((PCIDevice*)buffer) = pciData[bus][slot][function];
			}
			else if((buffer[0] & 0x02) == 1)
			{
				*(unsigned int*)buffer = ReadWord(bus,slot,function,0x06);
			}
			else
			{
				// in case we define some other options that buffer[0] could allow for
			}
			return true;
		}

		bool PCI::Open(FDTEntry* entry, unsigned char* buffer, size_t length)
		{
			return true;
		}

		bool PCI::Close(FDTEntry* entry)
		{
			return true;
		}

		unsigned int PCI::GetMemoryAddress(unsigned int BAR)
		{
			if(BAR & 0x1)
			{
				return 0;
			}
			return (BAR & ~0xF);
		}

		unsigned int PCI::GetIOAddress(unsigned int BAR)
		{
			if(!(BAR & 0x1))
			{
				return 0;
			}
			return BAR & ~0x3;
		}

		void PCI::WriteBAR(unsigned char bus, unsigned char slot, unsigned char function, unsigned char bar, unsigned int value)
		{

		}

		void PCI::Init()
		{
			// We are going to use an older version of the code since PCIBIOS-32bit doesn't
			// seem to be pervasive yet.
			Probe();
			// Now install the PCI Driver
			Device us;
			us.AWrite = Write;
			us.Write = Write;
			us.Read = Read;
			us.ARead = Read;
			us.Shutdown = NULL;
			us.DeviceThread = NULL;
			us.Close = Close;
			us.Open = Open;
			us.type = LunOS::IO::DeviceTypes::PCI_BUS;
			::System::InstallDevice(us);
		}
	}
}
