/*  CdiFile.h, 2015/07/28

    Copyright (C) 2015 Lu ZongJing <sonic3d{at}gmail.com>   
  
    Please check license file in the root folder for detail license info.                 
*/
#pragma once
#include <stdio.h>

#define SAFE_DELETE(p)            do { if(p) { delete (p); (p) = 0; } } while(0)
#define SAFE_DELETE_ARRAY(p)     do { if(p) { delete[] (p); (p) = 0; } } while(0)
#define SAFE_FREE(p)                do { if(p) { free(p); (p) = 0; } } while(0)

typedef unsigned long DWORD;
typedef unsigned char byte;
typedef unsigned long ulong;
typedef unsigned int uint;

// Region Code Mask
#define RGN_J 0x01
#define RGN_T (RGN_J<<1) // 0x02
#define RGN_U (RGN_J<<2) // 0x04
#define RGN_B (RGN_J<<3) // 0x08
#define RGN_K (RGN_J<<4) // 0x10
#define RGN_A (RGN_J<<5) // 0x20
#define RGN_E (RGN_J<<6) // 0x40
#define RGN_L (RGN_J<<7) // 0x80

typedef enum enumDeveloperTierType
{
	kDeveloperTier_FirstParty = 0,
	kDeveloperTier_ThirdParty = 1,
} DeveloperTierType;

typedef struct SCdiTrack
{
	DWORD dwBlockSize;		// Sector size in bytes (2048, 2336, 2352, 2368, 2448)
} stCdiTrack;

typedef struct SCdiSession
{
	DWORD dwTrackCount;
	stCdiTrack* pstTrack; 
} stCdiSession;

typedef struct SCdiToc {
 	DWORD dwSessionCount;
	stCdiSession* pstSession;
} stCdiToc;

class CdiFile
{
protected:
	FILE* m_pFd;
	long m_lFileSize;
	long m_lCdiHeaderOffset;
	stCdiToc m_stCdiToc;

private:
// 	uint* edc_lut;
// 	uint* ecc_b_lut;
// 	uint* ecc_f_lut;
	void ECCEDCInit();
	void EDCgenerate(byte* sector, byte* dest);
	void ECCgenerate(byte* sector, char ECCtype, byte* dest);		// parameter ECCtype:'P' or 'p' for ECC type P,other value for ECC type 'Q'
	void fixEDCECC_bySector(byte* pSector);

protected:
	void updateFileSize();
	unsigned int getCdiHeaderSize();

public:
	CdiFile(const char* inputCdiFilename);
	virtual ~CdiFile(void);
public:
	long getFileSize();
	stCdiTrack* getTrackById(unsigned int trackId);

protected:
	void patchSaturnRegionCode(byte regionMask);
	void patchSaturnAreaString(byte regionMask);
	void patchSaturnDeveloperTier(DeveloperTierType devTierType);
public:
	void patchSaturnRegion(const char* szRegionInPriority);
	void fixSaturnEDCECC();
};
