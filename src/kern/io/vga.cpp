/*
 * vga.cpp
 *
 *  Created on: 19-Jun-2009
 *      Author: james
 */
#include <kern/io/vga.h>
#include <kern/io/io.h>
#include <synchronization.h>
#include <SystemCalls.h>
#include <io/Mouse.h>
#include <kern/io/mouse.h>
#include <kern/io/pci.h>

//#define DEBUGVGA


using namespace LunOS::Graphics;

Lock* vgaLock;
VGAModes::Modes CurrentVGAMode;
unsigned int VGAWidth = 0;
unsigned int VGAHeight = 0;
unsigned int VGABPP = 8;
unsigned char *VGAMemoryBase = (unsigned char*)NULL;
Bitmap* CursorImage;

unsigned char* BackBuffer = NULL;


inline bool CheckBounds(unsigned int x, unsigned int y)
{
	if((x >= VGAWidth) | (y>=VGAHeight)) return false;
	return true;
}

inline void GetXY(int *buffer, int *x, int *y)
{
	*x = buffer[0];
	*y = buffer[1];
}

inline void GetWidthHeight(unsigned int *buffer, unsigned int *width, unsigned int *height)
{
	*width  = buffer[0];
	*height = buffer[1];
}

inline void RenderToScreen()
{
	int screenSize = VGAWidth * VGAHeight;
	int i = 0;
	// now copy this to screen
	for(i = 0; i < screenSize; i++)
	{
		((unsigned int*)VGAMemoryBase)[i] = ((unsigned int*)BackBuffer)[i];
	}
}

inline void RenderPixelToScreen(unsigned char r, unsigned char g, unsigned char b, unsigned int x, unsigned int y)
{
	unsigned int width = VGAWidth;
	BackBuffer[(x + (width * y)) * 4 + 0] = r;
	BackBuffer[(x + (width * y)) * 4 + 1] = g;
	BackBuffer[(x + (width * y)) * 4 + 2] = b;
}



bool Write(FDTEntry* fdt, unsigned char* buffer, size_t length)
{
	unsigned char commandType;
	vgaLock->GetLock();

	while((commandType = *(buffer++)))
	{
		switch(commandType)
		{
		// Change Graphics Mode
		case 1:
#ifndef DEBUGVGA
			Vga::ChangeResoltion((LunOS::Graphics::VGAModes::Modes)*(buffer++));
#endif
			break;
		// Render Pixel
		case 2:
#ifndef DEBUGVGA
			RenderPixelToScreen(buffer[8],buffer[9],buffer[10],((int*)buffer)[0],((int*)buffer)[1]);
#endif
			buffer += 11;
			break;
		// Render Rect
		case 3:
#ifndef DEBUGVGA
			Vga::DrawRectangle(buffer[16],buffer[17],buffer[18],
					((int*)buffer)[0],((int*)buffer)[1],((int*)buffer)[2],((int*)buffer)[3]);
#endif
			buffer += 19;
			break;
		// Render Bitmap
		case 4:
		{
			unsigned int* buf = (unsigned int*)buffer;
#ifndef DEBUGVGA
			Vga::DrawBitmap((Bitmap*)buf[0],buf[1],buf[2],buf[3],buf[4]);
#else

#endif
			buffer += 20;
		}
			break;
		// Render Font
		case 5:
		{
			unsigned int* buf = (unsigned int*)buffer;
#ifndef DEBUGVGA
			// Font* == [0->3], string* == [4->7], X == [8->11], Y == [12->15]
			Vga::DrawString((Font*)buf[0], (unsigned char*)buf[1], buf[2], buf[3], buffer[16], buffer[17], buffer[18]);
#else
			printf("DrawString:r=%i,g=%i,b=%i\n", buffer[16], buffer[17], buffer[18]);
#endif
			buffer += 19;
		}
			break;
		// Set mouse to the given image
		case 6:
		{
			Vga::SetMouse((Bitmap*)buffer);
			buffer += sizeof(Bitmap*);
		}
			break;
			// RenderLine
        case 7:
        {
            unsigned int* buf = (unsigned int*)buffer;
            #ifndef DEBUGVGA
            Vga::DrawLine(buf[0],buf[1],buf[2],buf[3], buffer[16],buffer[17],buffer[18]);
            #else
            printf("Draw Line: (%i,%i -> %i, %i)(%i:%i:%i)\n", *(unsigned int*)buffer,
                   *((unsigned int*) buffer + 4), *((unsigned int*)buffer + 8),
                   *((unsigned int*)buffer + 12), buffer[16],buffer[17], buffer[18]);
            #endif
            buffer += 19;
            break;
        }
		case 255:
			vgaLock->Release();
			return true;
		}
	}
#ifndef DEBUGVGA
	RenderToScreen();
#endif
	vgaLock->Release();
	return true;
}

void Vga::DrawLine(int x1, int y1, int x2, int y2, unsigned char r, unsigned char g, unsigned char b)
{
    int deltaX = x2 - x1;
    int deltaY = y2 - y1;
    int i,j;
    if(deltaX >= deltaY)
    {
        int ratio = deltaX / deltaY;
        int x = x1;
        j = y1;
        for(;j <= y2; j++)
        {
            for(i = 0;i < ratio || i + x < x2; i ++)
            {
                RenderPixelToScreen(r,g,b,x + i, j);
            }
            x+= ratio;
        }
    }
    else
    {
        //int ratio = deltaY / deltaX;
    }
}

namespace VGASPACE
{
bool Read(FDTEntry* fdt, unsigned char* buffer, size_t length)
{
	// no reading from the VGA yet
	return false;
}

bool Open(FDTEntry* fdt, unsigned char* buffer, size_t length)
{
	// We can trivially open a link to the vga
	return true;
}

bool Init(struct Device* us, unsigned char* buffer, size_t length)
{
	// We automatically work in regards to initialisation
	return true;
}

bool Close(FDTEntry* entry)
{
	// nothing to do to close a stream to us
	return true;
}
}
using namespace VGASPACE;
void Vga::Init()
{
	// set us up for success
	vgaLock = new Lock();
	CursorImage = NULL;
	// Now install a VGA driver
	Device us;
	us.AWrite = Write;
	us.Write = Write;
	us.Read = Read;
	us.ARead = Read;
	us.Shutdown = NULL;
	us.DeviceThread = NULL;
	us.Close = Close;
	us.Open = Open;
	us.type = LunOS::IO::DeviceTypes::DEVICE_GFX;
	CurrentVGAMode = LunOS::Graphics::VGAModes::TextMode;
	VGAWidth = 80;
	VGAHeight = 25;
::System::InstallDevice(us);
}

inline bool ValidMode(VGAModes::Modes modeToTest)
{
	// we will actually test this later
	return true;
}

void SwitchToGraphicsMode()
{
	unsigned int i,j;
	VGAWidth = 800;
	VGAHeight = 600;
	VGABPP = 32;
	unsigned int screenSize = VGAHeight * VGAWidth * (VGABPP / 8);

	if(BackBuffer != NULL)
	{
		delete[] BackBuffer;
	}
	BackBuffer = new unsigned char[screenSize];
	for(i = 0; i < screenSize; i += 0x1000)
	{
		if(!Memory::linkPage((unsigned int*)(VGAMemoryBase + i),(unsigned int*)(VGAMemoryBase + i)))
		{
			printf("%3ERROR LINKING PAGE FOR VESA!\n");
			LunOS::System::Sleep(1000);
			return;
		}
		Memory::RemoveFromFreeStack((unsigned int*)(VGAMemoryBase + i));
	}
	// Clear the back buffer
	for(i = 0; i < VGAWidth; i++)
	{
		for(j = 0; j < VGAHeight; j++)
		{
			RenderPixelToScreen((unsigned char)i,(unsigned char)j,0,i,j);
		}
	}

	// doing this for Bochs for now
	outportw(0x1ce, 4);
	outportw(0x1cf, 0); // enable changing modes

	outportw(0x1ce, 1);
	outportw(0x1cf, VGAWidth); // set the X

	outportw(0x1ce, 2);
	outportw(0x1cf, VGAHeight); // set the Y

	outportw(0x1ce, 3);
	outportw(0x1cf, 0x20); // 32bpp

	outportw(0x1ce, 4);
	outportw(0x1cf, 0x41); // enable this sexy mode | linear frame buffer
}

void Vga::FindAndLoadVRamStart()
{
	using namespace LunOS;
	using namespace LunOS::IO;
	for(int bus = 0; bus < 255; bus++)
	{
		if(!PCI::ActivePCIBus[bus]) continue;
		for(int device = 0; device < 32; device++)
		{
			for(int function = 0; function < 4; function++)
			{
				if(PCI::pciData[bus][device][function].ClassCode == 3)
				{
					if(PCI::pciData[bus][device][function].VendorNumber == 0x15AD)
					{
						VGAMemoryBase = (unsigned char*)PCI::GetMemoryAddress(PCI::pciData[bus][device][function].BAR[1]);
					}
					else
					{
						VGAMemoryBase = (unsigned char*)PCI::GetMemoryAddress(PCI::pciData[bus][device][function].BAR[0]);
					}
				}
			}
		}
	}
}

void Vga::ChangeResoltion(VGAModes::Modes mode)
{
	if(!ValidMode(mode)) return;
	// if we are already in this mode just stay here
	if(CurrentVGAMode == mode)
	{
		vgaLock->Release();
		return;
	}
	Vga::FindAndLoadVRamStart();
	switch(mode)
	{
	case VGAModes::TextMode:
		outportb(0x3C0, 0x10);
		outportb(0x3C0, 0x0C);
		break;
	case VGAModes::GraphicsMode:
		SwitchToGraphicsMode();
		break;
	}
	CurrentVGAMode = mode;
}

VGAModes::Modes Vga::GetResolution()
{
	return CurrentVGAMode;
}

void Vga::DrawPixel(unsigned char r, unsigned char g, unsigned char b, unsigned int x, unsigned int y)
{
	RenderPixelToScreen(r,g,b,x,y);
}

void Vga::DrawRectangle(unsigned char r, unsigned char g, unsigned char b, int x, int y, unsigned int width, unsigned int height)
{
	int i,j;
	unsigned int w = width,h = height;
	if(x < 0)
	{
		w += x;
		x = 0;
	}
	if(y < 0)
	{
		h += y;
		y = 0;
	}
	// Make sure we are only rendering in our given area
	if(x + w > VGAWidth) w = VGAWidth - x;
	if(y + h > VGAHeight) h = VGAHeight - y;
	// looping backwards takes less opcodes [faster]
	// Also, if width < 0 / height < 0 it will not render
	for(i = w - 1; i >= 0; i--)
	{
		for(j = h - 1; j >= 0; j--)
		{
			RenderPixelToScreen(r,g,b,x+i,y+j);
		}
	}
}

void Vga::DrawBitmap(Bitmap* img, unsigned int x, unsigned int y, unsigned int width, unsigned int height)
{
	unsigned int i,j;
	unsigned int w = width,h = height;
	unsigned int imageWidth = img->GetWidth();
	if(x >= VGAWidth) return;
	if(y >= VGAHeight) return;
	// Make sure we are only rendering in our given area
	if(x + w > VGAWidth) w = VGAWidth - x;
	if(y + h > VGAHeight) h = VGAHeight - y;
	// looping backwards takes less opcodes [faster]
	// Also, if width < 0 / height < 0 it will not render
	unsigned char* dataPos = img->Data;
	if(img->HasTransparency())
	{
		Colour colour = img->GetTransparentColor();
		for(j = 0; j < h; j++)
		{
			for(i = 0; i < w; i++)
			{
				unsigned char r = dataPos[0],g = dataPos[1],b = dataPos[2];
				if(colour.R != r || colour.G != g || colour.B != b)
				{
					RenderPixelToScreen(r,g,b,x+i,y+j);
				}
				dataPos += 4;
			}
			// Skip the rest of the image that we are not able to see
			dataPos += (imageWidth - w) * 4;
		}
	}
	else
	{
		for(j = 0; j < h; j++)
		{
			for(i = 0; i < w; i++)
			{
	#ifndef DEBUGVGA
				RenderPixelToScreen(*(dataPos),*(dataPos + 1),*(dataPos +2),x+i,y+j);
	#else
				//printf("R:%i,G:%i,B:%i\n",*(dataPos + 2),*(dataPos + 1),*(dataPos));
	#endif
				dataPos += 4;
			}
			// Skip the rest of the image that we are not able to see
			dataPos += (imageWidth - w) * 4;
		}
	}
}

void Vga::DrawString(Font* font, unsigned char* string, unsigned int x, unsigned int y, unsigned char r, unsigned char g, unsigned char b)
{
	unsigned int i,j,k;
	for(i = 0; string[i]; i++)
	{
		unsigned int characterWidth = font->GetWidth(string[i]);
		unsigned int characterHeight = font->GetHeight(string[i]);
		for(k = 0; k < characterHeight; k++)
		{
			if(y + k >= VGAHeight) break;
			for(j = 0; j < characterWidth; j++)
			{
				if(x + j >= VGAWidth) break;
				if(font->GetPlace(string[i], j, k))
				{
					RenderPixelToScreen(r,g,b,x + j, y + k);
				}
			}
		}
		x += characterWidth;
	}
}

void Vga::SetMouse(Bitmap* image)
{
	CursorImage = image;
}


