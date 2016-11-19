#include <kern/system.hpp>

Lock* User::userLock;
LinkedList* User::users;

User::User(const char* password)
{
	this->processes = new LinkedList();
	this->password = password;
}

User::~User()
{
	//TODO: Log the user off properly
}

bool User::ChangePassword(const char* oldPassword, const char* newPassword)
{
	if(this->checkPassword(oldPassword))
	{
		this->password = newPassword;
		return true;
	}
	return false;
}

void User::LoadUserIDs(unsigned int* buffer)
{
	GetUserLock();
	LinkedList* list = User::users;
	Node* node = list->Root;
	int i = 1;
	while(node != NULL)
	{
		User* user = (User*)node->data;
		buffer[i++] = user->userID;
		node = node->next;
	}
	ReleaseUserLock();
	buffer[0] = i - 1;
}


bool User::checkPassword(const char* password)
{
	bool success = false;
	GetUserLock();
	success = (strcmp(password,this->password) == 0);
	ReleaseUserLock();
	return success;
}

unsigned int User::GetCurrentUserID()
{
	return Sched->GetActiveThread()->GetProcess()->owner->userID;
}

void User::GetUserName(unsigned int userID, char* store, size_t bufferSize)
{
	LinkedList* list = User::users;
	User::GetUserLock();
	Node* current = list->Root;
	while(current != NULL)
	{
		if(userID == ((User*)current->data)->userID)
		{
			break;
		}
		current = current->next;
	}

	// If we actually found the user
	if(current != NULL)
	{
		User* curUser = (User*)current->data;
		size_t i;
		for(i = 0; curUser->name[i] && i < bufferSize;i++)
		{
			store[i] = curUser->name[i];
		}
		if(i == bufferSize)
		{
			// we are just going to make sure it is zero terminated
			store[bufferSize - 1] = 0;
		}
		else
		{
			store[i] = 0;
		}
	}
	else
	{
		// we need to fill this with an error
		if(bufferSize > 0)
			store[0] = 0;
	}
	User::ReleaseUserLock();
}

void User::GetUserLock()
{
	User::userLock->GetLock();
}

void User::ReleaseUserLock()
{
	User::userLock->Release();
}

void User::InitUsers()
{
	User::userLock = new Lock();
	User::users = new LinkedList();
}

User* User::GetUserWithID(unsigned int uid)
{
	Node* current;
	User::GetUserLock();
	current = User::users->Root;
	while(current != NULL && ((User*)current->data)->userID != uid)
	{
		current = current->next;
	}
	User::ReleaseUserLock();
	return (current == NULL? NULL : (User*)current->data);
}

//TODO: Make sure that we use an interlocked lib to access this next time, to ensure we don't double dip
volatile unsigned int uid = 1;
unsigned int User::CreateNewUser(const char* name, const char* newUserPassword, const char* userPassword)
{
	User* user = Sched->GetActiveProcess()->owner;
	if(user != NULL && !user->checkPassword(userPassword))
	{
		return NULL;
	}
	user = new User(newUserPassword);
	unsigned int size;
	user->name = ((const char*)new char[(size = strlen(name))]);
	memcpy((void*)user->name,(const void*)name,size);
	user->processes = new LinkedList();
	user->userID = uid++;
	User::users->AddLast(user);
	return user->userID;
}

