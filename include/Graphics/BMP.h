#ifndef BMP_H
#define BMP_H

#include <io/FileSystem.h>

namespace LunOS
{
namespace Graphics
{
	typedef struct Colour
	{
		unsigned char A;
		unsigned char R;
		unsigned char G;
		unsigned char B;
	} Colour;

	class Bitmap
	{
	public:
		static Bitmap* LoadBitmap(LunOS::IO::FileSystem* fileSystem, LunOS::IO::File* bitmapFile);
		static Bitmap* LoadBitmap(const char* fileName, LunOS::IO::FileSystem* fileSystem);
		unsigned int GetWidth() const;
		unsigned int GetHeight() const;
		// Gets if this images uses transparency or not
		bool HasTransparency();
		bool IsOpen();
		void Close();
		void SetTransparentColor(Colour colour);
		Colour GetTransparentColor();
		unsigned char* Data;
	private:
		Bitmap(unsigned int Width, unsigned int Height);
		~Bitmap();
		unsigned int Width;
		unsigned int Height;
		bool Transparent;
		Colour TransparentColor;
		bool Open;
	};
}
}

#endif
