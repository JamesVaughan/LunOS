#ifndef SYSTEMCALLS_H_
#define SYSTEMCALLS_H_

#ifndef NULL
#define NULL 0
#endif

#include <io/Io.h>

extern "C"
{
	void Sys_Call(unsigned int ecx, unsigned int ebx, unsigned int eax);
}

namespace LunOS { namespace IO {
class Stream; // we need this so stream can actually call IO
}}

namespace LunOS
{
	namespace Calls
	{
		enum SystemCalls{
			Yield,
			Sleep,
			CreateProcess,
			CreateThread,
			KillThread,
			KillProcess,
			ExitProcess,
			ExitThread,
			GetThreadID,
			GetProcessID,
			GetUserID,
			GetUserName,
			GetUsers,
			LogOn,
			LogOff,
			SwitchProcessToUser,
			CreateUser,
			ShutdownComputer,
			GetNumberOfDevices,
			GetDeviceInfo
		};
	}

	// This is the class used to interact with the operating system itself
	class System
	{
	public:
		// Make the current thread yield
		static void Yield(void);

		// Make the current thread sleep for a given time
		static void Sleep(unsigned int milliseconds);

		// Gets the current user ID for the given process
		static unsigned int GetUserID();

		// Gets the current user Name for the process.
		static const char* GetUserName();

		// Get the name of the user with the given name
		static const char* GetUserName(unsigned int uid);

		// Change who the process is running under
		static bool SwitchCurrentProcessToUser(unsigned int uid, const char* password);

		// Try to create a new user
		static unsigned int CreateNewUser(const char* name, const char* password, const char* yourPassword);
		// Creates a thread with parameters
		static unsigned int CreateThread(unsigned char* name, unsigned int (*startingAddress)(void*), void* params);

		static unsigned int* GetActiveUsers();

		static unsigned int GetNumberOfDevices();

		static LunOS::IO::DeviceInfo GetDeviceInfo(unsigned int deviceNumber);

		static void KillThread();

	private:
		static void Call(unsigned char fd, unsigned int callNumber, unsigned char* buffer, unsigned int bufferSize )
		{
			Sys_Call(bufferSize, (unsigned int)buffer, ((callNumber<<8) + fd));
		}

	friend class LunOS::IO::Stream;
	};
}



#endif /*SYSTEMCALLS_H_*/
