#include <Graphics/Font.h>
#include <io/TextReader.h>
#include <String.h>
#include <kern/core.h>
#ifndef NULL
	#define NULL 0
#endif

using namespace LunOS::IO;
using namespace LunOS::Graphics;

Font* Font::LoadFont(const char* fontFileName, FileSystem* fs)
{
	File* f = fs->LoadFile(fontFileName, 0);
	if(f)
	{
		Font* font = Font::LoadFont(f, fs);
		fs->CloseFile(f);
		return font;
	}
	return NULL;
}

Font* Font::LoadFont(File* fontFile, FileSystem* fileSystem)
{
	unsigned int i,j;
	TextReader* reader = TextReader::LoadTextReader(fontFile, fileSystem);
	unsigned char buff[128];
	if(reader)
	{
		Font* font = new Font();
		String s;
		unsigned int ammountRead;
		if(!reader->LoadLine(buff,128,&ammountRead))
		{
			reader->Close();
			delete font;
			return NULL;
		}
		s = buff;
		s.toInt((unsigned int*)&font->CharacterWidth);
		if(!reader->LoadLine(buff,128,&ammountRead))
		{
			reader->Close();
			delete font;
			return NULL;
		}
		s = buff;
		s.toInt((unsigned int*)&font->CharacterHeight);
		if(font->CharacterHeight == 0 || font->CharacterWidth == 0)
		{
			reader->Close();
			delete font;
			return NULL;
		}
		font->Data = new bool[font->CharacterWidth * font->CharacterHeight * 128];
		for(i = 0; i < font->CharacterWidth * font->CharacterHeight * 128;i++)
		{
			font->Data[i] = false;
		}
		// Keep reading until the end of file
		while(reader->LoadLine(buff,128,&ammountRead) || ammountRead != 0)
		{
			unsigned int characterNumber;
			// Allow for newlines between characters
			if(ammountRead == 0) continue;
			//Get the character number
			characterNumber = (unsigned int)buff[0];
			unsigned int dataStart = font->CharacterWidth * font->CharacterHeight * characterNumber;
			for(i = 0; i < font->CharacterHeight; i++)
			{
				reader->LoadLine(buff, 128, &ammountRead);
				for(j = 0; j < font->CharacterWidth; j++)
				{
					font->Data[dataStart + font->CharacterWidth * i + j] = (buff[j] != 'X');
				}
			}
		}
		reader->Close();
		return font;
	}
	return NULL;
}

unsigned int Font::GetWidth(unsigned char character) const
{
	// For now I am only supporting MonoSpace Fonts
	return this->CharacterWidth;
}

unsigned int Font::GetWidth(unsigned char* string) const
{
	// For now I am only supporting MonoSpace Fonts
	return this->CharacterWidth * String::GetLength(string);
}

unsigned int Font::GetHeight(unsigned char character) const
{
	// For now I am only supporting MonoSpace Fonts
	return this->CharacterHeight;
}

unsigned int Font::GetHeight(unsigned char* string) const
{
	// For now I am only supporting MonoSpace Fonts
	return this->CharacterHeight;
}

bool Font::GetPlace(unsigned char c, unsigned int x, unsigned int y) const
{
	return this->Data[c * this->CharacterWidth *
	                  this->CharacterHeight + y * this->CharacterWidth
	                  + x];
}

void Font::Close()
{
	if(this)
	{
		if(this->Data)
		{
			delete this->Data;
		}
		delete this;
	}
}
