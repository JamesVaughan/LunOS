#ifndef Kern_IO_HARDDRIVE_H
#define Kern_IO_HARDDRIVE_H

#include <kern/io/io.h>
#include <io/DiskAccess.h>
#include <synchronization.h>

namespace LunOS
{
namespace IO
{

	class BasicHDD
	{
		static Lock DiskLocks[2];
	public:
		static void Init();
		static bool Open(FDTEntry* fdt, unsigned char* buffer, size_t length);
		static bool Read(FDTEntry* fdt, unsigned char* buffer, size_t length);
		static bool Write(FDTEntry* fdt, unsigned char* buffer, size_t lengt);
		static bool Close(FDTEntry* entry);
	private:
		static DiskInfo DiskInformation[8];
		static void ScanBuses();
		static void ScanDrives(int bus);
		static bool WaitForDrive();
		static void ScanAndLoadFromPCI();
		static unsigned short IdePIOLocations[4];
		static unsigned short IdePIOControlRegisters[4];
	};
}
}

#endif
