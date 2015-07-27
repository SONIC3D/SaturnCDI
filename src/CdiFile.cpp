/*  CdiFile.cpp, 2015/07/28

    Copyright (C) 2015 Lu ZongJing <sonic3d{at}gmail.com>   
  
    Please check license file in the root folder for detail license info.                 
*/
#include "CdiFile.h"
#include <memory.h>
#include <malloc.h>
#include <string.h>
#include "EDCECCTool.h"


CdiFile::CdiFile(const char* inputCdiFilename)
:m_pFd(NULL)
,m_lFileSize(0)
,m_lCdiHeaderOffset(0)
// ,edc_lut(NULL)
// ,ecc_b_lut(NULL)
// ,ecc_f_lut(NULL)
{
	ECCEDCInit();

	memset(&m_stCdiToc, 0, sizeof(stCdiToc));

	m_pFd = fopen(inputCdiFilename, "r+b");
	updateFileSize();
	m_lCdiHeaderOffset = this->getFileSize() - getCdiHeaderSize();

	// Fill TOC structure
	unsigned short swSessionCount;
	fseek(m_pFd, m_lCdiHeaderOffset, SEEK_SET);
	fread(&swSessionCount, sizeof(unsigned short), 1, m_pFd);
	m_stCdiToc.dwSessionCount = swSessionCount;
	m_stCdiToc.pstSession = (stCdiSession*)malloc(swSessionCount*sizeof(stCdiSession));

	if (m_stCdiToc.dwSessionCount > 0)
	{
		// Fill Session 1 info only
		unsigned short swTrackCount;
		fread(&swTrackCount, sizeof(unsigned short), 1, m_pFd);	// May not be correct here.The word data in this offset may represent the overall track count of the disc(include all sessions).But as Sega Saturn disc contains only 1 session,it would not cause any problem I think...( x_x...
		m_stCdiToc.pstSession[0].dwTrackCount = swTrackCount;
		m_stCdiToc.pstSession[0].pstTrack = (stCdiTrack*)malloc(swTrackCount*sizeof(stCdiTrack));

		if (m_stCdiToc.pstSession[0].dwTrackCount > 0)
		{
			// Fill Track 01 info only
			fseek(m_pFd, 4, SEEK_CUR);		// Skip 4 unknown bytes
			fseek(m_pFd, 20, SEEK_CUR);		// Skip 20 bytes for Track start mark
			fseek(m_pFd, 4, SEEK_CUR);		// Skip 4 bytes for 3 unknown bytes and 1 byte for number of tracks in current track's container session.
			byte lenFilenameStr;
			fread(&lenFilenameStr, sizeof(byte), 1, m_pFd);
			fseek(m_pFd, lenFilenameStr+1, SEEK_CUR);	// Skip filename string with additional 1 byte for '\0'
			fseek(m_pFd, 22, SEEK_CUR);		// Skip 22 unknown bytes
			fseek(m_pFd, 4, SEEK_CUR);		// Skip 4 bytes block size(Not sector size,the block size is specified in DiscJuggler Advanced tab,default is 360000,which equals 0x057E40 or 80min or 700MB.)
			fseek(m_pFd, 6, SEEK_CUR);		// Skip 6 unknown bytes
			fseek(m_pFd, 8, SEEK_CUR);		// Skip 4 bytes for PreGap's sectors count of this track and 4 bytes for the track length (in unit of sector)
			fseek(m_pFd, 18, SEEK_CUR);		// Skip 18 unknown bytes
			fseek(m_pFd, 4, SEEK_CUR);		// Skip 4 bytes for the Track Index(0 for the first track)
			fseek(m_pFd, 4, SEEK_CUR);		// Skip 4 bytes for End sector of previous Track + 150(This value is 0 for the first track)
			fseek(m_pFd, 4, SEEK_CUR);		// Skip 4 bytes for the Track sector len + PreGap sector len(For the first track(data track),pregap is 150,and for the first audio track after the data track,the pregap is 150,for audio track after other audio track,the pregap is 0)
			fseek(m_pFd, 16, SEEK_CUR);		// Skip 16 unknown bytes
			unsigned int nTrackSectorTypeId;
			fread(&nTrackSectorTypeId, sizeof(unsigned int), 1, m_pFd);
			switch (nTrackSectorTypeId)
			{
			case 0:
				// 0:2048Data track + 2352Audio track(Uncheck R-W,PQ,RAW Read)
				m_stCdiToc.pstSession[0].pstTrack[0].dwBlockSize = 2048;
				break;
			case 1:
				// 1:2336 per sector(Disc Juggler's pfctoc SDK mentioned this,but I don't know how to switch the option to make a cdi image with 2336 sector size)
				m_stCdiToc.pstSession[0].pstTrack[0].dwBlockSize = 2336;
				break;
			case 2:
				// 2:2352 per sector(Check RAW Read Only)
				m_stCdiToc.pstSession[0].pstTrack[0].dwBlockSize = 2352;
				break;
			case 3:
				// 3:2368 per sector(Check PQ)
				m_stCdiToc.pstSession[0].pstTrack[0].dwBlockSize = 2368;
				break;
			case 4:
				// 4:2448 per sector(Check R-W)
				m_stCdiToc.pstSession[0].pstTrack[0].dwBlockSize = 2448;
				break;
			default:
				m_stCdiToc.pstSession[0].pstTrack[0].dwBlockSize = 0;
				printf("Error:Found unknown sector type!");
				break;
			}
		}
	}
}

CdiFile::~CdiFile(void)
{
	if (m_pFd != NULL)
	{
		fclose(m_pFd);
		m_pFd = NULL;
		m_lFileSize = 0;
	}

	for (DWORD i=0; i<m_stCdiToc.dwSessionCount; i++)
	{
		stCdiSession currCdiSession = m_stCdiToc.pstSession[i];
		for (DWORD j=0; j<currCdiSession.dwTrackCount; j++)
		{
			stCdiTrack currCdiTrack = currCdiSession.pstTrack[j];
		}
		SAFE_FREE(currCdiSession.pstTrack);
	}
	SAFE_FREE(m_stCdiToc.pstSession);

// 	SAFE_DELETE_ARRAY(edc_lut);
// 	SAFE_DELETE_ARRAY(ecc_b_lut);
// 	SAFE_DELETE_ARRAY(ecc_f_lut);
}

/************************** EDC/ECC Feature ******************************/
void CdiFile::ECCEDCInit()
{
	EDCECCTool::eccedc_init();
	return;

// 	SAFE_DELETE_ARRAY(edc_lut);
// 	SAFE_DELETE_ARRAY(ecc_b_lut);
// 	SAFE_DELETE_ARRAY(ecc_f_lut);
// 	edc_lut = new uint[0x100];
// 	ecc_b_lut = new uint[0x100];
// 	ecc_f_lut = new uint[0x100];
// 
// 	uint i = 0;
// 	uint j = 0;
// 	uint edc = 0;
// 	for (i=0; i<0x100; i++)
// 	{
// 		if ((i & 0x80) != 0)
// 			j = (i << 1) ^ ((uint) 0x11d);
// 		else
// 			j = (i << 1) ^ ((uint) 0);
// 
// 		ecc_f_lut[i] = j;
// 		ecc_b_lut[i ^ j] = i;
// 
// 		edc = i;
// 		for (j = 0; j <8; j ++)
// 		{
// 			if ((edc & 1) != 0)
// 				edc = (edc >> 1) ^ ((uint) 0xd8018001);
// 			else
// 				edc = (edc >> 1) ^ ((uint) 0);
// 		}
// 		edc_lut[i] = edc;
// 	}
}

void CdiFile::EDCgenerate(byte* sector, byte* dest)
{
	EDCECCTool::edc_computeblock(sector + 0x00, 0x810, dest);
	return;

// 	long edc = 0;
// 	int index = 0;
// 	for (long i = 0x810; i > 0; i -= 1)
// 	{
// 		edc = (edc >> 8) ^ edc_lut[(int) ((edc ^ sector[index]) & ((ulong) 0xffL))];
// 		index++;
// 	}
// 	dest[0] = (edc >> 0) & 0xFF;
// 	dest[1] = (edc >> 8) & 0xFF;
// 	dest[2] = (edc >> 16) & 0xFF;
// 	dest[3] = (edc >> 24) & 0xFF;
}

void CdiFile::ECCgenerate(byte* sector, char ECCtype, byte* dest)
{
	long major_count = 0L;
	long minor_count = 0L;
	long major_mult = 0L;
	long minor_inc = 0L;
	byte lenResultBuf = 0;

	if ((ECCtype == 'P') || (ECCtype == 'p'))	// ECCtype "P"
	{
		major_count = 0x56;
		minor_count = 0x18;
		major_mult = 0x2;
		minor_inc = 0x56;
		lenResultBuf = 172;
	}
	else										// ECCtype "Q"
	{
		major_count = 0x34;
		minor_count = 0x2b;
		major_mult = 0x56;
		minor_inc = 0x58;
		lenResultBuf = 104;
	}
	byte* pResultBuf = (byte*)malloc(lenResultBuf*sizeof(byte));

	// Calculate ECC and store in pResultBuf
	EDCECCTool::ecc_computeblock(sector, major_count, minor_count, major_mult, minor_inc, pResultBuf);
	// Another implementation
// 	uint size = major_count * minor_count;
// 	for (uint i = 0; i<major_count-1; i++)
// 	{
// 		uint index = ((i >> 1) * major_mult) + (i & 1);
// 		uint ecc_a = 0;
// 		uint ecc_b = 0;
// 		for (long k = 0; k < minor_count; k++)
// 		{
// 			uint temp = sector[index + 0xC];
// 			index += minor_inc;
// 			if (index >= size)
// 				index -= size;
// 			ecc_a ^= temp;
// 			ecc_b ^= temp;
// 			ecc_a = ecc_f_lut[ecc_a];
// 		}
// 		ecc_a = ecc_b_lut[(int) (ecc_f_lut[ecc_a] ^ ecc_b)];
// 		pResultBuf[i] = (byte) ecc_a;
// 		pResultBuf[i + major_count] = (byte) (ecc_a ^ ecc_b);
// 	}

	memcpy(dest, pResultBuf, lenResultBuf*sizeof(byte));
	SAFE_FREE(pResultBuf);
}

void CdiFile::fixEDCECC_bySector(byte* pSector)
{
	// Fill 4 bytes EDC for sector1
	int nLenEDC = 4;
	byte* bufEDC = (byte*)malloc(nLenEDC*sizeof(byte));
	EDCgenerate(pSector, bufEDC);
	for (int i=0; i<nLenEDC; i++)
		pSector[0x810 + i] = bufEDC[i];
	SAFE_FREE(bufEDC);

	// Fill 172 bytes ECC P for sector1
	int nLenECC_P = 172;
	byte* bufECC_P = (byte*)malloc(nLenECC_P*sizeof(byte));
	ECCgenerate(pSector, 'P', bufECC_P);
	for (int i=0; i<nLenECC_P; i++)
		pSector[0x81C + i] = bufECC_P[i];
	SAFE_FREE(bufECC_P);

	// Fill 104 bytes ECC Q for sector1
	int nLenECC_Q = 104;
	byte* bufECC_Q = (byte*)malloc(nLenECC_Q*sizeof(byte));
	ECCgenerate(pSector, 'Q', bufECC_Q);
	for (int i=0; i<nLenECC_Q; i++)
		pSector[0x8C8 + i] = bufECC_Q[i];
	SAFE_FREE(bufECC_Q);
}

/****************** CDI file format related feature **********************/
unsigned int CdiFile::getCdiHeaderSize()
{
	unsigned int ret = 0;

	if (getFileSize() >= 4)
	{
		fseek(m_pFd, -4, SEEK_END);
		fread(&ret, sizeof(unsigned int), 1, m_pFd);
	}
	return ret;
}

long CdiFile::getFileSize()
{
	return m_lFileSize;
}

void CdiFile::updateFileSize()
{
	if (m_pFd != NULL)
	{
		fseek(m_pFd, 0, SEEK_END);
		m_lFileSize = ftell(m_pFd);
	}
}

stCdiTrack* CdiFile::getTrackById(unsigned int trackId)
{
	// Temporary dirty implementation,only the 1st track is allowed to get //
	if ((trackId == 0)
		&& (m_stCdiToc.dwSessionCount>0) && (m_stCdiToc.pstSession[0].dwTrackCount>0))
	{
		return &(this->m_stCdiToc.pstSession[0].pstTrack[0]);
	}
	else
	{
		return NULL;
	}
}

/************ Sega Saturn disc region patch related feature **************/
// Usage sample: patchSaturnRegion("JTUBKAEL");     // Insert region as much as possible in the provided priority.
//               patchSaturnRegion("UEJ");
void CdiFile::patchSaturnRegion(const char* szRegionInPriority)
{
	// Determine current region support first
	int nSectorSize = 0x990;						// 2448 per sector
	int nSectorsToSkip = 150;						// Skip first 150 sectors(PreGap)
	byte* pSectorBuf = (byte*)malloc(nSectorSize*sizeof(byte));
	fseek(m_pFd, nSectorsToSkip*nSectorSize, SEEK_SET);
	fread(pSectorBuf, sizeof(byte), nSectorSize, m_pFd);

	int nLenRegionCode = 8;
	int nOriginalRegionCount = 0;
	for (int i = 0; i < nLenRegionCode; i++)
	{
		if (pSectorBuf[80 + i] != 0x20)
			nOriginalRegionCount++;
	}
	char* bufOriginalRegionStr = (char*)malloc((nOriginalRegionCount+1)*sizeof(char));
	for (int i = 0; i < nOriginalRegionCount; i++)
	{
		bufOriginalRegionStr[i] = pSectorBuf[80 + i];
	}
	bufOriginalRegionStr[nOriginalRegionCount] = '\0';
	SAFE_FREE(pSectorBuf);
	printf("Original Region Count:%d\n", nOriginalRegionCount);
	printf("Original Region String:%s\n", bufOriginalRegionStr);

	/* Modify and apply new region for no more than original count.*/

	// Generate new region string contains as much as user wanted region code
	int nLenRegionInPriority = strlen(szRegionInPriority);
	char* bufNewRegionStr = (char*)malloc((nOriginalRegionCount+1)*sizeof(char));
	if (nLenRegionInPriority>=nOriginalRegionCount)
	{
		// Crop user wanted region string to the length of original region count(Assume there is no dupe region code in szRegionInPriority)
		memcpy(bufNewRegionStr, szRegionInPriority, nOriginalRegionCount);
	}
	else
	{
		// Copy user required region string and add original string without dupe.
		memcpy(bufNewRegionStr, szRegionInPriority, nLenRegionInPriority);
		int nRegionSlotLeft = nOriginalRegionCount-nLenRegionInPriority;
		for (int i = 0; i < nOriginalRegionCount; i++)
		{
			char char2Add = bufOriginalRegionStr[i];
			bool trueFoundDupe = false;
			for (int j=0; j<(nOriginalRegionCount-nRegionSlotLeft); j++)
			{
				if (char2Add == bufNewRegionStr[j])
				{
					trueFoundDupe = true;
					break;
				}
			}

			if (!trueFoundDupe)
			{
				bufNewRegionStr[nOriginalRegionCount-nRegionSlotLeft] = char2Add;
				nRegionSlotLeft--;
			}

			if (nRegionSlotLeft == 0)
				break;
		}
	}
	SAFE_FREE(bufOriginalRegionStr);
	bufNewRegionStr[nOriginalRegionCount] = '\0';
	printf("Patched Region String:%s\n", bufNewRegionStr);

	// Generate region mask based on the new region string.
	byte regionMask = 0;
	for (int i=0; i<nOriginalRegionCount; i++)
	{
		byte addMask;
		switch(bufNewRegionStr[i])
		{
		case 'J':
			addMask = RGN_J;
			break;
		case 'T':
			addMask = RGN_T;
			break;
		case 'U':
			addMask = RGN_U;
			break;
		case 'B':
			addMask = RGN_B;
			break;
		case 'K':
			addMask = RGN_K;
			break;
		case 'A':
			addMask = RGN_A;
			break;
		case 'E':
			addMask = RGN_E;
			break;
		case 'L':
			addMask = RGN_L;
			break;
		}
		regionMask |= addMask;
	}
	SAFE_FREE(bufNewRegionStr);

	// Invoke region patch routine with the final regionMask value
	patchSaturnRegionCode(regionMask);
	patchSaturnAreaString(regionMask);
}

void CdiFile::patchSaturnRegionCode(byte regionMask)
{
	int nSectorSize = 0x990;						// 2448 per sector
	int nSectorsToSkip = 150;						// Skip first 150 sectors(PreGap)
	byte* pSectorBuf = (byte*)malloc(nSectorSize*sizeof(byte));
	fseek(m_pFd, nSectorsToSkip*nSectorSize, SEEK_SET);
	fread(pSectorBuf, sizeof(byte), nSectorSize, m_pFd);

	// Modify Compatible RegionCode
	char bufPossibleRegionCode[8]={'J','T','U','B','K','A','E','L'};

	int nLenRegionCode = 8;
	char* bufRegionCodeSlot = (char*)malloc(nLenRegionCode*sizeof(char));
	for (int i=0; i<nLenRegionCode; i++)
		bufRegionCodeSlot[i] = 0x20;				// space char as default value
	int offsetRgnCode = 0;
	for (int i=0; i<nLenRegionCode; i++)
	{
		if (regionMask & (RGN_J << i))				// Test regionMask with all possible region mask value in order
		{
			bufRegionCodeSlot[offsetRgnCode] = bufPossibleRegionCode[i];
			offsetRgnCode++;
		}
	}
	for (int i = 0; i < nLenRegionCode; i++)
		pSectorBuf[80 + i] = bufRegionCodeSlot[i];	// after 16 bytes Sync and Header Field and 64 bytes of data
	SAFE_FREE(bufRegionCodeSlot);

	fseek(m_pFd, nSectorsToSkip*nSectorSize, SEEK_SET);
	fwrite(pSectorBuf, sizeof(byte), nSectorSize, m_pFd);
	SAFE_FREE(pSectorBuf);
}

void CdiFile::patchSaturnAreaString(byte regionMask)
{
	int nSectorSize = 0x990;						// 2448 per sector
	int nSectorsToSkip = 151;						// Skip first 150 sectors(PreGap) and the first content sector
	byte* pSectorBuf = (byte*)malloc(nSectorSize*sizeof(byte));
	fseek(m_pFd, nSectorsToSkip*nSectorSize, SEEK_SET);
	fread(pSectorBuf, sizeof(byte), nSectorSize, m_pFd);

	// Modify AreaInitCode
	byte bufPossibleAreaInitCode[8][32]={
		{
			0xA0, 0x0E, 0x00, 0x09, 0x46, 0x6F, 0x72, 0x20, 0x4A, 0x41, 0x50, 0x41, 0x4E, 0x2E, 0x20, 0x20, 
			0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20
		},	// J
		{
			0xA0, 0x0E, 0x00, 0x09, 0x46, 0x6F, 0x72, 0x20, 0x54, 0x41, 0x49, 0x57, 0x41, 0x4E, 0x20, 0x61,
			0x6E, 0x64, 0x20, 0x50, 0x48, 0x49, 0x4C, 0x49, 0x50, 0x49, 0x4E, 0x45, 0x53, 0x2E, 0x20, 0x20
		},	// T
		{
			0xA0, 0x0E, 0x00, 0x09, 0x46, 0x6F, 0x72, 0x20, 0x55, 0x53, 0x41, 0x20, 0x61, 0x6E, 0x64, 0x20,
			0x43, 0x41, 0x4E, 0x41, 0x44, 0x41, 0x2E, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20
		},	// U
		{
			0xA0, 0x0E, 0x00, 0x09, 0x46, 0x6F, 0x72, 0x20, 0x42, 0x52, 0x41, 0x5A, 0x49, 0x4C, 0x2E, 0x20, 
			0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20
		},	// B
		{
			0xA0, 0x0E, 0x00, 0x09, 0x46, 0x6F, 0x72, 0x20, 0x4B, 0x4F, 0x52, 0x45, 0x41, 0x2E, 0x20, 0x20, 
			0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20
		},	// K
		{
			0xA0, 0x0E, 0x00, 0x09, 0x46, 0x6F, 0x72, 0x20, 0x41, 0x53, 0x49, 0x41, 0x20, 0x50, 0x41, 0x4C, 
			0x20, 0x61, 0x72, 0x65, 0x61, 0x2E, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20
		},	// A
		{
			0xA0, 0x0E, 0x00, 0x09, 0x46, 0x6F, 0x72, 0x20, 0x45, 0x55, 0x52, 0x4F, 0x50, 0x45, 0x2E, 0x20, 
			0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20
		},	// E
		{
			0xA0, 0x0E, 0x00, 0x09, 0x46, 0x6F, 0x72, 0x20, 0x4C, 0x41, 0x54, 0x49, 0x4E, 0x20, 0x41, 0x4D, 
			0x45, 0x52, 0x49, 0x43, 0x41, 0x2E, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20
		}	// L
	};
	int nLenRegionCode = 8;
	byte* bufAreaInitCodeSlot = (byte*)malloc(nLenRegionCode*32*sizeof(byte));
	for (int i=0; i<nLenRegionCode*32; i++)
		bufAreaInitCodeSlot[i] = 0x20;
	int offsetRgnCode = 0;
	for (int i=0; i<nLenRegionCode; i++)
	{
		if (regionMask & (RGN_J << i))				// Test regionMask with all possible region mask value in order
		{
			memcpy(bufAreaInitCodeSlot+offsetRgnCode*32, &(bufPossibleAreaInitCode[i]), 32);
			offsetRgnCode++;
		}
	}
	int nLenAreaInitCode = offsetRgnCode*32;
	for (int i = 0; i < nLenAreaInitCode; i++)
		pSectorBuf[0x610 + i] = bufAreaInitCodeSlot[i];
	SAFE_FREE(bufAreaInitCodeSlot);

	fseek(m_pFd, nSectorsToSkip*nSectorSize, SEEK_SET);
	fwrite(pSectorBuf, sizeof(byte), nSectorSize, m_pFd);
	SAFE_FREE(pSectorBuf);
}

void CdiFile::patchSaturnDeveloperTier(DeveloperTierType devTierType)
{
	int nSectorSize = 0x990;						// 2448 per sector
	int nSectorsToSkip = 150;						// Skip first 150 sectors(PreGap)
	byte* pSectorBuf = (byte*)malloc(nSectorSize*sizeof(byte));
	fseek(m_pFd, nSectorsToSkip*nSectorSize, SEEK_SET);
	fread(pSectorBuf, sizeof(byte), nSectorSize, m_pFd);

	// Modify DevParty
	int nLenDevTier = 3;
	byte* buf1st3rdParty = (byte*)malloc(nLenDevTier*sizeof(byte));
	switch(devTierType)
	{
	case kDeveloperTier_FirstParty:
		buf1st3rdParty[0] = 0x45;
		buf1st3rdParty[1] = 0x4E;
		buf1st3rdParty[2] = 0x54;
		break;
	default:
		buf1st3rdParty[0] = 0x54;
		buf1st3rdParty[1] = 0x50;
		buf1st3rdParty[2] = 0x20;
		break;
	}
	for (int i=0; i<nLenDevTier; i++)
		pSectorBuf[0x25 + i] = buf1st3rdParty[i];
	SAFE_FREE(buf1st3rdParty);

	fseek(m_pFd, nSectorsToSkip*nSectorSize, SEEK_SET);
	fwrite(pSectorBuf, sizeof(byte), nSectorSize, m_pFd);
	SAFE_FREE(pSectorBuf);
}

void CdiFile::fixSaturnEDCECC()
{
	int nSectorSize = 0x990;						// 2448 per sector
	int nSectorsToSkip = 150;						// Skip first 150 sectors(PreGap)
	byte* pSectorBuf = (byte*)malloc(2*nSectorSize*sizeof(byte));
	fseek(m_pFd, nSectorsToSkip*nSectorSize, SEEK_SET);
	fread(pSectorBuf, sizeof(byte), 2*nSectorSize, m_pFd);

	fixEDCECC_bySector(pSectorBuf);					// fix sector 1 for Region Code and Developer Tier modification
	fixEDCECC_bySector(pSectorBuf + nSectorSize);	// fix sector 2 for Area Init Code modification

	fseek(m_pFd, nSectorsToSkip*nSectorSize, SEEK_SET);
	fwrite(pSectorBuf, sizeof(byte), 2*nSectorSize, m_pFd);
	SAFE_FREE(pSectorBuf);
}
