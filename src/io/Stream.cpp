/*
 * Stream.cpp
 *
 *  Created on: 25-Jun-2009
 *      Author: james
 */

#include <io/Stream.h>
#include <kern/console.h>
#include <SysCall.h>

using namespace LunOS::IO;

#define StreamRead 1
#define StreamAReal 2
#define StreamWrite 3
#define StreamAWrite 4
#define StreamClose 5
#define StreamShutdown 6 // We shouldn't really use this here

Stream::Stream(unsigned int deviceNumber, unsigned int bufferSize) : streamLock(&(this->StreamLockData))
{
	unsigned int buffer;
	unsigned int size = sizeof(unsigned int);
	System::Call(255, deviceNumber, (unsigned char*)&buffer, size); // this opens a stream to the new device
	this->Disposed = false;
	this->fd = buffer;
	this->IsOpen = (fd != 255);
	this->position = 0;
	this->length = 0;
	if (this->IsOpen)
	{
		unsigned int i, ints = bufferSize / sizeof(unsigned int), remainder = bufferSize % sizeof(unsigned int);

		this->buffer = new unsigned char[bufferSize];
		unsigned int *pos = (unsigned int*)this->buffer;
		this->bufferLength = bufferSize;
		// Zero out the buffer
		if (bufferLength > sizeof(unsigned int))
		{
			for (i = 0; i < ints; i++)
			{
				*(pos++) = 0;
			}
			for (i = 0; i < remainder; i++)
			{
				this->buffer[i - bufferSize - 1] = 0;
			}
		}
		else
		{
			for (i = 0; i < bufferSize; i++)
			{
				this->buffer[i] = 0;
			}
		}
	}
	else
	{
		this->buffer = NULL;
		this->bufferLength = 0;
	}
}

Stream::Stream(Stream* stream)
{
	this->StreamLockData = 0;
	stream->streamLock.GetLock();
	this->fd = stream->fd;
	this->buffer = stream->buffer;
	this->bufferLength = this->length = stream->length;
	this->position = 0; // we have a different position in the stream
	this->IsOpen = stream->IsOpen;
	this->streamLock.BindAddresses(&stream->streamLock);
	this->Disposed = false;
	stream->streamLock.Release();
}

Stream::~Stream()
{
	this->Disposed = true;
}

void Stream::Close()
{
	if (!this->Disposed)
	{
		this->streamLock.GetLock();
		if (this->IsOpen)
		{
			this->Write(); // make sure everything has been written
			this->IsOpen = false;
			System::Call(this->fd, StreamClose, NULL, NULL);
			this->bufferLength = 0;
			delete this->buffer;
			this->buffer = NULL;
		}
		this->Disposed = true;
		delete this;
	}
}

void Stream::Write()
{
	bool gotLock;
	// check to see if the lock is already had, everything calling this must have tried to have already gotten it
	if (this->position == 0) return; // if there is nothing to write just skip it
	gotLock = this->streamLock.TryLock();
	System::Call(this->fd, StreamWrite, (this->buffer), this->position);
	this->position = 0;
	if (gotLock) this->streamLock.Release();
}

void Stream::Read()
{
	bool gotLock = this->streamLock.TryLock();
	unsigned int ammount = this->length - this->position;
	System::Call(this->fd, StreamRead, (this->buffer + this->position), ammount);
	if (gotLock)this->streamLock.Release();
}

void Stream::Read(unsigned int ammount)
{
	bool gotLock = this->streamLock.TryLock();
	this->length = this->position + ammount;
	ammount = this->length - this->position - ammount > 0 ? ammount : this->length - this->position;
	System::Call(this->fd, StreamRead, (this->buffer), ammount);
	if (gotLock)this->streamLock.Release();

}

