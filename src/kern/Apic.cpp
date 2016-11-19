#include <kern/Apic.h>
using namespace LunOS;
namespace LunOS
{
	unsigned int Apic::Location;
	unsigned int Apic::Version;
	unsigned int Apic::LVTEntries;
}

void Apic::Init()
{
	Apic::Location = 0xFEE00000;
	if(Memory::linkPage((unsigned int*)Apic::Location, (unsigned int*)Apic::Location))
	{
		Memory::MarkUncacheable((unsigned int*)Apic::Location);
	}
	else
	{

	}
	Apic::Version = (*(unsigned int*)(Location + 0x30) & 0xFF);
	Apic::LVTEntries = (*(unsigned int*)(Location + 0x30) & 0xFF0000);
}

void Apic::PrintInformation()
{
	printf("%2Apic Version => 0x%x\n%4LVT Entries  => %i\nCPU          => %i\n", Apic::Version, Apic::LVTEntries, Apic::GetCPUID());
	printf("%3Other Core initialisation will occur here in the future.\n");
}

unsigned int Apic::GetCPUID()
{
	return (*(unsigned int*)(Apic::Location + 0x20) & 0xFF000000)>>24;
}
