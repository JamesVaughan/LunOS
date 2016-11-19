/*
 * Sata.h
 *
 *  Created on: 2011-03-29
 *      Author: james
 *
 *      Some notes for the future...
 *      Device == Ahci card plugged into the PCI BUS
 *      Drive == Sata HDD/CD/DVD drive
 */

#ifndef KERN_IO_SATA_H_
#define KERN_IO_SATA_H_

#include <kern/io/io.h>
#include <io/DiskAccess.h>
#include <synchronization.h>
#include <kern/linkedList.h>

namespace LunOS
{

namespace IO
{

namespace Kern
{
	namespace AchiFrame
	{
		typedef enum
		{
			REG_H2D	    = 0x27,	// Register FIS - host to device
			REG_D2H	    = 0x34,	// Register FIS - device to host
			DMA_ACT	    = 0x39,	// DMA activate FIS - device to host
			DMA_SETUP	= 0x41,	// DMA setup FIS - bidirectional
			DATA		= 0x46,	// Data FIS - bidirectional
			BIST		= 0x58,	// BIST activate FIS - bidirectional
			PIO_SETUP	= 0x5F,	// PIO setup FIS - device to host
			DEV_BITS	= 0xA1,	// Set device bits FIS - device to host
		} FrameType;
	}

	typedef struct AhciHostToDevice
	{
		// DWORD 0
		unsigned char Type;	// FIS_TYPE_REG_H2D

		unsigned char PortMultiplyer:4;	// Port multiplier
		unsigned char RESERVED0:3;		// Reserved
		unsigned char CommandOrControl:1;		// 1: Command, 0: Control

		unsigned char Command;	// Command register
		unsigned char FeatureLow;	// Feature register, 7:0

		// DWORD 1
		unsigned char LBA0;		// LBA low register, 7:0
		unsigned char LBA1;		// LBA mid register, 15:8
		unsigned char LBA2;		// LBA high register, 23:16
		unsigned char Device;		// Device register

		// DWORD 2
		unsigned char LBA3;		// LBA register, 31:24
		unsigned char LBA4;		// LBA register, 39:32
		unsigned char LBA5;		// LBA register, 47:40
		unsigned char FeatureHigh;	// Feature register, 15:8

		// DWORD 3
		unsigned char CountLow;		// Count register, 7:0
		unsigned char CountHigh;		// Count register, 15:8
		unsigned char IscochronousCommandComplete;		// Isochronous command completion
		unsigned char Control;	// Control register

		// DWORD 4
		unsigned char rsv1[4];	// Reserved
	} __attribute__((packed)) AhciHostToDevice;

	typedef struct AhciDeviceToHost
	{
		// DWORD 0
		unsigned char Type;    // FIS_TYPE_REG_D2H

		unsigned char PortMultiplyer:4;    // Port multiplier
		unsigned char RESERVED0:2;      // Reserved
		unsigned char Interrupt:1;         // Interrupt bit
		unsigned char RESERVED1:1;      // Reserved

		unsigned char Status;      // Status register
		unsigned char Error;       // Error register

		// DWORD 1
		unsigned char LBA0;        // LBA low register, 7:0
		unsigned char LBA1;        // LBA mid register, 15:8
		unsigned char LBA2;        // LBA high register, 23:16
		unsigned char Device;      // Device register

		// DWORD 2
		unsigned char LBA3;        // LBA register, 31:24
		unsigned char LBA4;        // LBA register, 39:32
		unsigned char LBA5;        // LBA register, 47:40
		unsigned char RESERVED2;        // Reserved

		// DWORD 3
		unsigned char CountLow;      // Count register, 7:0
		unsigned char CountHigh;      // Count register, 15:8
		unsigned char RESERVED3[2];     // Reserved

		// DWORD 4
		unsigned char RESERVED4[4];     // Reserved
	} __attribute__((packed)) AhciDeviceToHost;

	typedef struct AhciData
	{
		// DWORD 0
		unsigned char Type;	// FIS_TYPE_DATA

		unsigned char PortMultiplyer:4;	// Port multiplier
		unsigned char RESERVED0:4;		// Reserved

		unsigned char RESERVED1[2];	// Reserved

		// DWORD 1 ~ N
		unsigned int Data[1];	// Payload
	} __attribute__((packed)) AhciData;

	typedef struct AhciPio
	{
		// DWORD 0
		unsigned char Type;	// FIS_TYPE_PIO_SETUP

		unsigned char PortMultiplyer:4;	// Port multiplier
		unsigned char RESERVED0:1;		// Reserved
		unsigned char DataToHost:1;		// Data transfer direction, 1 - device to host
		unsigned char Interrupt:1;		// Interrupt bit
		unsigned char RESERVED1:1;

		unsigned char Status;		// Status register
		unsigned char Error;		// Error register

		// DWORD 1
		unsigned char LBA0;		// LBA low register, 7:0
		unsigned char LBA1;		// LBA mid register, 15:8
		unsigned char LBA2;		// LBA high register, 23:16
		unsigned char Device;		// Device register

		// DWORD 2
		unsigned char LBA3;		// LBA register, 31:24
		unsigned char LBA4;		// LBA register, 39:32
		unsigned char LBA5;		// LBA register, 47:40
		unsigned char RESERVED2;		// Reserved

		// DWORD 3
		unsigned char CountLow;		// Count register, 7:0
		unsigned char CountHigh;		// Count register, 15:8
		unsigned char RESERVED3;		// Reserved
		unsigned char NewStatus;	// New value of status register

		// DWORD 4
		unsigned short TransferCount;		// Transfer count
		unsigned char RESERVED4[2];	// Reserved
	} __attribute__((packed)) AhciPio;

	typedef volatile struct
	{
		unsigned int CommandListLow;		// 0x00, command list base address, 1K-byte aligned
		unsigned int CommandListHigh;		// 0x04, command list base address upper 32 bits
		unsigned int FrameLow;		// 0x08, FIS base address, 256-byte aligned
		unsigned int FrameHigh;		// 0x0C, FIS base address upper 32 bits
		unsigned int InterruptStatus;		// 0x10, interrupt status
		unsigned int InterruptEnable;		// 0x14, interrupt enable
		unsigned int CommandAndStatus;		// 0x18, command and status
		unsigned int RESERVED0;		// 0x1C, Reserved
		unsigned int TaskFileData;		// 0x20, task file data
		unsigned int Signature;		// 0x24, signature
		unsigned int SataStatus;		// 0x28, SATA status (SCR0:SStatus)
		unsigned int SataControl;		// 0x2C, SATA control (SCR2:SControl)
		unsigned int SataError;		// 0x30, SATA error (SCR1:SError)
		unsigned int SataActive;		// 0x34, SATA active (SCR3:SActive)
		unsigned int CommandIssue;		// 0x38, command issue
		unsigned int SataNotification;		// 0x3C, SATA notification (SCR4:SNotification)
		unsigned int FrameSwitchControl;		// 0x40, FIS-based switch control
		unsigned int RESERVED1[11];	// 0x44 ~ 0x6F, Reserved
		unsigned int VENDORSPECIFIC[4];	// 0x70 ~ 0x7F, vendor specific

	}__attribute__((packed)) AhciPort;

	typedef volatile struct
	{
		// 0x00 - 0x2B, Generic Host Control
		unsigned int HostCapability; // 0x00, Host capability
		unsigned int GlobalHostControl;	// 0x04, Global host control
		unsigned int InterruptStatus; // 0x08, Interrupt status
		unsigned int PortImplemented;	// 0x0C, Port implemented (binary)
		unsigned int Version;		// 0x10, Version
		unsigned int CoalecingControlComplete;	// 0x14, Command completion coalescing control
		unsigned int CoalesingPortComplete;	// 0x18, Command completion coalescing ports
		unsigned int EnclosureLocation;		// 0x1C, Enclosure management location
		unsigned int EnclosureControl;		// 0x20, Enclosure management control
		unsigned int ExtendedCapabilities;	// 0x24, Host capabilities extended
		unsigned int BiosOSHandofControlStatus;		// 0x28, BIOS/OS handoff control and status

		// 0x2C - 0x9F, Reserved
		unsigned char RESERVED[0xA0-0x2C];

		// 0xA0 - 0xFF, Vendor specific registers
		unsigned char VensorSpecificRegisters[0x100-0xA0];

		// 0x100 - 0x10FF, Port control registers
		AhciPort ports[1];	// 1 ~ 32

	}__attribute__((packed)) AhciRegisters;

	typedef struct
	{
		// DW0
		unsigned char cfl:5;	// Command FIS length in DWORDS, 2 ~ 16
		unsigned char a:1;		// ATAPI
		unsigned char w:1;		// Write, 1: H2D, 0: D2H
		unsigned char p:1;		// Prefetchable

		unsigned char r:1;		// Reset
		unsigned char b:1;		// BIST
		unsigned char c:1;		// Clear busy upon R_OK
		unsigned char rsv0:1;	// Reserved
		unsigned char pmp:4;	// Port multiplier port

		unsigned short	prdtl;	// Physical region descriptor table length in entries

		// DW1
		volatile
		unsigned int prdbc;		// Physical region descriptor byte count transferred

		// DW2, 3
		unsigned int ctba;		// Command table descriptor base address
		unsigned int ctbau;		// Command table descriptor base address upper 32 bits

		// DW4 - 7
		unsigned int	rsv1[4];	// Reserved
	} __attribute__((packed)) HBA_CMD_HEADER;

	typedef struct
	{
		unsigned int dba;		// Data base address
		unsigned int dbau;		// Data base address upper 32 bits
		unsigned int rsv0;		// Reserved

		// DW3
		unsigned int dbc:22;	// Byte count, 4M max
		unsigned int rsv1:9;	// Reserved
		unsigned int i:1;		// Interrupt on completion
	} __attribute__((packed))  HBA_PRDT_ENTRY;

	typedef struct
	{
		// 0x00
		unsigned char cfis[64];	// Command FIS

		// 0x40
		unsigned char acmd[16];	// ATAPI command, 12 or 16 bytes

		// 0x50
		unsigned char rsv[48];	// Reserved

		// 0x80
		HBA_PRDT_ENTRY	prdt_entry[1];	// Physical region descriptor table entries, 0 ~ 65535
	} __attribute__((packed))  HBA_CMD_TBL;

	typedef volatile struct
	{
		// 0x00
		unsigned char dsfis[16];		// DMA Setup FIS
		unsigned char pad0[4];

		// 0x20
		AhciPio	psfis;		// PIO Setup FIS
		unsigned char pad1[12];

		// 0x40
		AhciDeviceToHost	rfis;		// Register – Device to Host FIS
		unsigned char pad2[4];

		// 0x58
		unsigned short	sdbfis;		// Set Device Bit FIS

		// 0x60
		unsigned char ufis[64];   //unknown FIS

		// 0xA0
		unsigned char rsv[0x100-0xA0];
	} __attribute__((packed)) HBA_FIS;


	typedef struct
	{
		AhciRegisters* Registers;
		unsigned int InterruptVector;
	} AhciDevice;

	typedef struct
	{
		DiskInfo BasicInformation;
		AhciDevice* Controller;
		unsigned int DeviceNumber;
		unsigned int* BufferMemoryVirtual[32];
		unsigned int* BufferMemoryPhysical[32];
		HBA_CMD_HEADER* CommandHeader;
		HBA_CMD_TBL* CommandTable;

	}__attribute__((packed)) SataDiskInfo;

	class Sata
	{
	public:
		// Returns true if we have detected an ahci system attached to the computer
		static bool ScanAndInitialise();
	private:
		// For user space IO
		static bool Open(FDTEntry* fdt, unsigned char* buffer, size_t length);
		static bool Read(FDTEntry* fdt, unsigned char* buffer, size_t length);
		static bool Write(FDTEntry* fdt, unsigned char* buffer, size_t length);
		static bool Close(FDTEntry* entry);

		// Initialise an ahci device
		static void InitialiseDevice(LinkedList* driveList, unsigned int deviceNumber);
		static void StopPort(void* port);
		static void StartPort(void* port);

		static void IRQHandler(struct regs *r);

		static unsigned int AHCIVersion;
		static unsigned int NumberOfDevices;
		static AhciDevice* DeviceList;
		static SataDiskInfo* DriveInformation;
		static unsigned int NumberOfDrives;
	};
}

}

}


#endif /* SATA_H_ */
