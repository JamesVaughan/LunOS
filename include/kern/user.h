#ifndef USER_H_
#define USER_H_

//Promise that we are going to declare what a User is
class User;

#include <kern/system.hpp>
#include <kern/linkedList.h>
#include <synchronization.h>

class User
{
	public:

		User(const char* password);
		~User();
		bool ChangePassword(const char* oldPassowrd, const char* newPassword);
		//The list of users
		static LinkedList* users;
		// The processes that are owned by this user
		LinkedList* processes;
		//The privilege level for this user
		unsigned int privileges;
		//The user's ID number
		unsigned int userID;
		//The name of the user
		const char* name;

		//Checks to see if the password for the user is valid.
		bool checkPassword(const char* password);

		// Gets the current user's ID
		static unsigned int GetCurrentUserID();

		// Returns the User's name
		static void GetUserName(unsigned int userID, char* store, size_t bufferSize);

		// Loads all of the ID's for active users
		static void LoadUserIDs(unsigned int* buffer);

		// Call this to start up the engine for users, REQUIRES DYNAMIC MEMORY
		static void InitUsers();

		// Returns the user object that has the given ID [DO NOT GIVE THIS TO ANYTHING OUTSIDE OF THE KERNEL!!]
		static User* GetUserWithID(unsigned int uid);

		// Creates a new user for the system [returns the ID]
		static unsigned int CreateNewUser(const char* name, const char* newUserPassword, const char* userPassword);

	private:
		//The user's password
		const char* password;

		static Lock* userLock;
		// Locks the user lock
		static void GetUserLock();
		// releases the user lock
		static void ReleaseUserLock();
};

#endif /*USER_H_*/
