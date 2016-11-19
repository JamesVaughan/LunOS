#include <kern/system.hpp>
#include <io/Pci.h>
#include <io/Io.h>
#include <SysCall.h>
#include <kern/console.h>

using namespace LunOS::IO;

namespace LunOS
{
	namespace IO
	{
		unsigned char* GetClassName(PCIDevice* dev)
		{
			switch(dev->ClassCode)
			{
			case 0:
				return (unsigned char*)"OldDevices";
			case 1:
				return (unsigned char*)"MassStorageController";
			case 2:
				return (unsigned char*)"NetworkController";
			case 3:
				return (unsigned char*)"DisplayController";
			case 4:
				return (unsigned char*)"MultimediaDevice";
			case 5:
				return (unsigned char*)"MemoryController";
			case 6:
				return (unsigned char*)"BrideDevice";
			case 7:
				return (unsigned char*)"SimpleCommunicationsController";
			case 8:
				return (unsigned char*)"BaseSystemPeripherals";
			case 9:
				return (unsigned char*)"InputDevices";
			case 10:
				return (unsigned char*)"DockingStations";
			case 11:
				return (unsigned char*)"Processor";
			case 12:
				return (unsigned char*)"SerialBusController";
			case 13:
				return (unsigned char*)"WirelessController";
			case 14:
				return (unsigned char*)"IntelegentIOController";
			case 15:
				return (unsigned char*)"SatelliteCommunicationController";
			case 16:
				return (unsigned char*)"EncryptionDecryptionController";
			case 17:
				return (unsigned char*)"SignalProcessingController";
			default:
				return (unsigned char*)"Unknown";
			}
		}

		unsigned char* GetSubclassName(PCIDevice* dev)
		{
			switch(dev->ClassCode)
			{
			case 0:
				return (unsigned char*)"Unknown";
			case 1:
				//{SCSI = 0, IDE = 1, FloppyDisk = 2, IPI = 3, RAID = 4, ATA = 5, SATA = 6, SAS = 7, OtherMassStorage = 0x80 };
				switch(dev->Subclass)
				{
				case PCIClassCodes::SCSI:
					return (unsigned char*)"SCSI";
				case PCIClassCodes::IDE:
					return (unsigned char*)"IDE";
				case PCIClassCodes::FloppyDisk:
					return (unsigned char*)"Floppy Disk";
				case PCIClassCodes::IPI:
					return (unsigned char*)"IPI";
				case PCIClassCodes::RAID:
					return (unsigned char*)"RAID";
				case PCIClassCodes::ATA:
					return (unsigned char*)"ATA";
				case PCIClassCodes::SATA:
					return (unsigned char*)"SATA";
				case PCIClassCodes::SAS:
					return (unsigned char*)"SAS";
				default:
					return (unsigned char*)"Unknown";
				}
				break;
			case 2:
				//{Ethernet = 0, TokenRing = 1, FDDI = 2, ATM = 3, ISDN = 4, WorldFip = 5, PICGM = 6, OtherNetworkController = 0x80 };
				switch(dev->Subclass)
				{
				case 0:
					return (unsigned char*)"Ethernet";
				case 1:
					return (unsigned char*)"Token Ring";
				case 2:
					return (unsigned char*)"FDDI";
				case 3:
					return (unsigned char*)"ATM";
				case 4:
					return (unsigned char*)"ISDN";
				case 5:
					return (unsigned char*)"WorldFip";
				case 6:
					return (unsigned char*)"PICGM";
				case 0x80:
					return (unsigned char*)"Other Network Controller";
				default:
					return (unsigned char*)"Unknown";
				}
				break;
			case 3:
				//{VGA = 0, XGA = 1, ThreeDController = 2, OtherDisplayController = 0x80};
				switch(dev->Subclass)
				{
				case 0:
					return (unsigned char*)"VGA";
				case 1:
					return (unsigned char*)"XGA";
				case 2:
					return (unsigned char*)"3D Controller";
				default:
					return (unsigned char*)"Unknown";
				}
				break;
			case 4:
				//{VideoDevice = 0, AudioDevice = 1, ComputerTelephonyDevice = 2, OtherMultimediaDevice = 0x80};
				switch(dev->Subclass)
				{
				case 0:
					return (unsigned char*)"Video Device";
				case 1:
					return (unsigned char*)"Audio Device";
				case 2:
					return (unsigned char*)"Computer-Telephony Device";
				case 3:
					return (unsigned char*)"Integrated Audio Controller";
				default:
					return (unsigned char*)"Unknown";
				}
				break;
			case 5:
				//{RAM = 0, Flash = 1};
				switch(dev->Subclass)
				{
				case 0:
					return (unsigned char*)"RAM";
				case 1:
					return (unsigned char*)"Flash";
				default:
					return (unsigned char*)"Unknown";
				}
				break;
			case 6:
			//{HostPCI = 0, PciIsa = 1, PciEisa = 2, PciMicroChannel = 3, PciPci = 4, PciPcmcia = 5,PciNuBus = 6, PciCardbus = 7};
				//RACEWayBridge = 8, SemiTransparentPCI = 9, InfiniBandToPCI = 10, OtherBridgeDevice = 0x80 };
				switch(dev->Subclass)
				{
				case 0:
					return (unsigned char*)"Host->PCI";
				case 1:
					return (unsigned char*)"Pci->Isa";
				case 2:
					return (unsigned char*)"Pci->Eisa";
				case 3:
					return (unsigned char*)"Pci->MicroChannel";
				case 4:
					return (unsigned char*)"Pci->Pci";
				case 5:
					return (unsigned char*)"Pci->Pcmcia";
				case 6:
					return (unsigned char*)"Pci->NuBus";
				case 7:
					return (unsigned char*)"Pci->CardBus";
				case 8:
					return (unsigned char*)"Pci->RACEWayBridge";
				case 9:
					return (unsigned char*)"Pci->SemiTransparentPCI";
				case 10:
					return (unsigned char*)"InfiniBand->PCI";
				default:
					return (unsigned char*)"Unknown";
				}
				break;
			case 7:
				//{Serial = 0, Parallel = 1};
				//MultiPortSerial = 2, Modem = 3, GPIB = 4, SmartCard = 5, OtherCommunicationDevice = 0x80};
				switch(dev->Subclass)
				{
				case 0:
					return (unsigned char*)"Serial";
				case 1:
					return (unsigned char*)"Parallel";
				case 3:
					return (unsigned char*)"Modem";
				case 4:
					return (unsigned char*)"GPIB";
				case 5:
					return (unsigned char*)"SmartCard";
				default:
					return (unsigned char*)"Unknown";
				}
				break;
			case 8:
				//{PIC = 0, DMA = 1, Timer = 2, Clock = 3};
				//PCIHotPlug = 4, SDHostController = 5, OtherSystemPeripheral = 0x80};
				switch(dev->Subclass)
				{
				case 0:
					return (unsigned char*)"PIC";
				case 1:
					return (unsigned char*)"DMA";
				case 2:
					return (unsigned char*)"Timer";
				case 3:
					return (unsigned char*)"Clock";
				case 4:
					return (unsigned char*)"PCIHotPlug";
				case 5:
					return (unsigned char*)"SDHostController";
				default:
					return (unsigned char*)"Unknown";
				}
				break;
			case 9:
				//{Keyboard = 0, Pen = 1, Mouse = 2};
				// Scanner = 3, GamePort = 4, OtherInputDevice = 0x80 };
				switch(dev->Subclass)
				{
				case 0:
					return (unsigned char*)"Keyboard";
				case 1:
					return (unsigned char*)"Pen";
				case 2:
					return (unsigned char*)"Mouse";
				case 3:
					return (unsigned char*)"Scanner";
				case 4:
					return (unsigned char*)"Game Port";
				default:
					return (unsigned char*)"Unknown";
				}
				break;
			case 10:
				// {Generic = 0};
				switch(dev->Subclass)
				{
				case 0:
					return (unsigned char*)"Generic";
				default:
					return (unsigned char*)"Unknown";
				}
				break;
			case 11:
				//{ intel386 = 0x0, intel486 = 0x1, Pentium = 0x2, Alpha = 0x10, PowerPC = 0x20, CoProcessor = 0x40 }
				switch(dev->Subclass)
				{
				case 0:
					return (unsigned char*)"i386";
				case 1:
					return (unsigned char*)"i486";
				case 2:
					return (unsigned char*)"Pentium";
				case 0x10:
					return (unsigned char*)"Alpha";
				case 0x20:
					return (unsigned char*)"PowerPC";
				case 0x30:
					return (unsigned char*)"MIPS";
				case 0x40:
					return (unsigned char*)"CoProcessor";
				default:
					return (unsigned char*)"Unknown";
				}
				break;
			case 12:
				//{ Firewire = 0x0, AccessBus = 0x01, SerialStorage = 0x02, USB = 0x03 };
				// FiberChannel = 0x04, SMBus = 0x05, InfiniBand = 0x06, IPMI = 0x07, SERCOS = 0x08, CANBus = 0x09};
				switch(dev->Subclass)
				{
				case 0:
					return (unsigned char*)"FireWire";
				case 1:
					return (unsigned char*)"AccessBus";
				case 2:
					return (unsigned char*)"SerialStorage";
				case 3:
					return (unsigned char*)"USB";
				case 4:
					return (unsigned char*)"FiberChannel";
				case 5:
					return (unsigned char*)"SMBus";
				case 6:
					return (unsigned char*)"InfiniBand";
				case 7:
					return (unsigned char*)"IPMI";
				case 8:
					return (unsigned char*)"SERCOS";
				case 9:
					return (unsigned char*)"SERCOS";
				case 10:
					return (unsigned char*)"CANBus";
				default:
					return (unsigned char*)"Unknown";
				}
				break;
			case 13:
				//{ iRDA = 0x00, ConsumerIR = 0x01, RF = 0x10, Bluetooth = 0x11, Broadband = 0x12, WifiA = 0x20, WifiB = 0x21, OtherWireless = 0x80 };
				switch(dev->Subclass)
				{
				case 0x00:
					return (unsigned char*)"iRDA";
				case 0x01:
					return (unsigned char*)"Consumer IR";
				case 0x10:
					return (unsigned char*)"RadioFrequency";
				case 0x11:
					return (unsigned char*)"BlueTooth";
				case 0x20:
					return (unsigned char*)"Wifi-A";
				case 0x21:
					return (unsigned char*)"Wifi b/g";
				default:
					return (unsigned char*)"Unknown Wireless device";
				}
				break;
			case 14:
				//{ I20 = 0x00 };
				switch(dev->Subclass)
				{
				case 0:
					return (unsigned char*)"I2O";
				}
				break;
			case 15:
				//{ TV = 0x01, Audio = 0x02, Voice = 0x03, Data = 0x04 };
				switch(dev->Subclass)
				{
				case 0x01:
					return (unsigned char*)"Satellite-TV";
				case 0x02:
					return (unsigned char*)"Satellite-Audio";
				case 0x03:
					return (unsigned char*)"Satellite-Voice";
				case 0x04:
					return (unsigned char*)"Satellite-Data";
				}
				break;
			case 16:
				// { NetworkComuputing = 0x00, Entertainment = 0x10, OtherDecryption = 0x80 };
				switch(dev->Subclass)
				{
				case 0x00:
					return (unsigned char*)"Network Computing Encryption";
				case 0x10:
					return (unsigned char*)"Entertainment Decryption";
				case 0x80:
					return (unsigned char*)"OtherDecryption";
				}
				break;
			case 17:
				//{ DPIO = 0x00, PerformanceCounter = 0x01, CommunicationSynchronization = 0x10, ManagementCard = 0x20,  OtherDataProcessingCard = 0x80 };
				switch(dev->Subclass)
				{
				case 0x00:
					return (unsigned char*)"DPIO";
				case 0x01:
					return (unsigned char*)"Performance Counter";
				case 0x10:
					return (unsigned char*)"Communication Synchronization";
				case 0x20:
					return (unsigned char*)"Management Card";
				case 0x80:
					return (unsigned char*)"Unknown Data Processing Card";
				default:
					return (unsigned char*)"Unknown";
				}
				break;
			default:
				return (unsigned char*)"Unknown";
			}
			return (unsigned char*)"Unknown";
		}
	}
}

PCIStream::PCIStream(unsigned int DeviceNumber, unsigned int bufferSize) : Stream(DeviceNumber,bufferSize)
{
}

PCIStream::~PCIStream()
{
	this->Close();
}

PCIStream* PCIStream::GetPCIStream()
{
	unsigned int numberOfDevices = LunOS::System::GetNumberOfDevices();
	unsigned int i;
	for(i = 0; i < numberOfDevices; i++)
	{
		LunOS::IO::DeviceInfo info = LunOS::System::GetDeviceInfo(i);
		if(info.type == LunOS::IO::DeviceTypes::PCI_BUS)
		{
			return new PCIStream(i,1024);
		}
	}
	return NULL;
}

PCIDevice PCIStream::GetDeviceInformation(unsigned char bus, unsigned char slot, unsigned char function)
{
	PCIDevice dev;
	if(slot >= 32)
	{
		memset(&dev,0,sizeof(PCIDevice));
		return dev;
	}
	this->streamLock.GetLock();
	// we add the 1 to signal
	this->buffer[0] = 1;
	this->buffer[1] = slot;
	this->buffer[2] = bus;
	this->buffer[3] = function;
	this->Read();
	dev = *((PCIDevice*)this->buffer);
	this->streamLock.Release();
	return dev;
}

unsigned int PCIStream::GetDeviceStatus(unsigned char bus, unsigned char slot, unsigned char function)
{
	if(slot >= 32)
	{
		return 0;
	}
	this->streamLock.GetLock();
	// we add the 1 to signal
	this->buffer[0] = 2;
	this->buffer[1] = slot;
	this->buffer[2] = bus;
	this->buffer[3] = function;
	this->Read();
	unsigned int status = *(unsigned short*)this->buffer;
	this->streamLock.Release();
	return status;
}



