#ifndef IO_FONT_H
#define IO_FONT_H
#include <io/FileSystem.h>

namespace LunOS
{
	namespace Graphics
	{
		class Font
		{
		public:
			// Load a font from file
			static Font* LoadFont(LunOS::IO::File* fontFile, LunOS::IO::FileSystem* fileSystem);
			// Load a font from file
			static Font* LoadFont(const char* fontFileName, LunOS::IO::FileSystem* fileSystem);
			// Get the width of a character
			unsigned int GetWidth(unsigned char character) const;
			// Get the width of a string
			unsigned int GetWidth(unsigned char* string) const;
			// Get the height of a character
			unsigned int GetHeight(unsigned char character) const;
			// Get the height of a string
			unsigned int GetHeight(unsigned char* string) const;
			// Release all of the resources held by this Font and will delete itself
			void Close();
			// Get if a place for a character should be set
			bool GetPlace(unsigned char c, unsigned int x, unsigned int y) const;
		private:
			bool* Data;
			unsigned int CharacterWidth;
			unsigned int CharacterHeight;
		};
	}
}

#endif
