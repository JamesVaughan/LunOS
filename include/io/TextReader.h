#ifndef IO_TEXTREADER_H
#define IO_TEXTREADER_H
#include <io/FileSystem.h>

namespace LunOS
{
	namespace IO
	{
		class TextReader
		{
		public:
			// Create a text reader for the given file
			static TextReader* LoadTextReader(LunOS::IO::File* file, LunOS::IO::FileSystem* fileSystem);
			// Gives a pointer to the loaded line (do not delete this)
			unsigned char* LoadLine();
			// Load a line into the buffer (true if completed)
			bool LoadLine(unsigned char* buffer, unsigned int size, unsigned int* ammountRead);
			// Release held resources
			void Close();
		private:
			TextReader(LunOS::IO::File* file, LunOS::IO::FileSystem* fileSystem);
			unsigned char* Buffer;
			unsigned int BufferSize;
			unsigned int CurrentPosition;
			unsigned int BufferFilledIndex;
			LunOS::IO::File* File;
			LunOS::IO::FileSystem* FileSystem;
		};
	}
}

#endif
