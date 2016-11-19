/*
 * Graphics.h
 *
 *  Created on: 25-Jun-2009
 *      Author: james
 */

#ifndef GRAPHICS_H_
#define GRAPHICS_H_

#include <io/Stream.h>
#include <Graphics/BMP.h>
#include <Graphics/Font.h>

namespace LunOS
{
namespace Graphics
{
namespace VGAModes
	{
		enum Modes
		{
			TextMode,
			GraphicsMode
		};
	}
}
namespace IO
{

class GraphicsStream : public LunOS::IO::Stream
{
public:
	// Gets the stream for the graphics adapter
	static GraphicsStream* GetGraphicsConnection();
	// Draw a pixel to the screen
	void DrawPixel(unsigned char r, unsigned char g, unsigned char b, unsigned int x, unsigned int y);
	// Draw a rectangle to the screen
	void DrawRectangle(unsigned char r, unsigned char g, unsigned char b, unsigned int x, unsigned int y, unsigned int width, unsigned int height);
	// Draw a blitmap to the screen
	void DrawBitmap(LunOS::Graphics::Bitmap* bitmap, unsigned int x, unsigned int y, unsigned int width, unsigned int height);
	// Draw a string to the screen in the given font
	void DrawString(LunOS::Graphics::Font* font, unsigned char* string, unsigned char red, unsigned char green, unsigned char blue, unsigned int x, unsigned int y);
	// draw a line of the given colour
	void DrawLine(unsigned char red, unsigned char green, unsigned char blue, int x1, int y1, int x2, int y2);
	// Change the graphics settings to text mode
	void ChangeToTextMode();
	// Change the graphics settings to graphics
	void ChangeToGraphicsMode();
	// Set the cursor
	void SetCursorImage(LunOS::Graphics::Bitmap* bitmap);
	// Write the buffer to the screen
	void Flush();
private:
	GraphicsStream(unsigned int DeviceNumber, unsigned int bufferSize);
	~GraphicsStream();
};

}
}

#endif /* GRAPHICS_H_ */
