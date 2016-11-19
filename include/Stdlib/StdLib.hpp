/*
 * StdLib.hpp
 *
 *  Created on: 22-Jul-2009
 *      Author: james
 */

#ifndef STDLIB_HPP_
#define STDLIB_HPP_

typedef unsigned int size_t;
#include <kern/core.h>
#include <kern/console.h>

#ifndef NULL
#define NULL 0
#endif

#ifndef SafeDelete
#define SafeDelete(X) if(!X){delete X; X = NULL; }
#endif

template<class T> class Array
{
public:
	Array(unsigned int length)
	{
		this->ArrayLength = length;
		this->Data = new T[length];
	}

	~Array()
	{
		SafeDelete(this->Data);
	}

	T& operator[](const int nIndex)
	{
		return this->Data[nIndex];
	}

	unsigned int Length() const
	{
		return this->ArrayLength;
	}
private:
	T* Data;
	unsigned int ArrayLength;
};

//LunLib
namespace LunOS
{
namespace DataStructures
{
// Our Node Types
template<class T> class LinkNode;
template<class T> class TreeNode;

//Iterator
template<class T> class Iterator;

template<class T> class DataStruct;

template<class T> class LinkedList;

template<class T> class Queue
{
public:
	Queue()
	{
		this->Size = 10;
		this->Count = 0;
		this->Head = 0;
		this->Tail = 0;
		this->Data = new T[this->Size];
		memset(this->Data, 0, sizeof(T) * this->Size);
	}

	void Add(T data)
	{
		if (this->Count == this->Size)
		{
			T* temp = new T[this->Size * 2];
			memcpy((void*) temp, (void*) (this->Data + this->Head),
					sizeof(T) * (this->Size - this->Head));
			memcpy((void*) (temp + this->Head), (void*) this->Data,
					sizeof(T) * (this->Count - (this->Size - this->Head)));
			delete this->Data;
			this->Size *= 2;
			this->Data = temp;
		}
		this->Data[this->Tail] = data;
		this->Tail = (this->Tail + 1) % this->Size;
		this->Count++;
	}

	T RemoveLast()
	{
		if (this->Count > 0)
		{
			unsigned int tail = this->Tail;
			// since they are unsigned we would end up with max value if we wrapped around
			if (tail == 0)
			{
				this->Tail = this->Size - 1;
			}
			else
			{
				this->Tail = (tail - 1);
			}
			this->Count--;
			return this->Data[tail];
		}
		return NULL;
	}

	T Dequeue()
	{
		if (this->Count > 0)
		{
			unsigned int head = this->Head;
			this->Head = (head + 1) % this->Size;
			this->Count--;
			return this->Data[head];
		}
		return NULL;
	}

	void Clear()
	{
		this->Count = 0;
		this->Head = 0;
		this->Tail = 0;
	}

	unsigned int GetSize()
	{
		return this->Count;
	}

	~Queue()
	{
		SafeDelete(this->Data);
	}

private:
	unsigned int Size;
	unsigned int Head;
	unsigned int Tail;
	unsigned int Count;
	T* Data;
};

template<class T> class Stack;

template<class T> class FixedStack
{
public:
	FixedStack(unsigned int size)
	{
		this->Size = size;
		this->Index = 0;
		this->Data = new T[this->Size];
		memset(this->Data, 0, sizeof(T) * size);
	}

	void Push(T data)
	{
		this->Data[this->Index++] = data;
	}

	T PoP()
	{
		return this->Data[--this->Index];
	}

	void Clear()
	{
		memset(this->Data, 0, sizeof(T) * this->Size);
		this->Index = 0;
	}

	unsigned int GetSize()
	{
		return this->Index;
	}

	~FixedStack()
	{
		delete[] this->Data;
		this->Data = NULL;
	}

private:
	unsigned int Size;
	unsigned int Index;
	T* Data;
};

template<class T> class Heap;
template<class T> class MaxHeap;
template<class T> class MinHeap;

/*	template <class T, class U> Map;
 template <class T, class U> HashMap;
 template <class T, class U> HashTable;
 */
}
}
;

#endif /* STDLIB_HPP_ */
