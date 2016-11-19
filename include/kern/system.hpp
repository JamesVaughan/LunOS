/*
 * System.h
 * James Vaughan, July 29th 2007
 *
 * This file includes all of the core kernel functionality.
 */
#ifndef __SYSTEM_H
#define __SYSTEM_H
typedef unsigned int size_t;
#define NULL 0
#define null 0

extern volatile unsigned int timer_ticks;

class System;

//Used for many data structures
#include "linkedList.h"
#include <Stdlib.h>

//core.h contains all of our important basic functions
#include "core.h"

//Does our memory management
#include "memory.h"

//String operations
#include "String.h"

// For the root of all I/O
#include <kern/io/io.h>

//does our console
#include "console.h"

//handels the keyboard
#include "keyboard.h"

//Used for managing the users
#include "user.h"

//handels our processes
#include "process.hpp"

//For power management needs
#include "powerManagement.h"

//For scheduling needs
#include "scheduler.hpp"

//Used for dealing with different users
#include "user.h"

//Used for dealing with the screen as a graphcs object
#include <kern/io/vga.h>

extern void IdleThread(void* unused);

class System
{
private:
	static void IncommingInterrupt(struct regs* r);
	static IO io;
public:
	static void Init();
	// This is where the operating system starts with its first thread
	static void SystemStart(void* params);
	static void Call(unsigned char fd, unsigned int callNumber, unsigned char* buffer, size_t bufferSize);
	static void InstallDevice(Device device);
};



#endif

