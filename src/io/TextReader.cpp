#include <io/TextReader.h>
#include <kern/console.h>
#ifndef NULL
#define NULL 0
#endif

using namespace LunOS::IO;

TextReader* TextReader::LoadTextReader(LunOS::IO::File* file, LunOS::IO::FileSystem* fileSystem)
{
	if(file && fileSystem)
	{
		return new TextReader(file,fileSystem);
	}
	return NULL;
}

TextReader::TextReader(LunOS::IO::File *file, LunOS::IO::FileSystem *fileSystem)
{
	this->Buffer = new unsigned char[0x1000];
	this->BufferSize = 0x1000;
	this->File = file;
	this->FileSystem = fileSystem;
	this->CurrentPosition = 0;
	this->BufferFilledIndex = 0;
}

void TextReader::Close()
{
	if(this)
	{
		if(this->Buffer)
		{
			delete this->Buffer;
		}
		delete this;
	}
}

unsigned char* TextReader::LoadLine(void)
{
	// Return a pointer to the parsed line (this is ackward)
	return NULL;
}

bool TextReader::LoadLine(unsigned char* buffer,unsigned int size, unsigned int* ammountRead)
{
	unsigned int dataRead = 0;
	bool finishedLine = false;
	// We always let there be room to add in a 0 to finish the string
	size = size - 1;
	while(dataRead < size)
	{
		// If we are here then we have loaded more than one line so start here
		while(this->CurrentPosition < this->BufferFilledIndex && dataRead < size)
		{

			switch(this->Buffer[this->CurrentPosition])
			{
			case 13:
				//Just do nothing on a carriage return
				this->CurrentPosition++;
				break;
			case 10:
			case 0:
				buffer[dataRead] = 0;
				this->CurrentPosition++;
				*ammountRead = dataRead;
				return true;
			default:
				buffer[dataRead++] = this->Buffer[this->CurrentPosition++];
				break;
			}
		}
		if(this->File->CurrentPosition < this->File->Length)
		{
			// We need to load more data into our buffer to finish processing
			this->CurrentPosition = 0;
			this->BufferFilledIndex = this->FileSystem->ReadFile(this->File, this->Buffer, this->BufferSize);
		}
		else
		{
			// If there is no data left just end now
			break;
		}
	}
	buffer[dataRead] = 0;
	// Return true if we finished loading in a line
	*ammountRead = dataRead;
	return finishedLine;
}
