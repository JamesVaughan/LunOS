#ifndef KERN_MOUSE_H
#define KERN_MOUSE_H
#include <kern/system.hpp>

namespace LunOS
{
	namespace IO
	{
	namespace Kernel
	{
		class MouseDriver
		{
		public:
			static void Init();
			static bool Read(FDTEntry* fdt, unsigned char* buffer, size_t length);
			static bool Write(FDTEntry* fdt, unsigned char* buffer, size_t length);
			static bool Init(Device*, unsigned char*, size_t);
			static bool Open(FDTEntry* entry, unsigned char*, size_t);
			static bool Close(FDTEntry*);
			static void InterruptHandler(struct regs* r);
		private:
			static void Wait(bool data);
			static void Write(unsigned char data);
			static unsigned char Read();

			static unsigned char MessagePosition;
			static unsigned char Message[4];
			static unsigned int LastMessage;

			static bool Button[5];
			static int X;
			static int Y;
		};
	}
	}
}

#endif
