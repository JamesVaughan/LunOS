#include <kern/system.hpp>
#include <SysCall.h>
#include <prompt.h>
#include <io/Graphics.h>
#include <io/DiskAccess.h>
#include <io/FileSystem.h>
#include <io/TextReader.h>
#include <io/Fat32.h>
#include <Minesweeper.h>
#include <io/Pci.h>
#include <kern/Apic.h>
#include <io/Mouse.h>
#include <Threading/Tasks.hpp>

using namespace LunOS::Calls;

// just declare it here
IO System::io;

// The one, the only, the amazing IDLE THREAD!!!!!
void IdleThread(void* param)
{
	for (;;)
	{
		Sched->HandelIdleThread();
	}
}

// INTERNEL SUPPORT FOR USB
void USBHost(void* param)
{
	for (;;)
	{
		//POLL FOR USB
		//Write if we found anything
		//Go back to another task
		LunOS::System::Sleep(1000);
	}
}

void PrintPrimes(unsigned int number)
{
	unsigned int i = 2;
	unsigned int amount = 0;
	printf("%4%i:", number);
	unsigned int half = number / 2;
	while (i <= half)
	{
		if (number % i == 0)
		{
			number /= i;
			amount++;
			printf("%2 %i", i);
		}
		else
		{
			i++;
		}
	}
	if (amount == 1)
	{
		printf("%2%i%4******%2Prime!%4*********", number);
		// Sleep for a bit after we get a prime
		LunOS::System::Sleep(500);
	}
	putchConsoleText('\n');
}

void runPrimes()
{
	int i = 2;
	LunOS::IO::KeyEvent k;
	LunOS::IO::Keyboard* keyboard = LunOS::IO::Keyboard::GetKeyboardStream();
	for (;; i++)
	{
		LunOS::System::Yield();
		PrintPrimes(i);
		k = keyboard->TryReadKey();
		if (k.type == KEY_DOWN)
		{
			return;
		}
		else if (k.type != KEY_NO_EVENT)
		{
			getKeyEvent();
		}
	}
	keyboard->Close();
}

void reboot()
{
	powerManagement.Reboot();
}

void useDynamicMemory()
{
	Memory::printDynamicMemoryState();
	int *f, *s;
	printf("%2Allocation of f, 150 ints.\n");
	f = new int[150];
	Memory::printDynamicMemoryState();
	printf("%2Allocation of s, 130 ints.\n");
	s = new int[130];
	Memory::printDynamicMemoryState();
	printf("%2Releasing of f\n");
	delete[] f;
	Memory::printDynamicMemoryState();
	printf("%2Releasing of s\n");
	delete[] s;
	Memory::printDynamicMemoryState();
}

void ReadMemory(int argc, unsigned char** argv)
{
	int i;
	if (argc < 1)
	{
		printf("\n%3USAGE: readmemory [memoryAddress]");
		return;
	}
	LunOS::String s;
	for (i = 0; i < argc; i++)
	{
		s = argv[i];
		unsigned int res;
		if (s.toInt(&res))
		{
			printf("\n%2%s: 0x%x", s.toCharArray(), *((unsigned int*) res));
		}
		else if (i == 0)
		{
			printf("\n%3USAGE: readmemory [memoryAddress]\n");
			printf("A memory address needs to be in base 10 or in base 16.");
		}
		else
		{
			printf("\n%3Parameter %i is not a number", i);
		}
	}
}

void ReadMemoryAllocation(int argc, unsigned char** argv)
{
	int i;
	if (argc < 1)
	{
		printf("\n%3USAGE: readmemory [memoryAddress]");
		return;
	}
	LunOS::String s;
	for (i = 0; i < argc; i++)
	{
		s = argv[i];
		unsigned int res;
		if (s.toInt(&res))
		{
			Memory::printMemoryAllocation((void*) res);
		}
		else if (i == 0)
		{
			printf("\n%3USAGE: readmemoryallocation [memoryAddress]\n");
			printf("A memory address needs to be in base 10 or in base 16.");
		}
		else
		{
			printf("\n%3Parameter %i is not a number", i);
		}
	}
}

void ShowUsers()
{
	unsigned int* users = LunOS::System::GetActiveUsers();
	unsigned int i = 0;
	printf("There are %2%i %1users:\n", users[0]);
	// this is <= since we don't count user[0]
	for (i = 1; i <= users[0]; i++)
	{
		const char* userName;
		printf("%2%i%1:%3%s\n", users[i], (userName =
				LunOS::System::GetUserName(users[i])));
		delete[] userName;
	}
	delete[] users;
}

void CreateNewUserProgram(int argc, unsigned char** argv)
{
	if (argc != 2)
	{
		printf("%3USAGE: NAME PASSWORD\n");
		return;
	}
	printf("\nPlease enter your %2password%1:");
	char buff[80];
	buff[0] = 0;
	int i = 0;
	while (true)
	{
		// sleep and wait for input
		LunOS::System::Yield();
		LunOS::IO::KeyEvent k = getKeyEvent();
		if (k.type == KEY_DOWN)
		{
			if (k.key == VK_Enter)
			{
				unsigned int uid = 0;
				if ((uid = LunOS::System::CreateNewUser((const char*) argv[0],
						(const char*) argv[1], (const char*) buff)) <= 0)
				{
					printf("\n%3Failed to create user, password was invalid");
					return;
				}
				else
				{
					printf("\n%2Successfully %1created user with ID %2%i!\n",
							uid);
					return;
				}
			}
			else if (k.key == VK_Backspace)
			{
				if (i > 0)
				{
					buff[i--] = 0;
				}
			}
			else if (k.key >= 'a' && k.key <= 'z' && i < 79)
			{
				buff[i++] = k.key;
				buff[i] = 0;
			}
		}
	}
}

void UserReadMSR(int argc, unsigned char** argv)
{
	if (argc != 1)
	{
		printf("%3USAGE: Register\n");
		return;
	}
	long long value;
	unsigned int reg;
	LunOS::String s;
	s = argv[0];
	if (!s.toInt(&reg))
	{
		printf("%3Invalid Register!\n");
		return;
	}
	ReadMSR(reg, &value);
	printf("%2=>%x", (unsigned int) value);
}

void UserWriteMSR(int argc, unsigned char** argv)
{
	if (argc != 3)
	{
		printf("%3USAGE: Register HigherOrder LowerOrder\n");
		return;
	}
	long long value;
	unsigned int reg, temp;
	LunOS::String s;
	s = argv[0];
	if (!s.toInt(&reg))
	{
		printf("%3Invalid Register!\n");
		return;
	}
	s = argv[1];
	if (!s.toInt(&temp))
	{
		printf("%3Problems reading Higher Order Bits\n");
		return;
	}
	value = temp;
	value = value << 32;
	s = argv[2];
	if (!s.toInt(&temp))
	{
		printf("%3Problems reading Lower Order Bits\n");
		return;
	}
	value += temp;
	WriteMSR(reg, &value);
	printf("%2Done");
}

void GMode(int argc, unsigned char** argv)
{
	using namespace LunOS::IO;
	using namespace LunOS::Graphics;
	int i;

	// Get our default disk
	DiskAccess *disk = LunOS::IO::DiskAccess::GetDiskStream(0);
	if (!disk)
	{
		printf("%3The primary disk does not exist!\n");
		return;
	}
	GraphicsStream* vga = LunOS::IO::GraphicsStream::GetGraphicsConnection();
	// Get the first partition on the disk
	FileSystem *fs = LunOS::IO::FileSystem::GetFileSystem(disk, 1);
	Keyboard* keyboard = LunOS::IO::Keyboard::GetKeyboardStream();
	Mouse* mouse = LunOS::IO::Mouse::GetMouseStream();
	Bitmap* image = Bitmap::LoadBitmap("LUNOS/SYS/BACKGR~1.BMP", fs);
	Bitmap* cursor = Bitmap::LoadBitmap("LUNOS/SYS/CURSOR.BMP", fs);
	Font* font = Font::LoadFont("LUNOS/SYS/BASICF~1.FNT", fs);
	if (!image)
	{
		printf("Unable to load the background image!\n");
		return;
	}
	if (!font)
	{
		printf("Unable to load the system font!\n");
		return;
	}
	Colour black;
	black.R = black.G = black.B = black.A = 0;
	cursor->SetTransparentColor(black);
	vga->ChangeToGraphicsMode();
	vga->Flush();
	unsigned char ScreenBuffer[25][81];
	memset(&ScreenBuffer, ' ', 81 * 25);
	for (i = 0; i < 25; i++)
	{
		ScreenBuffer[i][80] = 0;
	}
	for (i = 0; i < 26; i++)
	{
		ScreenBuffer[0][i] = 'A' + i;
		ScreenBuffer[1][i] = 'a' + i;
	}
	for (i = 0; i < 10; i++)
	{
		ScreenBuffer[2][i] = '0' + i;
	}
	while (true)
	{
		MouseStatus mouseStatus = mouse->GetStatus();
		vga->DrawBitmap(image, 0, 0, 800, 600);
		/*for(i = 0; i < 25; i++)
		 {
		 vga->DrawString(font, ((unsigned char*)ScreenBuffer + 81 * i),
		 0,0,0,1, font->GetHeight('A')* i + 1);
		 vga->DrawString(font, ((unsigned char*)ScreenBuffer + 81 * i),
		 0,255,0,0, font->GetHeight('A')* i);
		 }*/
		unsigned char* helloWorld = (unsigned char*) "I LOVE MY WIFE";
		int width = font->GetWidth(helloWorld);
		vga->DrawRectangle(0, 0, 0, 200, 200, width + 6, 25);
		vga->DrawRectangle(25, 25, 25, 201, 201, width + 4, 23);
		vga->DrawString(font, helloWorld, 50, 150, 50, 203, 205);
		vga->DrawLine(0, 0, 100, 100, 1, 2, 3);
		vga->DrawBitmap(cursor, mouseStatus.X, mouseStatus.Y,
				cursor->GetWidth(), cursor->GetHeight());
		vga->Flush();
		LunOS::System::Yield();
	}
	font->Close();
	image->Close();
	cursor->Close();
	fs->Close();
	disk->Close();
	keyboard->Close();
	mouse->Close();
	vga->Close();
}

void ReadPCIInfo(int argc, unsigned char** argv)
{
	unsigned int bus, dev, function;
	bool first = true;
	LunOS::IO::PCIStream* pciStream = LunOS::IO::PCIStream::GetPCIStream();
	if (argc >= 4 && argv[0][0] == '-' && argv[0][1] == 'd')
	{
		LunOS::IO::PCIDevice pciData;
		LunOS::String s;
		s = argv[1];
		s.toInt(&bus);
		s = argv[2];
		s.toInt(&dev);
		s = argv[3];
		s.toInt(&function);
		pciData = pciStream->GetDeviceInformation(bus, dev, function);
		printf(
				"PciDevice %2%i%1:%2%i%1:%2%i\n%4Vendor Number%1:%20x%x%1\t%4Device Number%1:%20x%x\n%4Revision ID%1:%20x%i\t%4Program IF%1:%20x%i\n%4Subclass%1:%20x%i\t%4Class Code%1:%20x%i\n%4Cache Line Size%1:%20x%i\t%4Latancy Timer%1:%20x%i\n%4Header Type%1:%20x%i\t%4BIST%1:%20x%i\n%4BAR0%1:%20x%x\t%4BAR1%1:%20x%x\n%4BAR2%1:%20x%x\t%4BAR3%1:%20x%x\n%4BAR4%1:%20x%x\t%4BAR5%1:%20x%x\n%4CardBus CSI Pointer%1:%20x%x\t%4Expansion ROM Base Pointer%1:%20x%x\n%4CapabilityPointer%1:%20x%x\t%4Interrupt Line%1:%20x%i\n%4Interrupt Pin%1:%20x%i\n",
				bus, dev, function, pciData.VendorNumber, pciData.DeviceNumber,
				pciData.RevisionID, pciData.ProgramIF, pciData.Subclass,
				pciData.ClassCode, pciData.CacheLineSize, pciData.LatencyTimer,
				pciData.HeaderType, pciData.BIST, pciData.BAR[0],
				pciData.BAR[1], pciData.BAR[2], pciData.BAR[3], pciData.BAR[4],
				pciData.BAR[5], pciData.CardbusCSIPointer,
				pciData.ExpansionROMBaseAddress, pciData.CapabilityPointer,
				pciData.InterruptLine, pciData.InterruptPin);
	}
	else if (argc >= 4 && argv[0][0] == '-' && argv[0][1] == 's')
	{
		using namespace LunOS::IO;
		LunOS::String s;
		s = argv[1];
		s.toInt(&bus);
		s = argv[2];
		s.toInt(&dev);
		s = argv[3];
		s.toInt(&function);
		unsigned int status = pciStream->GetDeviceStatus(bus, dev, function);
		printf("Device status of %2%i%1:%2%i\n", bus, dev);
		bool first = true;
		if (PCIDeviceStatus::IOSpace & status)
		{
			printf(first ? "%2IO Space" : "%2, IO Space");
			first = false;
		}
		if (PCIDeviceStatus::MemorySpace & status)
		{
			printf(first ? "%2Memory Space" : "%2, Memory Space");
			first = false;
		}
		if (PCIDeviceStatus::BusMaster & status)
		{
			printf(first ? "%2BusMaster" : "%2, BusMaster");
			first = false;
		}
		if (PCIDeviceStatus::SpecialCycles & status)
		{
			printf(first ? "%2SpecialCycles" : "%2, SpecialCycles");
			first = false;
		}
		if (PCIDeviceStatus::MemWriteInvalidate & status)
		{
			printf(first ? "%2MemWriteInvalidate" : "%2, MemWriteInvalidate");
			first = false;
		}
		if (PCIDeviceStatus::VGASnoop & status)
		{
			printf(first ? "%2VGASnoop" : "%2, VGASnoop");
			first = false;
		}
		if (PCIDeviceStatus::ParityError & status)
		{
			printf(first ? "%2ParityError" : "%2, ParityError");
			first = false;
		}
		if (PCIDeviceStatus::SERREnable & status)
		{
			printf(first ? "%2SERREnable" : "%2, SERREnable");
			first = false;
		}
		if (PCIDeviceStatus::FastBackToBack & status)
		{
			printf(first ? "%2FastBackToBack" : "%2, FastBackToBack");
			first = false;
		}
		if (PCIDeviceStatus::InterruptDisable & status)
		{
			printf(first ? "%2InterruptDisable" : "%2, InterruptDisable");
			first = false;
		}

	}
	else if (argc == 3)
	{
		LunOS::IO::PCIDevice pciData;
		LunOS::String s;
		s = argv[0];
		s.toInt(&bus);
		s = argv[1];
		s.toInt(&dev);
		s = argv[2];
		s.toInt(&function);
		pciData = pciStream->GetDeviceInformation(bus, dev, function);
		printf("PciDevice %2%i%1:%2%i%1:%2%i\n%4Device Type%1:%2%s", bus, dev,
				function, LunOS::IO::GetSubclassName(&pciData));
	}
	else
	{
		// if we have no arguments just write out all of the valid PCI number
		int count = 0;
		for (bus = 0; bus < 256; bus++)
		{
			for (dev = 0; dev < 32; dev++)
			{
				for (function = 0; function < 4; function++)
				{
					//printf("%i\n", function);
					LunOS::IO::PCIDevice device =
							pciStream->GetDeviceInformation(bus, dev, function);
					if (device.VendorNumber != 0)
					{
						if (first)
						{
							printf("Devices found at %2%i%1:%2%i%1:%2%i", bus,
									dev, function);
							first = false;
						}
						else
						{
							printf(", %2%i%1:%2%i%1:%2%i", bus, dev, function);
						}
						count++;
						if (count >= 6)
						{
							printf("\n");
							count = 0;
						}
					}
				}
			}
		}
	}
	pciStream->Close();
}

void UserWriteMemory(int argc, unsigned char** argv)
{
	if (argc != 2)
	{
		printf("%3Usage: address value");
		return;
	}
	unsigned int *addr, value;

	LunOS::String s;
	s = argv[0];
	s.toInt((unsigned int*) (&addr));
	s = argv[1];
	s.toInt(&value);
	*addr = value;
}

void UserWriteIOB(int argc, unsigned char** argv)
{
	if (argc != 2)
	{
		printf("%3Usage: address value");
		return;
	}
	unsigned int addr, value;
	LunOS::String s;
	s = argv[0];
	s.toInt(&addr);
	s = argv[1];
	s.toInt(&value);
	outportb(addr, value);
}

void UserWriteIOW(int argc, unsigned char** argv)
{
	if (argc != 2)
	{
		printf("%3Usage: address value");
		return;
	}
	unsigned int addr, value;
	LunOS::String s;
	s = argv[0];
	s.toInt(&addr);
	s = argv[1];
	s.toInt(&value);
	outportw(addr, value);
}

void UserWriteIOD(int argc, unsigned char** argv)
{
	if (argc != 2)
	{
		printf("%3Usage: address value");
		return;
	}
	unsigned int addr, value;
	LunOS::String s;
	s = argv[0];
	s.toInt(&addr);
	s = argv[1];
	s.toInt(&value);
	outportd(addr, value);
}

void UserReadIOB(int argc, unsigned char** argv)
{
	if (argc != 1)
	{
		printf("%3Usage: address");
		return;
	}
	unsigned int addr, value;
	LunOS::String s;
	s = argv[0];
	s.toInt(&addr);
	value = inportb(addr);
	printf("%2%i", value);
}

void UserReadIOW(int argc, unsigned char** argv)
{
	if (argc != 1)
	{
		printf("%3Usage: address");
		return;
	}
	unsigned int addr, value;
	LunOS::String s;
	s = argv[0];
	s.toInt(&addr);
	s = argv[1];
	value = inportw(addr);
	printf("%2%i", value);
}

void UserReadIOD(int argc, unsigned char** argv)
{
	if (argc != 1)
	{
		printf("%3Usage: address");
		return;
	}
	unsigned int addr, value;
	LunOS::String s;
	s = argv[0];
	s.toInt(&addr);
	value = inportd(addr);
	printf("%2%i", value);
}

void MultiThreadThread(void* param)
{
	LunOS::System::Sleep(500);
	*((volatile unsigned int*) param) = 2;
}

void MultiThreadThread2(void* param)
{
	while (*((volatile unsigned int*) param) != 2)
	{
		printf("Waiting for param to be equal to 2!\n");
		LunOS::System::Sleep(500);
	}
	*((volatile unsigned int*) param) = 0;
}

void MultiThreads()
{
	using namespace LunOS::Tasks;
	volatile int first = 10;
	Threadpool* factory = Threadpool::GetDefault();
	for (int i = 0; i < 10; i++)
	{
		first = 10;
		printf("Starting iteration %2%i\n", i);
		printf("Allocating Tasks\n");
		Task* task1 = new Task(MultiThreadThread, (void*) &first, false);
		Task* task2 = new Task(MultiThreadThread2, (void*) &first, false);
		printf("Starting Tasks\n");
		factory->StartTask(task1);
		factory->StartTask(task2);
		printf("Waiting on first\n");
		while (((volatile int) first) != 0)
		{
			LunOS::System::Sleep(18);
		}
		Task::Wait(task1);
		Task::Wait(task2);
		delete task1;
		delete task2;
		LunOS::System::Sleep(5000);
	}
	printf("%2Complete\n");
}

void TestQueue(void* params)
{
	//auto queue = LunOS::DataStructures::Queue<int>();

}

void ShowThreads()
{
	Sched->PrintOutThreads();
}

void ReadTextFile(int argc, unsigned char** argv)
{
	using namespace LunOS::IO;
	if (argc < 1)
	{
		printf("%3USAGE: TextFileName");
		return;
	}
	DiskAccess* disk = DiskAccess::GetDiskStream(0);
	if (!disk->GetDiskInformation().Exists)
	{
		disk->Close();
		printf("%3Primary Disk Does not Exist!");
		return;
	}
	FileSystem* fs = FileSystem::GetFileSystem(disk, 1);
	if (fs == NULL)
	{
		disk->Close();
		printf("%3File System does not exit!\n");
		return;
	}
	// Now that we have the file system activated we can load a file
	File* file = fs->LoadFile((const char*) argv[0], 0);
	if (file == NULL)
	{
		fs->Close();
		disk->Close();
		printf("%3FILE NOT FOUND!");
		return;
	}
	unsigned char* buff = new unsigned char[0x1000];
	unsigned int ammountRead = 0;
	TextReader* reader = TextReader::LoadTextReader(file, fs);
	Keyboard* keys = Keyboard::GetKeyboardStream();
	while (reader->LoadLine(buff, 0x1000, &ammountRead) || ammountRead != 0)
	{
		printf("%s\n", buff);
		KeyEvent e;
		bool exit = false;
		while ((e = keys->ReadKeyEvent()).type == KEY_NO_EVENT
				|| (e.key != VK_Down && e.key != VK_Escape))
		{
			LunOS::System::Sleep(20);
		}
		exit = e.key == VK_Escape;
		if (exit)
			break;
	}
	keys->Close();
	fs->Close();
	disk->Close();
	delete buff;
}

void TestDiskAccess(int argc, unsigned char** argv)
{
	using namespace LunOS::IO;
	using namespace LunOS::IO::FileSystems;
	DiskAccess* disk = DiskAccess::GetDiskStream(0);
	DiskInfo diskInformation = disk->GetDiskInformation();
	// Check to see if the disk exists
	if (!diskInformation.Exists)
	{
		printf("%3The primary disk does not exist!\n");
		disk->Close();
		return;
	}
	printf("Argc == %i\n", argc);
	if (argc == 1)
	{
		int i = 1;
		Fat32* f = null;
		for (; i <= 4; i++)
		{
			printf("Looking at partition %i\n", i);
			f = (Fat32*) FileSystem::GetFileSystem(disk, i);
			if (f != null)
			{
				break;
			}
		}
		if (f == null)
		{
			disk->Close();
			return;
		}
		f->PrintFat32Information();
		f->Close();
	}
	else if (argc >= 2)
	{
		unsigned char buff[512];
		unsigned int lba;
		LunOS::String s;
		s = argv[argc == 2 ? 1 : 2];
		int display = 0;
		int sectionSize = 8;
		int numberOfSections = 4;
		if (argc > 2)
		{
			if (strncmp("-b", (const char*) argv[1], 2) == 0)
			{
				display = 1;
				numberOfSections = 3;
			}
			else if (strncmp("-w", (const char*) argv[1], 2) == 0)
			{
				display = 2;
				numberOfSections = 3;
			}
			else if (strncmp("-d", (const char*) argv[1], 2) == 0)
			{
				display = 3;
				numberOfSections = 3;
			}
		}
		if (!s.toInt(&lba))
		{
			Fat32* f = (Fat32*) FileSystem::GetFileSystem(disk, 1);
			if (f == null)
			{
				printf("%3Partition was unreadable!\n");
				disk->Close();
				return;
			}
			// print the file that was asked for
			f->PrintFile((char*) argv[1]);
			f->Close();
			disk->Close();
			return;
		}
		disk->ReadDisk(buff, 1, lba);
		int i, j;
		unsigned int pos = 0;

		for (pos = 0; pos < 512;)
		{
			for (j = 0; j < numberOfSections; j++)
			{
				if (j > 0)
					printf("|");
				for (i = 0; i < sectionSize; i++)
				{
					switch (display)
					{
					case 0:
					{
						printf("%2%c", buff[pos++]);
					}
						break;
					case 1:
					{
						printf("%2%x,",
								(unsigned int) (*(unsigned char*) (buff + pos)));
						pos += 1;
					}
						break;
					case 2:
					{
						printf("%2%x,",
								(unsigned int) (*(unsigned short*) (buff + pos)));
						pos += 2;
					}
						break;
					case 3:
					{
						printf("%2%x,",
								(unsigned int) (*(unsigned int*) (buff + pos)));
						pos += 4;
					}
						break;
					}
				}
			}
			printf("\n");
		}
	}
	disk->Close();
}

void MouseTestApp()
{
	using namespace LunOS::IO;
	Mouse* mouse = Mouse::GetMouseStream();
	Keyboard* keyboard = Keyboard::GetKeyboardStream();
	KeyEvent event;
	while ((event = keyboard->TryReadKey()).type != KEY_DOWN)
	{
		MouseStatus status = mouse->GetStatus();
		printf("\r%2%s%1:%2%s%1:%2%s%1:%4%i%1:%4%i",
				status.Buttons[0] ? "true" : "false",
				status.Buttons[1] ? "true" : "false",
				status.Buttons[2] ? "true" : "false", status.X, status.Y);
		LunOS::System::Sleep(50);
	}
	mouse->Close();
}

const char* version = "0.05.04";
void Prompt(void* unusedParam)
{
	CommandPrompt commandPrompt;
	commandPrompt.Init();

	Command c;
	c.command = reboot;
	c.name = "reboot";
	c.params = false;
	commandPrompt.AddCommand(&c);

	c.paramCommand = GMode;
	c.name = "gfx";
	c.params = true;
	commandPrompt.AddCommand(&c);

	c.command = useDynamicMemory;
	c.name = "dynamic memory test";
	c.params = false;
	commandPrompt.AddCommand(&c);

	c.command = runPrimes;
	c.name = "primes";
	commandPrompt.AddCommand(&c);

	c.paramCommand = ReadMemoryAllocation;
	c.params = true;
	c.name = "readmemoryallocation";
	commandPrompt.AddCommand(&c);

	c.paramCommand = ReadMemory;
	c.params = true;
	c.name = "readmemory";
	commandPrompt.AddCommand(&c);

	c.paramCommand = UserWriteMemory;
	c.params = true;
	c.name = "writememory";
	commandPrompt.AddCommand(&c);

	c.paramCommand = UserWriteIOB;
	c.params = true;
	c.name = "writeiob";
	commandPrompt.AddCommand(&c);

	c.paramCommand = UserWriteIOW;
	c.params = true;
	c.name = "writeiow";
	commandPrompt.AddCommand(&c);

	c.paramCommand = UserWriteIOD;
	c.params = true;
	c.name = "writeiod";
	commandPrompt.AddCommand(&c);

	c.paramCommand = UserReadIOB;
	c.params = true;
	c.name = "readiob";
	commandPrompt.AddCommand(&c);

	c.paramCommand = UserReadIOW;
	c.params = true;
	c.name = "readiow";
	commandPrompt.AddCommand(&c);

	c.paramCommand = UserReadIOD;
	c.params = true;
	c.name = "readiod";
	commandPrompt.AddCommand(&c);

	c.paramCommand = ReadPCIInfo;
	c.params = true;
	c.name = "readpci";
	commandPrompt.AddCommand(&c);

	c.paramCommand = CreateNewUserProgram;
	c.params = true;
	c.name = "createuser";
	commandPrompt.AddCommand(&c);

	c.paramCommand = UserReadMSR;
	c.params = true;
	c.name = "readmsr";
	commandPrompt.AddCommand(&c);

	c.paramCommand = UserWriteMSR;
	c.params = true;
	c.name = "writemsr";
	commandPrompt.AddCommand(&c);

	c.command = ShowUsers;
	c.params = false;
	c.name = "getuserids";
	commandPrompt.AddCommand(&c);

	c.command = MultiThreads;
	c.params = false;
	c.name = "multithreading test";
	commandPrompt.AddCommand(&c);

	c.command = ShowThreads;
	c.params = false;
	c.name = "show threads";
	commandPrompt.AddCommand(&c);

	c.paramCommand = TestDiskAccess;
	c.params = true;
	c.name = "disk test";
	commandPrompt.AddCommand(&c);

	c.command = MouseTestApp;
	c.params = false;
	c.name = "mousetest";
	commandPrompt.AddCommand(&c);

	c.paramCommand = ReadTextFile;
	c.params = true;
	c.name = "readText";
	commandPrompt.AddCommand(&c);

	c.paramCommand = Minesweeper::MineSweeperMain;
	c.params = true;
	c.name = "Minesweeper";
	commandPrompt.AddCommand(&c);

	unsigned int uid = LunOS::System::GetUserID();
	const char* userName = LunOS::System::GetUserName(uid);
	printf("%2Loaded Lunos version-%s\n", version);
	printf("Running under user %2%s %1id %2%i\n", userName, uid);
	//LunOS::Apic::PrintInformation();
	// we need to do this since we already drew to the screen before the command prompt has run
	commandPrompt.ResetPosition();
	for (;;)
	{
		commandPrompt.Run();
		LunOS::System::Yield();
	}
	delete[] userName;
}

extern "C"
{
void Sys_Call(unsigned int ecx, unsigned int ebx, unsigned int eax);
}

void System::Call(unsigned char fd, unsigned int callNumber,
		unsigned char* buffer, size_t bufferSize)
{
	Sys_Call(bufferSize, (unsigned int) buffer, ((callNumber << 8) + fd));
}

void System::SystemStart(void* params)
{
	// Initialize the rest of the devices
	System::io.Init();
	System::io.InitSecondPass();
	Sched->CreateThread((unsigned char*) "USB Host", (void*) USBHost, NULL);
	Sched->CreateThread((unsigned char*) "LunOS CommandPrompt", (void*) Prompt,
			NULL);
}

// The true start of the Operating System
void System::Init()
{
	irq_install_handler(16, System::IncommingInterrupt);
	User* user = new User("mylord");
	user->InitUsers();
	user->name = "Root";
	user->privileges = ~0;
	user->userID = 0;
	User::users->AddLast(user);
	Process* p = new Process(user);
	Sched->SetInitialProcess(p);
	// Create our first working thread that will load the rest of the operating system
	user->processes->AddLast(p);
	Sched->CreateThread((unsigned char*) "Idle Thread", (void*) IdleThread,
			NULL);
	Sched->CreateThread((unsigned char*) "Base System Initialisation",
			(void*) SystemStart, NULL);
}

void System::InstallDevice(Device device)
{
	System::io.InstallDriver(device);
}

void System::IncommingInterrupt(struct regs* r)
{
	unsigned char fd = (unsigned char) r->eax;
	unsigned int callNumber = (unsigned int) (r->eax >> 8);
	unsigned char* localDataPointer = (unsigned char*) r->ebx;
	size_t bufferSize = r->ecx;
	//if this call was for us, and not I/O take a
	if (fd == 0)
	{
		//printf("%i:%x:%i\n", callNumber, localDataPointer, bufferSize);
		switch ((SystemCalls) callNumber)
		{
		case LunOS::Calls::Yield:
			Sched->Yield();
			break;
		case LunOS::Calls::Sleep:
			Sched->Sleep(bufferSize);
			break;
		case LunOS::Calls::CreateProcess:
			break;
		case LunOS::Calls::CreateThread:
		{	// Intentional scope to let us have variables
			void* function = (void*) *((unsigned int*) (localDataPointer + 64));
			void* data = (void*) *((unsigned int*) (localDataPointer + 64
					+ sizeof(char*)));
			*(unsigned int*) localDataPointer = Sched->CreateThread(
					localDataPointer, function, data);
		}
			break;
		case LunOS::Calls::KillThread:
		{
			Sched->KillThisThread();
		}
			break;
		case LunOS::Calls::KillProcess:
			Sched->KillProcess(*(unsigned int*) localDataPointer);
			break;
		case LunOS::Calls::ExitProcess:
			break;
		case LunOS::Calls::ExitThread:
			break;
		case LunOS::Calls::GetThreadID:
			break;
		case LunOS::Calls::GetProcessID:
			break;
		case LunOS::Calls::GetUserID:

			if (localDataPointer)
			{
				*((unsigned int*) localDataPointer) = User::GetCurrentUserID();
			}
			break;
		case LunOS::Calls::GetUsers:
			if (localDataPointer)
			{
				User::LoadUserIDs((unsigned int*) localDataPointer);
			}
			break;
		case LunOS::Calls::GetUserName:
			if (localDataPointer)
			{
				User::GetUserName(*((unsigned int*) localDataPointer),
						(char*) localDataPointer, bufferSize);
			}
			break;
		case LunOS::Calls::LogOn:
			break;
		case LunOS::Calls::LogOff:
			break;
		case LunOS::Calls::SwitchProcessToUser:
			if (localDataPointer)
			{
				*((bool*) localDataPointer) =
						Sched->GetActiveThread()->GetProcess()->SwitchToUser(
								*(unsigned int*) localDataPointer,
								(const char*) (localDataPointer + 4));
			}
			break;
		case LunOS::Calls::CreateUser:
			if (localDataPointer)
			{
				int name = strlen((const char*) localDataPointer) + 1;
				int pass = strlen((const char*) (localDataPointer + name)) + 1;
				//printf("\n%2%i:%s %i:%s %i:%s\n",name,(const char*)localDataPointer,pass,(const char*)(localDataPointer + name),strlen((const char*)(localDataPointer + name + pass)),(const char*)(localDataPointer + name + pass) );
				*((unsigned int*) localDataPointer) = User::CreateNewUser(
						(const char*) localDataPointer,
						(const char*) (localDataPointer + name),
						(const char*) (localDataPointer + name + pass));
			}
			break;
		case LunOS::Calls::GetNumberOfDevices:
			*((unsigned int*) localDataPointer) = io.GetNumberOfDevices();
			break;
		case LunOS::Calls::GetDeviceInfo:
			io.StoreDeviceInfo(*(unsigned int*) localDataPointer,
					(LunOS::IO::DeviceInfo*) localDataPointer);
			break;
		default:

			break;
		}
	}
	else
	{
		//call I/O at this point if it isn't a system's running state call
		io.Route(fd, callNumber, localDataPointer, bufferSize);
	}
}

