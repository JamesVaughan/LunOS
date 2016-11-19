class IO;
struct Device;
struct FDT;
struct FDTEntry;

typedef struct FDT FDT;
typedef struct FDTEntry FDTEntry;
typedef struct Device Device;

#include <kern/system.hpp>
#include <synchronization.h>

#ifndef KERN_IO_H
#define KERN_IO_H

#include <io/Io.h>

// promise IO
class IO;
struct Device;
struct FDT;
struct FDTEntry;

typedef struct FDT FDT;
typedef struct FDTEntry FDTEntry;
typedef struct Device Device;

#include <kern/io/dma.h>
#include <kern/io/pci.h>
#include <kern/io/HardDrive.h>
#include <kern/io/mouse.h>
#include <kern/io/isa.h>
#include <kern/Apic.h>

#define DEVICE_AMMOUNT 100

typedef struct Device
{
	Thread* DeviceThread;
	LunOS::IO::DeviceTypes::DeviceType type;
	bool (*Open)(FDTEntry* entry, unsigned char* buffer, size_t length);
	bool (*ARead)(FDTEntry* entry, unsigned char* buffer, size_t length);
	bool (*Read)(FDTEntry* entry, unsigned char* buffer, size_t length);
	bool (*AWrite)(FDTEntry* entry, unsigned char* buffer, size_t length);
	bool (*Write)(FDTEntry* entry, unsigned char* buffer, size_t length);
	bool (*Init)(struct Device* you, unsigned char* buffer, size_t length);
	bool (*Close)(FDTEntry* entry);
	bool (*Shutdown)(unsigned int reason);
} Device;

//END DEVICE TYPES
/**
 * This is the primary header for I/O, the heart of the LunOS kernel.
 */

class Thread;

typedef struct FDTEntry
{
	Device* device;
} FDTEntry;

//TODO: Balance it out so a process may have more than 32 FDTEnties
typedef struct FDT
{
	Lock* FdtLock;
	FDTEntry Entry[32];
	unsigned int NextAvailable;

	void Init();
	void Release();
} FDT;


typedef struct DeviceList
{
	Device devices[DEVICE_AMMOUNT];
} DeviceList;


//Define IO
class IO
{
private:
	DeviceList deviceList;
public:
	IO();
	void Init();
	// A second phase of initialisation for slower devices so we can use multi-threading
	void InitSecondPass();
	void Route(unsigned char fd, unsigned int callNumber, unsigned char* localDataBuffer, size_t bufferSize);
	void InstallDriver(Device device);
	unsigned int GetNumberOfDevices();
	void StoreDeviceInfo(unsigned int deviceNumber, LunOS::IO::DeviceInfo* devInfo);
};

#endif

