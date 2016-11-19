/*
 * StreamHandel.cpp
 *
 *  Created on: 2013-08-21
 *      Author: james
 */
#include <io/ResourceHandle.hpp>
#ifndef NULL
#define NULL 0
#endif

namespace LunOS
{
namespace IO
{
ResourceHandle::ResourceHandle(void* resource, void (*OnExit)(void* resource))
	{
		this->lock = Lock();
		this->Resource = resource;
		this->OnExit = OnExit;
	}

ResourceHandle::~ResourceHandle()
	{
		this->lock.GetLock();
		if(this->Resource)
		{
			this->OnExit(this->Resource);
			delete (unsigned int*)this->Resource;
			this->Resource = NULL;
		}
		this->lock.Release();
	}
}
}
