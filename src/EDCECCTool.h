/*  EDCECCTool.h, 2015/07/28

    Copyright (C) 2015 Lu ZongJing <sonic3d{at}gmail.com>   
  
    Please check license file in the root folder for detail license info.                 
*/
#pragma once

typedef unsigned char BYTE;
typedef unsigned long DWORD;
typedef unsigned short WORD;

class EDCECCTool
{
public:
	EDCECCTool(void);
	virtual ~EDCECCTool(void);

public:
	/* LUTs used for computing ECC / EDC */
	static BYTE ecc_f_lut[256];
	static BYTE ecc_b_lut[256];
	static DWORD edc_lut[256];

	static void eccedc_init (void);
	static void ecc_computeblock (BYTE* src, DWORD major_count, DWORD minor_count, DWORD major_mult, DWORD minor_inc, BYTE* dest);
	static void ecc_generate (BYTE* sector, int zeroaddress);
	static void edc_computeblock(const BYTE* src, WORD size, BYTE* dest);

public:
	void eccedc_generate (BYTE * sector, int type);
};
