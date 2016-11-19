#ifndef KERN_APIC_H
#define KERN_APIC_H
namespace LunOS
{
 class Apic;
}

#include <kern/system.hpp>

namespace LunOS
{
	class Apic
	{
		private:
		static unsigned int Location;
		static unsigned int Version;
		static unsigned int LVTEntries;
		public:
		static void Init();
		static unsigned int GetCPUID();
		static void PrintInformation();
	};
}
#endif
