/*
 * Pci.h
 *
 *  Created on: 19-Dec-2009
 *      Author: james
 */

#ifndef PCI_H_
#define PCI_H_

#include <io/Stream.h>

namespace LunOS
{
	namespace IO
	{
		namespace PCIClassCodes
		{
			enum ClassCodes{OldDevices = 0, MassStorageController = 1, NetworkController = 2, DisplayController = 3,
				MultimediaDevice = 4, MemoryController = 5, BrideDevice = 6, SimpleCommunicationsController = 7,
				BaseSystemPeripherals = 8, InputDevices = 9, DockingStations = 10, Processor = 11, SerialBusController = 12,
				WirelessController = 13, IntelegentIOController = 14, SatelliteCommunicationController = 15, EncryptionDecryptionController = 16,
				SignalProcessingController = 17, Misc = 0xff};
			enum MassStorageControllers {SCSI = 0, IDE = 1, FloppyDisk = 2, IPI = 3, RAID = 4, ATA = 5, SATA = 6, SAS = 7, OtherMassStorage = 0x80 };
			enum NetworkControllers {Ethernet = 0, TokenRing = 1, FDDI = 2, ATM = 3, ISDN = 4, WorldFip = 5, PICGM = 6, OtherNetworkController = 0x80 };
			enum DisplayControllers {VGA = 0, XGA = 1, ThreeDController = 2, OtherDisplayController = 0x80};
			enum MultimediaDevices {VideoDevice = 0, AudioDevice = 1, ComputerTelephonyDevice = 2, IntegratedAudioController = 3, OtherMultimediaDevice = 0x80};
			enum MemoryControllers {RAM = 0, Flash = 1};
			enum BrideDevices {HostPCI = 0, PciIsa = 1, PciEisa = 2, PciMicroChannel = 3, PciPci = 4, PciPcmcia = 5,
				PciNuBus = 6, PciCardbus = 7, RACEWayBridge = 8, SemiTransparentPCI = 9, InfiniBandToPCI = 10, OtherBridgeDevice = 0x80 };
			enum SimpleCommunications {Serial = 0, Parallel = 1, MultiPortSerial = 2, Modem = 3, GPIB = 4, SmartCard = 5, OtherCommunicationDevice = 0x80};
			enum BaseSystemPeripherals {PIC = 0, DMA = 1, Timer = 2, Clock = 3, PCIHotPlug = 4, SDHostController = 5, OtherSystemPeripheral = 0x80};
			enum InputDevices {Keyboard = 0, Pen = 1, Mouse = 2, Scanner = 3, GamePort = 4, OtherInputDevice = 0x80 };
			enum DockingStations {Generic = 0, OtherDockingStation = 0x80};
			enum Processors { intel386 = 0x0, intel486 = 0x1, Pentium = 0x2, Alpha = 0x10, PowerPC = 0x20,
				MIPS = 0x30, CoProcessor = 0x40	};
			enum SerialBusControllers { Firewire = 0x0, AccessBus = 0x01, SerialStorage = 0x02, USB = 0x03,
				FiberChannel = 0x04, SMBus = 0x05, InfiniBand = 0x06, IPMI = 0x07, SERCOS = 0x08, CANBus = 0x09};
			enum WirelessControllers { iRDA = 0x00, ConsumerIR = 0x01, RF = 0x10, Bluetooth = 0x11, Broadband = 0x12, WifiA = 0x20, WifiB = 0x21, OtherWireless = 0x80 };
			enum IntellegentIOControllers { I20 = 0x00 };
			enum SatelliteCommunicationControllers { TV = 0x01, Audio = 0x02, Voice = 0x03, Data = 0x04 };
			enum EncryptionDecryptionControllers { NetworkComuputing = 0x00, Entertainment = 0x10, OtherDecryption = 0x80 };
			enum SignalProcessingControllers { DPIO = 0x00, PerformanceCounter = 0x01, CommunicationSynchronization = 0x10, ManagementCard = 0x20,  OtherDataProcessingCard = 0x80 };


		}
		namespace PCIDeviceStatus
		{
			enum StatusBits {IOSpace = 0x01, MemorySpace = 0x02, BusMaster = 0x04, SpecialCycles = 0x08, MemWriteInvalidate = 0x10, VGASnoop = 0x20, ParityError = 0x40, SERREnable = 0x100, FastBackToBack = 0x200, InterruptDisable = 0x400};
		}
		typedef struct
		{
			unsigned short VendorNumber;
			unsigned short DeviceNumber;

			unsigned char RevisionID;
			unsigned char ProgramIF;
			unsigned char Subclass;
			unsigned char ClassCode;

			unsigned char CacheLineSize;
			unsigned char LatencyTimer;
			unsigned char HeaderType;
			unsigned char BIST;

			// Base Address Register
			unsigned int BAR[6];

			unsigned int CardbusCSIPointer;
			unsigned int ExpansionROMBaseAddress;

			unsigned char CapabilityPointer;
			unsigned char InterruptLine;
			unsigned char InterruptPin;
			// Keeps this structure align 4
			unsigned char RESERVED;

		} PCIDevice;

		extern unsigned char* GetClassName(PCIDevice* dev);
		extern unsigned char* GetSubclassName(PCIDevice* dev);

		class PCIStream : public Stream
		{
		public:
			static PCIStream* GetPCIStream();
			PCIDevice GetDeviceInformation(unsigned char bus, unsigned char slot, unsigned char function);
			unsigned int GetDeviceStatus(unsigned char bus, unsigned char slot, unsigned char function);
		private:
			PCIStream(unsigned int DeviceNumber, unsigned int bufferSize);
			~PCIStream();
		};
	}
}


#endif /* PCI_H_ */
