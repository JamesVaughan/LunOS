#ifndef STREAMHANDEL_HPP
#define STREAMHANDEL_HPP
#include <io/Stream.h>
#include <synchronization.h>
namespace LunOS
{
namespace IO
{
/*
 * This class is used for automatically handling a pointer and provides automatic calling
 * of the closure of the object.
 */
	class ResourceHandle
	{
	public:
		ResourceHandle(void* resource, void (*OnExit)(void* resource));
	private:
		Lock lock;
		void (*OnExit)(void* resource);
		void* Resource;
		~ResourceHandle();
	};
}
}


#endif
