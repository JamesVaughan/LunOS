#include <kern/maxheap.h>
#ifndef NULL
#define NULL 0
#endif

void MaxHeap::heapify(maxheap* heap)
{
	unsigned int right, left;
	void* temp;
	if(heap == NULL) return;
	heapify(heap->left);
	heapify(heap->right);
	//fill the values (remember -1 = 0xffffffff)
	left = heap->left == NULL? -NULL : heap->left->length;
	right = heap->right == NULL? NULL : heap->right->length;
	if (left > heap->length && left > right)
	{
		 temp = heap->address;
		 heap->address = heap->left->address;
		 heap->left->address = (char*)temp;
		 
		 temp = (void*)heap->length;
		 heap->length = heap->left->length;
		 heap->left->length = (unsigned int)temp;
		 
		 MaxHeap::heapify(heap->left);
	}
	else if(right > heap->length)
	{
		 temp = heap->address;
		 heap->address = heap->right->address;
		 heap->right->address = (char*)temp;
		 
		 temp = (void*)heap->length;
		 heap->length = heap->right->length;
		 heap->right->length = (unsigned int)temp;
		 
		 MaxHeap::heapify(heap->right);
	}
}

bool MaxHeap::insert(maxheap* heap,maxheap* node)
{
	return false;
}

maxheap* MaxHeap::pop(maxheap* heap){
	maxheap* head = NULL;
	if (heap != NULL){
		
	}
	return head;
}

