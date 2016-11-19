#include <Graphics/BMP.h>
#include <kern/console.h>
#include <kern/system.hpp>
#include <SysCall.h>

using namespace LunOS::IO;
using namespace LunOS::Graphics;

struct BitmapHeaderBasicInfo
{
	// Basic information
	unsigned char Type[2];
	unsigned int Length;
	unsigned char Reserved1[2];
	unsigned char Reserved2[2];
	unsigned int DataStart;
	// Secondary information, only SizeOfSecondaryInformation bytes are available
	unsigned int SizeOfSecondaryInformation;
}__attribute__((packed));

struct BitmapHeader
{
	BitmapHeaderBasicInfo Basic;
	unsigned int Width;
	unsigned int Height;
	unsigned short Planes;
	unsigned short BitsPerPixel;
	unsigned int CompressionMethod;
	unsigned int CompressedImageSize;
	unsigned int PixelsPerMeterX;
	unsigned int PixelsPerMeterY;
	unsigned int NumberOfColours;
	unsigned int NumberOfImportantColours;
}__attribute__((packed));

Bitmap::Bitmap(unsigned int width, unsigned int height)
{
	this->Open = true;
	this->Width = width;
	this->Height = height;
	this->Data = new unsigned char[height * width * 4];
}

Bitmap* Bitmap::LoadBitmap(const char* fileName, FileSystem* fs)
{
	File* f = fs->LoadFile(fileName, 0);
	if(f)
	{
		return Bitmap::LoadBitmap(fs, f);
	}
	printf("Tried to load %s however we could not find the file!\n", fileName);
	return NULL;
}

Bitmap* Bitmap::LoadBitmap(FileSystem* fs, File* file)
{
	BitmapHeader header;
	memset(&header,0,sizeof(BitmapHeader));
	fs->ReadFile(file,(unsigned char*)&header, sizeof(BitmapHeaderBasicInfo));
	if(header.Basic.SizeOfSecondaryInformation > 0x1000)
	{
		printf("The header's size was greater than 4kb unrecognized format!\n");
		return NULL;
	}
	fs->ReadFile(file,((unsigned char*)&header) + sizeof(BitmapHeaderBasicInfo), header.Basic.SizeOfSecondaryInformation - 4);
	bool upsideDown = true;
	if((int)header.Height < 0)
	{
		header.Height = -(int)header.Height;
		upsideDown = false;
	}
	Bitmap* b = new Bitmap(header.Width, header.Height);
	b->Transparent = false;
	unsigned int extraX = header.Width % 4;
	switch(header.BitsPerPixel)
	{
	// smaller than our native resolution
	case 24:
	{
		if(extraX == 0)
		{
			fs->ReadFile(file, b->Data, b->Width * b->Height * 3);
			// now that we have all of the data read in, lets fix the file
			unsigned char* toptr = b->Data + (b->Width * b->Height) * 4 - 4;
			unsigned char* fromptr = b->Data + (b->Width * b->Height) * 3 - 3;
			for(;fromptr > b->Data; toptr -= 4, fromptr -= 3)
			{
				toptr[0] = fromptr[0];
				toptr[1] = fromptr[1];
				toptr[2] = fromptr[2];
				toptr[3] = 0;
			}
		}
	}
		break;
	// Our native resolution
	case 32:
	{
		if(extraX == 0)
		{
			fs->ReadFile(file, b->Data, b->Width * b->Height * 4);
		}
	}
		break;
	default:
		{
			delete b;
			return NULL;
		}
	}
	if(upsideDown)
	{
		unsigned int lineLength = b->Width * 4;
		unsigned char *temp = new unsigned char[lineLength];
		unsigned int row;
		unsigned int halfrows = b->Height / 2;
		for(row = 0; row < halfrows; row++)
		{
			unsigned char* lowRow = b->Data + (row * lineLength);
			unsigned char* highRow = b->Data + (b->Height - 1 - row) * lineLength;
			memcpy(temp, lowRow, lineLength);
			memcpy(lowRow, highRow, lineLength);
			memcpy(highRow, temp, lineLength);
		}
		delete temp;
	}
	return b;
}

bool Bitmap::IsOpen()
{
	return this->Open;
}

void Bitmap::Close()
{
	if(this && this->IsOpen())
	{
		delete this;
	}
}

unsigned int Bitmap::GetWidth() const
{
	return this->Width;
}

unsigned int Bitmap::GetHeight() const
{
	return this->Height;
}

bool Bitmap::HasTransparency()
{
	return this->Transparent;
}

void Bitmap::SetTransparentColor(Colour colour)
{
	this->TransparentColor = colour;
	this->Transparent = true;
}

Colour Bitmap::GetTransparentColor()
{
	return this->TransparentColor;
}

Bitmap::~Bitmap()
{
	if(this && this->Data)
	{
		delete this->Data;
		this->Data = 0;
		this->Width = this->Height = 0;
	}
}
