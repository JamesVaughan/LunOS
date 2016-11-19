/*
 * Graphics.cpp
 *
 *  Created on: 25-Jun-2009
 *      Author: james
 */



#include <io/Graphics.h>
#include <SysCall.h>

using namespace LunOS::IO;
using namespace LunOS::Graphics;

#ifndef NULL
#define NULL 0
#endif

#define ChangeGraphicsMode 1
#define RenderPixel 2
#define RenderRectagle 3
#define RenderBitmap 4
#define RenderText 5
#define SetCursor 6
#define RenderLine 7

GraphicsStream* GraphicsStream::GetGraphicsConnection()
{
	unsigned int numberOfDevices = LunOS::System::GetNumberOfDevices();
	unsigned int i;
	for(i = 0; i < numberOfDevices; i++)
	{
		LunOS::IO::DeviceInfo info = LunOS::System::GetDeviceInfo(i);
		if(info.type == LunOS::IO::DeviceTypes::DEVICE_GFX)
		{
			return new GraphicsStream(i,0x500);
		}
	}
	return NULL;
}

GraphicsStream::GraphicsStream(unsigned int deviceNumber, unsigned int bufferSize) : Stream(deviceNumber,bufferSize)
{
}

GraphicsStream::~GraphicsStream()
{
	this->Close();
}

void GraphicsStream::DrawPixel(unsigned char r, unsigned char g, unsigned char b, unsigned int x, unsigned int y)
{
	this->streamLock.GetLock();
	if(this->position >= this->bufferLength - 13)
	{
		this->buffer[this->position++] = 255;
		this->buffer[this->position] = 0;
		this->Write();
		this->position = 0;
	}
	unsigned int* buf = (unsigned int*)(this->buffer + 1 + this->position);
	this->buffer[this->position + 0] = RenderPixel;
	buf[0] = x;
	buf[1] = y;
	this->buffer[this->position + 9] = r;
	this->buffer[this->position + 10] = g;
	this->buffer[this->position + 11] = b;
	this->buffer[this->position + 12] = 0;
	// we go 12 points up so the next instruction will overwrite that last zero
	this->position += 12;
	this->streamLock.Release();
}

void GraphicsStream::DrawRectangle(unsigned char r, unsigned char g, unsigned char b, unsigned int x, unsigned int y, unsigned int width, unsigned int height)
{

	this->streamLock.GetLock();
	if(this->bufferLength - 21 < this->position)
	{
		this->buffer[this->position++] = 255;
		this->buffer[this->position] = 0;
		this->Write();
	}
	unsigned int* buf = (unsigned int*)(this->buffer + 1 + this->position);
	this->buffer[this->position + 0] = RenderRectagle;
	buf[0] = x;
	buf[1] = y;
	buf[2] = width;
	buf[3] = height; // [1]->[16]
	this->buffer[this->position + 17] = r;
	this->buffer[this->position + 18] = g;
	this->buffer[this->position + 19] = b;
	this->buffer[this->position + 20] = 0;
	// we go 20 bytes up so we overwrite that last zero
	this->position += 20;
	this->streamLock.Release();
}

void GraphicsStream::DrawBitmap(Bitmap* bitmap, unsigned int x, unsigned int y, unsigned int width, unsigned int height)
{
	this->streamLock.GetLock();
	if(this->bufferLength - 22 < this->position)
	{
		this->buffer[this->position++] = 255;
		this->buffer[this->position] = 0;
		this->Write();
	}
	unsigned int* buf = (unsigned int*)(this->buffer + 1 + this->position);
	this->buffer[this->position + 0] = RenderBitmap;
	buf[0] = (unsigned int)bitmap;
	buf[1] = x;
	buf[2] = y;
	buf[3] = width;
	buf[4] = height; // [1]->[20]
	this->buffer[this->position + 21] = 0;
	// we go 20 bytes up so we overwrite that last zero
	this->position += 21;
	this->streamLock.Release();
}

void GraphicsStream::SetCursorImage(Bitmap *image)
{
	this->streamLock.GetLock();
	if(this->bufferLength - 6 < this->position)
	{
		this->buffer[this->position] = 255;
		this->Write();
	}
	this->buffer[this->position + 0] = SetCursor;
	*((Bitmap**)(this->buffer + this->position + 1)) = image;
	this->buffer[this->position + 5] = 0;
	this->position += 5;
	this->streamLock.Release();
}

void GraphicsStream::DrawString(Font* font, unsigned char* string, unsigned char r, unsigned char g, unsigned char b, unsigned int x, unsigned int y)
{
	this->streamLock.GetLock();
	if(this->bufferLength - 20 < this->position)
	{
		this->buffer[this->position++] = 255;
		this->buffer[this->position] = 0;
		this->Write();
	}
	unsigned int* buf = (unsigned int*)(this->buffer + 1 + this->position);
	// [0]
	this->buffer[this->position + 0] = RenderText;
	// [1->4]
	buf[0] = (unsigned int)font;
	// [5->8]
	buf[1] = (unsigned int)string;
	// [9->12]
	buf[2] = x;
	// [13->16]
	buf[3] = y;
	// [17 -> 19]
	this->buffer[this->position + 17] = r;
	this->buffer[this->position + 18] = g;
	this->buffer[this->position + 19] = b;
	this->buffer[this->position + 20] = 0;
	// we go up 19 to make sure to overwrite the last zero
	this->position += 20;
	this->streamLock.Release();
}

void GraphicsStream::DrawLine(unsigned char red, unsigned char green, unsigned char blue, int x1, int y1, int x2, int y2)
{
	this->streamLock.GetLock();
	if(this->bufferLength - 20 < this->position)
	{
		this->buffer[this->position++] = 255;
		this->buffer[this->position] = 0;
		this->Write();
	}
	//unsigned int* buf = (unsigned int*)(this->buffer + 1 + this->position);
	// [0]
	this->buffer[this->position + 0] = RenderLine;
	*(unsigned int*)(this->buffer + position + 1) = x1;
	*(unsigned int*)(this->buffer + position + 5) = y1;
	*(unsigned int*)(this->buffer + position + 9) = x2;
	*(unsigned int*)(this->buffer + position + 13) = y2;
	/*// [1->4]
	buf[0] = x1;
	// [5->8]
	buf[1] = y1;
	// [9->12]
	buf[2] = x2;
	// [13->16]
	buf[3] = y2;*/
	// [17 -> 19]
	this->buffer[this->position + 17] = red;
	this->buffer[this->position + 18] = green;
	this->buffer[this->position + 19] = blue;
	this->buffer[this->position + 20] = 0;
	// we go up 19 to make sure to overwrite the last zero
	this->position += 20;
	this->streamLock.Release();
}

void GraphicsStream::ChangeToGraphicsMode()
{
	this->streamLock.GetLock();
	if(this->bufferLength - 2 < this->position)
	{
		this->buffer[this->position] = 255;
		this->Write();
	}
	this->buffer[0] = ChangeGraphicsMode;
	this->buffer[1] = LunOS::Graphics::VGAModes::GraphicsMode;
	this->buffer[2] = 0;
	this->position += 2;
	this->streamLock.Release();
}

void GraphicsStream::ChangeToTextMode()
{
	this->streamLock.GetLock();
	if(this->bufferLength - 2 < this->position)
	{
		this->buffer[this->position] = 255;
		this->Write();
	}
	this->buffer[0] = ChangeGraphicsMode;
	this->buffer[1] = LunOS::Graphics::VGAModes::TextMode;
	this->buffer[2] = 0;
	this->position += 2;
	this->streamLock.Release();
}

void GraphicsStream::Flush()
{
	this->streamLock.GetLock();
	this->Write();
	this->buffer[0] = 0;
	this->streamLock.Release();
}
