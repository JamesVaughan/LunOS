#ifndef LINKEDLIST_H_
#define LINKEDLIST_H_
typedef struct Node
{
	void* data;
	Node* next;
	Node* prev;
	Node();
	Node(void* item);
} Node;

class LinkedList
{
	public:
	LinkedList();
	~LinkedList();
	Node* Root;
	Node* Tail;
	void AddLast(void* item);
	// adds the specific node to the end of the list
	void AddLast(Node* item);
	// Remove and free the space held by this node
	void RemoveNode(Node* node);

	Node* Search(void* item);
	// Remove and only free the memory held by the node
	bool RemoveItem(void* item);
	// Returns the node that was holding this item
	Node* RemoveItem(void* item, bool free);
	void DeleteNode(Node* node);
	void DeleteNode(Node* node, bool free);
	unsigned int Length;
};

class Queue
{
	private:
	LinkedList* list;
	public:
	Queue();
	~Queue();
	void* poll();
	Node* pollNode();
	void* peek();
	Node* peekNode();
	bool push(void* data);
	bool pushNode(Node* node);
	unsigned int getSize();
};

class Stack
{
	private:
	LinkedList* list;
	public:
	Stack();
	~Stack();
	void* pop();
	void* peek();
	bool push(void* data);
	unsigned int getSize();
};

#endif /*LINKEDLIST_H_*/
