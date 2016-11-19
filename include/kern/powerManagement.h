#ifndef POWERMANAGEMENT_H
#define POWERMANAGEMENT_H

typedef struct{
	char signature[8];
	unsigned char checkSum;
	char oemID[6];
	unsigned char revision;
	unsigned int rsdtAddress;
}RSDPDescriptor;

typedef struct
{
 char signature[8];
 unsigned char checkSum;
 char oemID[6];
 unsigned char revision;
 unsigned int rsdtAddress;

 unsigned int length;
 long long xsdtAddress;
 unsigned char extCheckSum;
 unsigned char reserved[3];
} RSDPDescriptor20;

typedef struct
{
	 char signature[4];
	 unsigned int length;
	 unsigned char revision;
	 unsigned char checkSum;
	 char oemID[6];
	 char oemTableID[8];
	 unsigned int oemRevision;
	 unsigned int creatorID;
	 unsigned int creatorRevision;
} SDTHeader;

typedef struct
{
	 char signature[4];
	 unsigned int length;
	 unsigned char revision;
	 unsigned char checkSum;
	 char oemID[6];
	 char oemTableID[8];
	 unsigned int oemRevision;
	 unsigned int creatorID;
	 unsigned int creatorRevision;
	 // We have "length" entries
	 SDTHeader* entry;
} RSDTHeader;

typedef struct
{
	unsigned WBINVD:1;
	unsigned WBINVD_FLUSH:1;
	unsigned PROC_C1:1;
	unsigned P_LVL2_UP;
	unsigned PWR_BUTTON:1;
	unsigned SLP_BUTTON:1;
	unsigned FIX_RTC:1;
	unsigned RTC_S4:1;
	unsigned TMR_VAL_EXT:1;
	unsigned DCK_CAP:1;
	unsigned _RESERVED:22;
} FACPFlags;

typedef struct
{
	unsigned S4BIOS_F:1;
	unsigned _RESERVED:31;
} FACSFlags;

typedef struct
{
	unsigned Pending:1;
	unsigned Owned:1;
	unsigned _RESERVED:30;
} ACPIGlobalLock;

typedef struct
{
	char Signature[4];
	unsigned int Length;
	unsigned int HardwareSignature;
	unsigned int FirmwareWakingVector;
	ACPIGlobalLock GlobalLock;
	FACSFlags flags;
	long long _RESERVED:40;
} FACS;

typedef struct
{
	 char signature[4];
	 unsigned int length;
	 unsigned char revision;
	 unsigned char checkSum;
	 char oemID[6];
	 char oemTableID[8];
	 unsigned int oemRevision;
	 unsigned int creatorID;
	 unsigned int creatorRevision;
	 FACS* firmwareControlStructure;
	 void* DSDT;
	 /* 0  Dual PIC, industry standard PC-AT type
		   implementation with 0-15 IRQs with EISA
		   edge-level-control register.
		1  Multiple APIC. Local processor APICs
		   with one or more IO APICs as defined by
		   the Multiple APIC Description Table.
		>1 Reserved. */
	 unsigned char int_model;
	 unsigned char _RESERVED;
	 /* Interrupt pin the SCI interrupt is wired to in 8259
	 mode. The OS is required to treat the ACPI SCI
	 interrupt as a sharable, level, active low interrupt*/
	 unsigned short sci_int;
	 /*System port address of the SMI Command Port.
       During ACPI OS initialization, the OS can
       determine that the ACPI hardware registers are
       owned by SMI (by way of the SCI_EN bit), in
       which case the ACPI OS issues the
       SMI_DISABLE command to the SMI_CMD
       port. The SCI_EN bit effectively tracks the
       ownership of the ACPI hardware registers. The
       OS issues commands to the SMI_CMD port
       synchronously from the boot processor.*/
	 unsigned int smi_cmd;
	 /*	The value to write to SMI_CMD to disable SMI
        ownership of the ACPI hardware registers. The
		last action SMI does to relinquish ownership is to
		set the SCI_EN bit. The OS initialization process
		will synchronously wait for the ownership
		transfer to complete, so the ACPI system releases
		SMI ownership as timely as possible. */
	 unsigned char acpi_enable;
	 unsigned char acpi_disable;
	 unsigned char s4bios_req;
	 unsigned char _RESERVED2;
	 unsigned int PM1a_EVT_BLK;
	 unsigned int PM1b_EVT_BLK;
	 unsigned int PM1a_CNT_BLK;
	 unsigned int PM1b_CNT_BLK;
	 unsigned int PM2_CNT_BLK;
	 unsigned int PM_TMR_BLK;
	 unsigned int GPE0_BLK;
	 unsigned int GPE1_BLK;
	 unsigned char PM1_EVT_LEN;
	 unsigned char PM2_EVT_LEN;
	 unsigned char PM_TM_LEN;
	 unsigned char GPE0_BLK_LEN;
	 unsigned char GPE1_BLK_LEN;
	 unsigned char GPE1_BASE;
	 unsigned char _RESERVED3;
	 unsigned short P_LVL2_LAT;
	 unsigned short P_LVL3_LAT;
	 unsigned short FLUSH_SIZE;
	 unsigned short FLUSH_STRIDE;
	 unsigned char DUTY_OFFSET;
	 unsigned char DUTY_WIDTH;
	 unsigned char DUTY_ALRM;
	 unsigned char DAY_ALRM;
	 unsigned char MON_ALRM;
	 unsigned char CENTURY;
	 unsigned char _RESERVED4;
	 unsigned char _RESERVED5;
	 unsigned char _RESERVED6;
	 FACPFlags Flags;

} FACPHeader;

class PowerManagement{
public:
	void Init();
	void Sleep();
	void Shutdown();
	void Reboot();
private:
	bool ACPISupport;
	unsigned char ACPIVersion;
	RSDTHeader* RSDT;
	//Multiple APIC Description Table. See section 5.2.8.
	bool isAPIC(SDTHeader* header);
	//Differentiated System Description Table. See section 5.2.7.1
	bool isDSDT(SDTHeader* header);
	//Fixed ACPI Description Table. See section 5.2.5.
	bool isFACP(SDTHeader* header);
	//Firmware ACPI Control Structure. See section 5.2.6
	bool isFACS(SDTHeader* header);
	//Persistent System Description Table. See section 5.2.7.3.
	bool isPSDT(SDTHeader* header);
	//Root System Description Table. See section 5.2.4.
	bool isRSDT(SDTHeader* header);
	//Secondary System Description Table. See section 5.2.7.2
	bool isSSDT(SDTHeader* header);
	//Smart Battery Specification Table. See section 5.2.9
	bool isSBST(SDTHeader* header);
};

extern PowerManagement powerManagement;

#endif

