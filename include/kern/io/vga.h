/*
 * vga.h
 *
 *  Created on: 19-Jun-2009
 *      Author: james
 */

#ifndef VGA_H_
#define VGA_H_
#include <io/Graphics.h>
#include <Graphics/BMP.h>
#include <Graphics/Font.h>

namespace LunOS
{
namespace Graphics
{
// The VGA driver for LunOS
class Vga
{
public:
	// change the current mode
	static void ChangeResoltion(VGAModes::Modes mode);
	// Returns the current mode
	static VGAModes::Modes GetResolution();
	// Draw a pixel to the screen
	static void DrawPixel(unsigned char r, unsigned char g, unsigned char b, unsigned int x, unsigned int y);
	// Draw a rectangle to the screen
	static void DrawRectangle(unsigned char r, unsigned char g, unsigned char b, int x, int y, unsigned int width, unsigned int height);
	// Draw a bitmap to the screen
	static void DrawBitmap(Bitmap* image, unsigned int x, unsigned int y, unsigned int width, unsigned int height);
	// Draw a string to the screen
	static void DrawString(Font* font, unsigned char* string, unsigned int x, unsigned int y, unsigned char r, unsigned char g, unsigned char b);
	// Set the image that will be used for the mouse cursor
	static void SetMouse(Bitmap* image);
    // Draw a line to the screen
	static void DrawLine(int x1, int y1, int x2, int y2, unsigned char r, unsigned char g, unsigned char b);
	// Setup the VGA to get read to be used
	static void Init();
private:
	static void FindAndLoadVRamStart();
};

}
}

#endif /* VGA_H_ */
