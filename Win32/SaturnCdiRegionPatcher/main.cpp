/*  SaturnCdiRegionPatcher.cpp, 2015/07/28

    Copyright (C) 2015 Lu ZongJing <sonic3d{at}gmail.com>   
  
    Please check license file in the root folder for detail license info.                 
*/
#include <stdio.h>
#include <tchar.h>
#include "../../src/CdiFile.h"

int main(int argc, const char* argv[])
{
	if (argc < 3)
	{
		printf("Usage:\n");
		printf("      SSCdiRP.exe <region string> <cdi image filename>\n");
		printf("Region string can be any combination of JTUBKAEL in any order.\n");
		printf("It's case sensitive and the order represent the priority you want to patch your image.\n");
		printf("\n");
		printf("Cdi image file should be made with the option \"R-W(CD+G)\" checked in Advanced tab of DiscJuggler.\n");
		return -1;
	}
	else
	{
// 		CdiFile cdiFile("test.cdi");
		CdiFile cdiFile(argv[2]);
		stCdiTrack* pFirstTrack = cdiFile.getTrackById(0);
		if (pFirstTrack->dwBlockSize == 2448)
		{
			cdiFile.patchSaturnRegion(argv[1]);
			cdiFile.fixSaturnEDCECC();
		}
		else
		{
			printf("Failed to patch:\nPlease remake the disc image with the option \"R-W(CD+G)\" checked in Advanced tab of DiscJuggler.");
			return -2;
		}

		return 0;
	}
}
