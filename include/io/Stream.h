#ifndef _Stream_H
#define _Stream_H

#include <synchronization.h>

namespace LunOS
{
namespace IO
{
	class Stream
	{
		friend class System;
	public:
		virtual void Close();
	protected:
		Stream(unsigned int DeviceNumber, unsigned int bufferSize);
		Stream(Stream* baseStream);
		void Write();
		void Read();
		void Read(unsigned int ammount);
		virtual ~Stream();
		unsigned int fd;
		unsigned int position;
		unsigned char* buffer;
		unsigned int bufferLength;
		unsigned int length;
		Lock streamLock;
		bool IsOpen;
	private:
		unsigned int StreamLockData;
	};
}
}
#endif
