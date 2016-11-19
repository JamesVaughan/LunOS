#ifndef IO_MOUSE_H
#define IO_MOUSE_H
#include <io/Stream.h>
#include <io/Io.h>

namespace LunOS
{
	namespace IO
	{
		struct MouseStatus
		{
			bool Buttons[5];
			int X;
			int Y;
		};

		class Mouse : public Stream
		{
		public:
			static Mouse* GetMouseStream();
			MouseStatus GetStatus();
		private:
			Mouse(unsigned int fd, unsigned int bufferSize);
			~Mouse();
		};
	}
}

#endif
