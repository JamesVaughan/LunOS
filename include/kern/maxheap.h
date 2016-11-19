#ifndef MAXHEAP_H_
#define MAXHEAP_H_

typedef struct maxheap
{
	unsigned int length;
	struct maxheap* left;
	struct maxheap* right;
	char*  address;
} maxheap;

class MaxHeap
{
	public:
	static void heapify(maxheap* heap);
	static bool insert(maxheap* heap, maxheap* node);
	static bool remove(maxheap* heap, maxheap* node);
	static maxheap* pop(maxheap* heap);
};
 

#endif /*MAXHEAP_H_*/
