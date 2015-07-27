/*  EDCECCTool.cpp, 2015/07/28

    Copyright (C) 2015 Lu ZongJing <sonic3d{at}gmail.com>   
  
    Please check license file in the root folder for detail license info.                 
*/
#include "EDCECCTool.h"

EDCECCTool::EDCECCTool(void)
{
}

EDCECCTool::~EDCECCTool(void)
{
}

/* LUTs used for computing ECC and EDC */
BYTE EDCECCTool::ecc_f_lut[256];
BYTE EDCECCTool::ecc_b_lut[256];
DWORD EDCECCTool::edc_lut[256];

/* Init routine */
void EDCECCTool::eccedc_init (void)
{
	DWORD i, j, edc;
	for (i = 0; i <256; i ++)
	{
		j = (i << 1) ^ (i & 0x80? 0x11D: 0);
		ecc_f_lut[i] = j;
		ecc_b_lut[i ^ j] = i;
		edc = i;
		for (j = 0; j <8; j ++)
			edc = (edc >> 1) ^ (edc & 1? 0xD8018001: 0);
		edc_lut [i] = edc;
	}
}

/* Compute EDC for a block */
void EDCECCTool::edc_computeblock(const BYTE* src, WORD size, BYTE* dest)
{
	DWORD edc = 0x00000000;
	while (size--)
		edc = (edc >> 8) ^ edc_lut [(edc ^ (*src++)) & 0xFF];
	dest[0] = (edc >> 0) & 0xFF;
	dest[1] = (edc >> 8) & 0xFF;
	dest[2] = (edc >> 16) & 0xFF;
	dest[3] = (edc >> 24) & 0xFF;
}

/* Compute ECC for a block (can do either P or Q) */
void EDCECCTool::ecc_computeblock (BYTE * src, DWORD major_count, DWORD minor_count, DWORD major_mult, DWORD minor_inc, BYTE * dest)
{
	DWORD size = major_count * minor_count;
	DWORD major, minor;
	for (major = 0; major <major_count; major++)
	{
		DWORD index = (major >> 1) * major_mult + (major & 1);
		BYTE ecc_a = 0;
		BYTE ecc_b = 0;
		for (minor = 0; minor <minor_count; minor ++)
		{
			BYTE temp = src[index+0xC];
			index += minor_inc;
			if (index >= size)
				index -= size;
			ecc_a ^= temp;
			ecc_b ^= temp;
			ecc_a = ecc_f_lut[ecc_a];
		}
		ecc_a = ecc_b_lut[ecc_f_lut[ecc_a] ^ ecc_b];
		dest[major] = ecc_a;
		dest[major + major_count] = ecc_a ^ ecc_b;
	}
}

/* Generate ECC P and Q codes for a block(Not used in saturn disc ecc fix) */
void EDCECCTool::ecc_generate (BYTE * sector, int zeroaddress)
{
	BYTE address[4], i;
	/* Save the address and zero it out */
	if (zeroaddress)
		for (i = 0; i <4; i ++)
		{
			address [i] = sector[0xC + i];
			sector[12 + i] = 0;
		}
	/* Compute ECC P code */
	ecc_computeblock(sector, 0x56, 0x18,  0x2, 0x56, sector + 0x81C);
	/* Compute ECC Q code */
	ecc_computeblock(sector, 0x34, 0x2B, 0x56, 0x58, sector + 0x8C8);
	/* Restore the address */
	if (zeroaddress)
		for (i = 0; i <4; i ++)
			sector [0xC + i] = address [i];
}

/*
   Generate ECC/EDC info for a sector(must be 2352 = 0x930 bytes)
   returns 0 on success
*/
void EDCECCTool::eccedc_generate (BYTE * sector, int type)
{
	DWORD i;
	switch (type)
	{
	case 1:
		/* Mode 1 */
		edc_computeblock(sector + 0x00, 0x810, sector + 0x810);
		/* Write out zero bytes */
		for (i = 0; i <8; i ++)
			sector [0x814 + i] = 0;
		ecc_generate (sector, 0);
		break;
	case 2:
		/* Mode 2 form 1 */
		edc_computeblock(sector + 0x10, 0x808, sector + 0x818);
		ecc_generate (sector, 1);
		break;
	case 3: /* Mode 2 form 2 */
		edc_computeblock(sector + 0x10, 0x91C, sector + 0x92C);
		break;
	}
}
