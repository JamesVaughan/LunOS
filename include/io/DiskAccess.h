#ifndef IO_DISKACCESS_H
#define IO_DISKACCESS_H
#include <io/Stream.h>
#include <io/Io.h>

namespace LunOS
{
	namespace IO
	{
		struct DiskInfo
		{
			unsigned char Name [128];
			unsigned long long Sectors;
			bool Exists;
			bool ReadAccess;
			bool WriteAccess;
			bool LBA28;
			bool LBA48;
			bool Serial;
		};

		class DiskAccess : public LunOS::IO::Stream
		{
		public:
			static DiskAccess* GetDiskStream(unsigned int disk);
			DiskInfo GetDiskInformation();
			// Read from the connected disk
			void ReadDisk(unsigned char* buffer, unsigned int sectors, unsigned int LBA);
		private:
			unsigned int StreamedDisk;
			DiskAccess(unsigned int disk, unsigned int fd, unsigned int bufferSize);
			~DiskAccess();
		};
	}
}

#endif

