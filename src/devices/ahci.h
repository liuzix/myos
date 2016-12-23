//
// Created by Zixiong Liu on 12/20/16.
//

#ifndef MYOS_AHCI_H
#define MYOS_AHCI_H

#include "pci.h"
#include "../sync.h"

void ahci_init_controller (pci_device* device);

typedef uint8_t BYTE;
typedef uint16_t WORD;
typedef uint32_t DWORD;
typedef uint64_t QWORD;

typedef struct tagHBA_CMD_HEADER
{
    // DW0
    BYTE	cfl:5;		// Command FIS length in DWORDS, 2 ~ 16
    BYTE	a:1;		// ATAPI
    BYTE	w:1;		// Write, 1: H2D, 0: D2H
    BYTE	p:1;		// Prefetchable

    BYTE	r:1;		// Reset
    BYTE	b:1;		// BIST
    BYTE	c:1;		// Clear busy upon R_OK
    BYTE	rsv0:1;		// Reserved
    BYTE	pmp:4;		// Port multiplier port

    WORD	prdtl;		// Physical region descriptor table length in entries

    // DW1
    volatile
    DWORD	prdbc;		// Physical region descriptor byte count transferred

    // DW2, 3
    DWORD	ctba;		// Command table descriptor base address
    DWORD	ctbau;		// Command table descriptor base address upper 32 bits

    // DW4 - 7
    DWORD	rsv1[4];	// Reserved
}  __attribute__((packed)) HBA_CMD_HEADER;

typedef struct tagHBA_PRDT_ENTRY
{
    DWORD	dba;		// Data base address
    DWORD	dbau;		// Data base address upper 32 bits
    DWORD	rsv0;		// Reserved

    // DW3
    DWORD	dbc:22;		// Byte count, 4M max
    DWORD	rsv1:9;		// Reserved
    DWORD	i:1;		// Interrupt on completion

} __attribute__((packed)) HBA_PRDT_ENTRY;

typedef struct tagHBA_CMD_TBL
{
    // 0x00
    BYTE	cfis[64];	// Command FIS

    // 0x40
    BYTE	acmd[16];	// ATAPI command, 12 or 16 bytes

    // 0x50
    BYTE	rsv[48];	// Reserved

    // 0x80
    HBA_PRDT_ENTRY	prdt_entry[1];	// Physical region descriptor table entries, 0 ~ 65535
} __attribute__((packed)) HBA_CMD_TBL;



typedef struct tagFIS_REG_H2D
{
    // DWORD 0
    BYTE	fis_type;	// FIS_TYPE_REG_H2D

    BYTE	pmport:4;	// Port multiplier
    BYTE	rsv0:3;		// Reserved
    BYTE	c:1;		// 1: Command, 0: Control

    BYTE	command;	// Command register
    BYTE	featurel;	// Feature register, 7:0

    // DWORD 1
    BYTE	lba0;		// LBA low register, 7:0
    BYTE	lba1;		// LBA mid register, 15:8
    BYTE	lba2;		// LBA high register, 23:16
    BYTE	device;		// Device register

    // DWORD 2
    BYTE	lba3;		// LBA register, 31:24
    BYTE	lba4;		// LBA register, 39:32
    BYTE	lba5;		// LBA register, 47:40
    BYTE	featureh;	// Feature register, 15:8

    // DWORD 3
    BYTE	countl;		// Count register, 7:0
    BYTE	counth;		// Count register, 15:8
    BYTE	icc;		// Isochronous command completion
    BYTE	control;	// Control register

    // DWORD 4
    BYTE	rsv1[4];	// Reserved
}  __attribute__((packed)) FIS_REG_H2D;

typedef struct tagFIS_REG_D2H
{
    // DWORD 0
    BYTE	fis_type;    // FIS_TYPE_REG_D2H

    BYTE	pmport:4;    // Port multiplier
    BYTE	rsv0:2;      // Reserved
    BYTE	i:1;         // Interrupt bit
    BYTE	rsv1:1;      // Reserved

    BYTE	status;      // Status register
    BYTE	error;       // Error register

    // DWORD 1
    BYTE	lba0;        // LBA low register, 7:0
    BYTE	lba1;        // LBA mid register, 15:8
    BYTE	lba2;        // LBA high register, 23:16
    BYTE	device;      // Device register

    // DWORD 2
    BYTE	lba3;        // LBA register, 31:24
    BYTE	lba4;        // LBA register, 39:32
    BYTE	lba5;        // LBA register, 47:40
    BYTE	rsv2;        // Reserved

    // DWORD 3
    BYTE	countl;      // Count register, 7:0
    BYTE	counth;      // Count register, 15:8
    BYTE	rsv3[2];     // Reserved

    // DWORD 4
    BYTE	rsv4[4];     // Reserved
}  __attribute__((packed)) FIS_REG_D2H;

typedef struct tagFIS_DMA_SETUP
{
    // DWORD 0
    BYTE	fis_type;	// FIS_TYPE_DMA_SETUP

    BYTE	pmport:4;	// Port multiplier
    BYTE	rsv0:1;		// Reserved
    BYTE	d:1;		// Data transfer direction, 1 - device to host
    BYTE	i:1;		// Interrupt bit
    BYTE	a:1;            // Auto-activate. Specifies if DMA Activate FIS is needed

    BYTE    rsved[2];       // Reserved

    //DWORD 1&2

    QWORD   DMAbufferID;    // DMA Buffer Identifier. Used to Identify DMA buffer in host memory. SATA Spec says host specific and not in Spec. Trying AHCI spec might work.

    //DWORD 3
    DWORD   rsvd;           //More reserved

    //DWORD 4
    DWORD   DMAbufOffset;   //Byte offset into buffer. First 2 bits must be 0

    //DWORD 5
    DWORD   TransferCount;  //Number of bytes to transfer. Bit 0 must be 0

    //DWORD 6
    DWORD   resvd;          //Reserved

}  __attribute__((packed)) FIS_DMA_SETUP;

typedef volatile struct tagHBA_FIS
{
    // 0x00
    FIS_DMA_SETUP	dsfis;		// DMA Setup FIS
    BYTE		pad0[4];

    // 0x20
    //FIS_PIO_SETUP	psfis;		// PIO Setup FIS
    BYTE		pad1[20];
    BYTE		pad_pio[12];

    // 0x40
    FIS_REG_D2H	rfis;		// Register – Device to Host FIS
    BYTE		pad2[4];

    // 0x58
    //FIS_DEV_BITS	sdbfis;		// Set Device Bit FIS
    BYTE		pad_bits[2];
    // 0x60
    BYTE		ufis[64];

    // 0xA0
    BYTE		rsv[0x100-0xA0];
}  __attribute__((packed)) HBA_FIS;

struct block_device {
    HBA_CMD_HEADER* command_list;
    HBA_CMD_TBL* command_table;
    HBA_FIS* received_fis;
    uint16_t regbase;
    uint8_t port = 0xff;
    uint8_t * receive_buf;
    void on_interrupt();
    void wait_for();
private:
    sleep_lock guard;
    thread* waiting_thread;
    int outstanding_interrupts;
};
#endif //MYOS_AHCI_H
void ahci_on_interrupt();