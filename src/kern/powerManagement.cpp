#include <kern/system.hpp>

//Global declaration for power management functionality
PowerManagement powerManagement;

bool CheckForRSDPDescriptor(RSDPDescriptor* desc)
{
	if(strncmp((const char*)desc->signature,"RSD PTR ", 8) == 0)
	{
		if(CheckSum((unsigned char*)desc, sizeof(RSDPDescriptor)))
		{
			return true;
		}
	}
	return false;
}

unsigned int* GetExtendedBIOS()
{
	return (unsigned int*)
	((*(unsigned short*)0x40e) * 16);
}

RSDPDescriptor* FindRSDPDescriptor()
{
	unsigned int i;
	unsigned short* extendedBIOS = (unsigned short*)GetExtendedBIOS();
	//printf("The extended BIOS is located at %x\n",(int)extendedBIOS);
	unsigned int extendedBIOSLength = *extendedBIOS * 0x1000;
	for(i = (int)*extendedBIOS; i < *extendedBIOS + extendedBIOSLength; i+= 16)
	{
		if(CheckForRSDPDescriptor((RSDPDescriptor*)i))
			return (RSDPDescriptor*)i;
	}
	// Scan the BIOS memory segments for our signature
	for(i = 0x000E0000; i < 0x000FFFFF; i += 16)
	{
		if(CheckForRSDPDescriptor((RSDPDescriptor*)i))
			return (RSDPDescriptor*)i;
	}
	return NULL;
}

void PowerManagement::Init(){
	//eventually we will init everything here
	RSDPDescriptor* desc;

	// if we didn't find anything just exit now
	if((desc = FindRSDPDescriptor()) == NULL)
	{
		printf("We %3didn't %1find any %2Power Management Support%1!\n");
		this->ACPISupport = false;
		this->ACPIVersion = 0;
		this->RSDT = NULL;
		return;
	}
	// store that we have succeeded in detecting ACPI
	this->ACPISupport = true;
	this->ACPIVersion = desc->revision;
	// If we did now activate it
	desc->signature[7] = 0;
	printf("ACPI Version %2%i FOUND\n",desc->revision);
	this->RSDT = (RSDTHeader*)Memory::walkTree(1);
	Memory::linkPage((unsigned int*)desc->rsdtAddress,(unsigned int*)this->RSDT);
}

void PowerManagement::Sleep(){
	//put the computer into a sleep state here
}

void PowerManagement::Shutdown(){
	//put the computer into a sleep state here
}

void PowerManagement::Reboot(){
	//this will reboot the machine
	outportb(0x64,0xFE);
	for(;;)
	{
		 __asm__ __volatile__ ("hlt");
	}
}


//Multiple APIC Description Table. See section 5.2.8.
bool PowerManagement::isAPIC(SDTHeader* header)
{
	return (strncmp(header->signature,"APIC",4) == 0);
}
//Differentiated System Description Table. See section 5.2.7.1
bool isDSDT(SDTHeader* header)
{
	return (strncmp(header->signature,"DSDT",4) == 0);
}
//Fixed ACPI Description Table. See section 5.2.5.
bool isFACP(SDTHeader* header)
{
	return (strncmp(header->signature,"FACP",4) == 0);
}
//Firmware ACPI Control Structure. See section 5.2.6
bool isFACS(SDTHeader* header)
{
	return (strncmp(header->signature,"FACS",4) == 0);
}

//Persistent System Description Table. See section 5.2.7.3.
bool isPSDT(SDTHeader* header)
{
	return (strncmp(header->signature,"PSDT",4) == 0);
}
//Root System Description Table. See section 5.2.4.
bool isRSDT(SDTHeader* header)
{
	return (strncmp(header->signature,"RSDT",4) == 0);
}
//Secondary System Description Table. See section 5.2.7.2
bool isSSDT(SDTHeader* header)
{
	return (strncmp(header->signature,"SSDT",4) == 0);
}
//Smart Battery Specification Table. See section 5.2.9
bool isSBST(SDTHeader* header)
{
	return (strncmp(header->signature,"SBST",4) == 0);
}


