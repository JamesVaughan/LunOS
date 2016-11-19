#ifndef IO_PCI_H
#define IO_PCI_H


#include <kern/system.hpp>
#include <io/Pci.h>

namespace LunOS
{
	namespace IO
	{
		class PCI
		{
		private:
			static void Probe();
			static unsigned short ReadWord(unsigned char bus, unsigned char device, unsigned char function, unsigned char offset);
			static void WriteWord(unsigned char bus, unsigned char device, unsigned char function, unsigned char offset, unsigned short value);
			static void WriteDWord(unsigned char bus, unsigned char device, unsigned char function, unsigned char offset, unsigned int value);
			static void StorePCIDeviceData(unsigned char bus, unsigned char device, unsigned char function, unsigned short vendorID);
			static unsigned short GetVendor(unsigned char bus, unsigned char device, unsigned char function);
			static bool Write(FDTEntry* entry, unsigned char* buffer, size_t length);
			static bool Read(FDTEntry* entry, unsigned char* buffer, size_t length);
			static bool Open(FDTEntry* entry, unsigned char* buffer, size_t length);
			static bool Close(FDTEntry* entry);
		public:
			static void Init();
			static void Update(unsigned char bus, unsigned char slot, unsigned char function);
			static unsigned int GetMemoryAddress(unsigned int BAR);
			static unsigned int GetIOAddress(unsigned int BAR);
			static void WriteBAR(unsigned char bus, unsigned char slot, unsigned char function, unsigned char bar, unsigned int value);
			static PCIDevice pciData[256][32][4];
			static bool ActivePCIBus[256];
		};

		extern PCI Pci;
	}
}


#endif
