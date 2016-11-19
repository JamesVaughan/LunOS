#include <kern/io/io.h>
#include <prompt.h>

using namespace LunOS::IO;
using namespace LunOS::IO::DeviceTypes;

unsigned int NumberOfDevices= 0;

IO::IO()
{
	this->Init();
}

void IO::Init()
{
	int i = 0;
	NumberOfDevices = 0;
	for(;i < DEVICE_AMMOUNT; i++)
	{
		this->deviceList.devices[i].type = DEVICE_NULL;
		this->deviceList.devices[i].ARead = NULL;
		this->deviceList.devices[i].Read = NULL;
		this->deviceList.devices[i].AWrite = NULL;
		this->deviceList.devices[i].Write = NULL;
		this->deviceList.devices[i].Close = NULL;
		this->deviceList.devices[i].Shutdown = NULL;
	}
	LunOS::Apic::Init();
	keyboard_Init();
}

void IO::InitSecondPass()
{
	printf("Initializing %2PCI\n");
	LunOS::IO::Pci.Init();
	printf("Initializing %2ISA\n");
	LunOS::IO::ISA::Init();
	printf("Initializing %2HDD\n");
	LunOS::IO::BasicHDD::Init();
	printf("Initializing %2VGA\n");
	LunOS::Graphics::Vga::Init();
	printf("Initializing %2Mouse\n");
	LunOS::IO::Kernel::MouseDriver::Init();
}

inline bool ValidDevice(FDT* cur, unsigned char fd)
{
	if(fd > 32) return false;
	if(cur->Entry[fd].device == NULL || cur->Entry[fd].device->type == DEVICE_NULL)
	{
		return false;
	}
	return true;
}

// CALL THIS WHEN fdt is already locked
void ClearEntry(FDT* fdt, unsigned int entryNumber)
{
	fdt->Entry[entryNumber].device = NULL;
	if(entryNumber < fdt->NextAvailable)
	{
		fdt->NextAvailable = entryNumber;
	}
}

void IO::Route(unsigned char fd, unsigned int callNumber, unsigned char* localDataBuffer, size_t bufferSize)
{
	Process* curProcess = Sched->GetActiveProcess();
	FDT* fdt = &curProcess->fdt;
	// Since we already need to go down 1 to offset the System level calls
	fdt->FdtLock->GetLock();

	if(fd == 255)
	{
		if(fdt->NextAvailable != 255)
		{
			// then we want to open something
			fdt->Entry[fdt->NextAvailable].device = &this->deviceList.devices[callNumber];
			fdt->Entry[fdt->NextAvailable].device->Open(&fdt->Entry[fdt->NextAvailable], localDataBuffer, bufferSize);
			// we need to up this by 1 so our negative offset cancels out
			*localDataBuffer = (fdt->NextAvailable + 1);
			for(;fdt->NextAvailable < 32; fdt->NextAvailable++)
			{
				if(fdt->Entry[fdt->NextAvailable].device == NULL)
				{
					break;
				}
			}
			if(fdt->NextAvailable >= 32)
			{
				fdt->NextAvailable = 255;
			}
			fdt->FdtLock->Release();
			return;
		}
		else
		{
			*localDataBuffer = (fdt->NextAvailable);
			return;
		}

	}
	fd = fd - 1;
	if(!ValidDevice(fdt,fd))
	{
		fdt->FdtLock->Release();
		return;
	}
	switch(callNumber)
	{
	// read
	case 1:
		if(fdt->Entry[fd].device->Read)
		{
			fdt->Entry[fd].device->Read(&fdt->Entry[fd],localDataBuffer,bufferSize);
		}
		break;
	// read async
	case 2:
		if(fdt->Entry[fd].device->ARead)
		fdt->Entry[fd].device->ARead(&fdt->Entry[fd],localDataBuffer,bufferSize);
		break;
	// write
	case 3:
		if(fdt->Entry[fd].device->Write)
		fdt->Entry[fd].device->Write(&fdt->Entry[fd],localDataBuffer,bufferSize);
		break;
	// write async
	case 4:
		if(fdt->Entry[fd].device->AWrite)
		fdt->Entry[fd].device->AWrite(&fdt->Entry[fd],localDataBuffer,bufferSize);
		break;
	// close
	case 5:
		if(fdt->Entry[fd].device->Close)
		fdt->Entry[fd].device->Close(&fdt->Entry[fd]);
		ClearEntry(fdt, fd);
		break;
	// shutdown
	case 6:
		if(fdt->Entry[fd].device->Shutdown)
		fdt->Entry[fd].device->Shutdown(*((unsigned int*)localDataBuffer));
		break;
	}
	curProcess->fdt.FdtLock->Release();
}

void IO::InstallDriver(Device device)
{
	int i = 0;
	for(i = 0; i < DEVICE_AMMOUNT; i++)
	{
		if(this->deviceList.devices[i].type == DEVICE_NULL)
		{
			this->deviceList.devices[i] = device;
			NumberOfDevices++;
			break;
		}
	}
}

unsigned int IO::GetNumberOfDevices()
{
	return NumberOfDevices;
}

void IO::StoreDeviceInfo(unsigned int deviceNumber, LunOS::IO::DeviceInfo* devInfo)
{
	if(deviceNumber < NumberOfDevices)
	{
		devInfo->deviceNumber = deviceNumber;
		devInfo->type = this->deviceList.devices[deviceNumber].type;
	}
	else
	{
		devInfo->deviceNumber = -1;
		devInfo->type = LunOS::IO::DeviceTypes::DEVICE_NULL;
	}
}



void FDT::Init()
{
	int i;
	this->FdtLock = new Lock();
	for(i = 0; i < 32; i++)
	{
		ClearEntry(this,i);
	}
	this->NextAvailable = 0;
}

void FDT::Release()
{
	this->FdtLock->GetLock();
	// we don't release the lock, the worst case is that things trying to access this hang
	// but, the only thing that should be accessing it is in fact another thread in the process
	// that we just terminated . . .
	delete this->FdtLock;
}
