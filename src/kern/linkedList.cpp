#include <kern/linkedList.h>
#include <kern/console.h>
//define null here so we don't have to check every time
#ifndef NULL
#define NULL 0
#endif

Node::Node(void* item)
{
	this->data = item;
}

Node::Node()
{
	this->data = NULL;
	this->next = NULL;
	this->prev = NULL;
}

//***********************LinkedList********************************//
LinkedList::LinkedList()
{
	this->Length = 0;
	this->Root = NULL;
	this->Tail = NULL;
}

//Run through the list and burn it down behind it
LinkedList::~LinkedList()
{
	Node *cur = this->Root, *prev = NULL;
	while(cur)
	{
		prev = cur;
		cur = cur->next;
		delete prev;
	}
}

void LinkedList::AddLast(void* item)
{
	Node* n = new Node(item);
	n->next = NULL;
	this->AddLast(n);
}

void LinkedList::AddLast(Node* node)
{
	node->next = NULL;
	if(this->Tail == NULL) {
		this->Tail = this->Root = node;
		node->prev = NULL;
	}
	else{
		node->prev = this->Tail;
		this->Tail->next = node;
		this->Tail = node;
	}
	this->Length++;
}

Node* LinkedList::Search(void* item)
{
	Node* current = this->Root;
	if(current == NULL)
	{
		return NULL;
	}
	do
	{
		if(current->data == item)
		{
			return current;
		}
		current = current->next;
	}while((current != NULL) && (current != this->Root));
	return NULL;
}

bool LinkedList::RemoveItem(void* item)
{
	return (this->RemoveItem(item, true) != NULL);
}

Node* LinkedList::RemoveItem(void* item, bool free)
{
	Node* current = this->Root;
	if(current == NULL)
	{
		return NULL;
	}
	Node* foundNode = NULL;
	do
	{
		if(current->data == item)
		{
			if(free)
			{
				this->DeleteNode(current, true);
				foundNode = current;
			}
			else
			{
				this->DeleteNode(current, false);
				return current;
			}
		}
		current = current->next;
	}while((current != NULL) && (current != this->Root));
	return foundNode;
}

void LinkedList::DeleteNode(Node* node)
{
	this->DeleteNode(node, true);
}

void LinkedList::DeleteNode(Node* node, bool free)
{
	if((node->prev == NULL) | (node->prev == node))
	{
		this->Root = node->next;
	}
	if((node->next == NULL) | (node->next == node))
	{
		this->Tail = node->prev;
	}
	// Make sure people aren't doing funny things with looping
	if((node->prev != node) & (node->prev != NULL))
	{
		node->prev->next = node->next;
	}
	else
	{
		this->Root = this->Tail = NULL;
	}
	if(node->next != NULL)
	{
		node->next->prev = node->prev;
	}
	this->Length--;
	// Make sure we only delete it if we want to
	if(free)
	{
		delete node;
	}
}

void LinkedList::RemoveNode(Node* node)
{
	if((node->prev == NULL) | (node->prev == node))
	{
		this->Root = node->next;
	}
	if((node->next == NULL) | (node->next == node))
	{
		this->Tail = node->prev;
	}
	// Make sure people aren't doing funny things with looping
	if((node->prev != node) & (node->prev != NULL))
	{
		node->prev->next = node->next;
	}
	else
	{
		this->Root = this->Tail = NULL;
	}
	if(node->next != NULL)
	{
		node->next->prev = node->prev;
	}

	this->Length--;
}

//************************QUEUE************************************//
Queue::Queue()
{
	this->list = new LinkedList();
}

Queue::~Queue(){
	delete this->list;
}

unsigned int Queue::getSize(){
	return this->list == NULL? 0 : this->list->Length;
}

void* Queue::peek(){
	return (this->list == NULL || this->list->Root == NULL)
				? NULL : this->list->Root->data;
}

Node* Queue::peekNode()
{
	return (this->list == NULL || this->list->Root == NULL)
					? NULL : this->list->Root;
}

void* Queue::poll()
{
	Node* head;
	void* data;
	if(this->list == NULL || this->list->Root == NULL)
	{
		return NULL;
	}
	//store the old root
	head = this->list->Root;
	//move to the next element
	this->list->Root = this->list->Root->next;
	//store the heads data
	data = head->data;
	//release its resources
	delete head;
	//report that we removed an item
	this->list->Length--;
	//return the data from the old head
	return data;
}

Node* Queue::pollNode()
{
	Node* head = NULL;
	if(this->list != NULL && this->list->Root != NULL)
	{
		head = this->list->Root;
		this->list->Length--;
	}
	return head;
}

bool Queue::push(void* data)
{
	this->list->AddLast(data);
	return true;
}

bool Queue::pushNode(Node* node)
{
	this->list->AddLast(node);
	return true;
}

//**************************Stack****************************//

Stack::Stack(){
	this->list = new LinkedList();
}

Stack::~Stack()
{
	delete this->list;
}

bool Stack::push(void* data){
	Node* node = new Node;
	if(!node) return false;
	node->next = this->list->Root;
	this->list->Root = node;
	list->Length++;
	return true;
}

void* Stack::pop(){
	Node* head;
	void* data;
	if(this->list == NULL || this->list->Root == NULL) return NULL;
	//store the old root
	head = this->list->Root;
	//move to the next element
	this->list->Root = this->list->Root->next;
	//store the heads data
	data = head->data;
	//release its resources
	delete head;
	//report that we removed an item
	this->list->Length--;
	//return the data from the old head
	return data;
}

