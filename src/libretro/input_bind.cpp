#include "burner.h"

static bool bInputBindOkay = false;

struct InputBind *InputBindList = NULL;
UINT32 nInputBindListCount = 0;
INT32 nInputBindDIPOffset = 0;
INT32 nInputBindResetOffset = -1;
INT32 nInputBindDiagOffset = -1;

static void InputBindFreeList(struct InputBind *List, INT32 nListCount)
{
	if (List == NULL)
		return;

	for (INT32 i = 0; i < nListCount; i++)
	{
		if (List[i].prid)
			free(List[i].prid);
		List[i].prid = NULL;
	}
	free(List);
}

static void InputBindGetDIPOffset()
{
	BurnDIPInfo bdi;
	nInputBindDIPOffset = 0;

	for (INT32 i = 0; BurnDrvGetDIPInfo(&bdi, i) == 0; i++)
	{
		if (bdi.nFlags == 0xF0)
		{
			nInputBindDIPOffset = bdi.nInput;
			break;
		}
	}

	if (InputBindList && nInputBindDIPOffset >= nInputBindListCount)
		nInputBindDIPOffset = -1;

	if (nInputBindDIPOffset < 0)
		log_cb(RETRO_LOG_INFO, "Get dipswitch input offset failed!\n");
	else
		log_cb(RETRO_LOG_INFO, "Get dipswitch input offset: %d\n", nInputBindDIPOffset);
}

static void InputBindGetResetOffset()
{
	BurnInputInfo bii;
	nInputBindResetOffset = -1;

	for (INT32 i = 0; BurnDrvGetInputInfo(&bii, i) == 0; i++)
	{
		if (strcmp(bii.szInfo, "reset") == 0)
		{
			nInputBindResetOffset = i;
			break;
		}
	}

	if (InputBindList && nInputBindResetOffset >= nInputBindListCount)
		nInputBindResetOffset = -1;

	if (nInputBindResetOffset < 0)
		log_cb(RETRO_LOG_INFO, "Get reset input offset failed!\n");
	else
		log_cb(RETRO_LOG_INFO, "Get reset input offset: %d\n", nInputBindResetOffset);
}

static void InputBindGetDiagOffset()
{
	BurnInputInfo bii;
	nInputBindDiagOffset = -1;

	for (INT32 i = 0; BurnDrvGetInputInfo(&bii, i) == 0; i++)
	{
		if (strcmp(bii.szInfo, "diag") == 0)
		{
			nInputBindDiagOffset = i;
			break;
		}
	}

	if (InputBindList && nInputBindDiagOffset >= nInputBindListCount)
		nInputBindDiagOffset = -1;

	if (nInputBindDiagOffset < 0)
		log_cb(RETRO_LOG_INFO, "Get diagnostic input offset failed!\n");
	else
		log_cb(RETRO_LOG_INFO, "Get diagnostic input offset: %d\n", nInputBindDiagOffset);
}

static INT32 InputBindDIPsInit()
{
	struct BurnDIPInfo bdi;
	struct InputBind *pib;
	INT32 nOffset;

	if (!InputBindList || nInputBindDIPOffset < 0)
		return 1;

	for (INT32 i = 0; BurnDrvGetDIPInfo(&bdi, i) == 0; i++)
	{
		// Set default value
		if (bdi.nFlags == 0xFF)
		{
			nOffset = nInputBindDIPOffset + bdi.nInput;
			if (nOffset < nInputBindListCount)
			{
				pib = InputBindList + nOffset;
				pib->nVal = pib->nDefVal = bdi.nSetting;
				*(pib->bii.pVal) = pib->nVal;
			}
		}
	}

	return 0;
}

static INT32 InputBindAutoOne(struct InputBind *pib)
{
	struct retro_input_descriptor *prid;
	const char *szName;
	const char *szInfo;
	const char *szInfoBase;

	szName = pib->bii.szName;
	szInfo = pib->bii.szInfo;

	bool bPlayerInInfo = (toupper(szInfo[0]) == 'P' && szInfo[1] >= '1' && szInfo[1] <= '5'); // Because some of the older drivers don't use the standard input naming.
	bool bPlayerInName = (toupper(szName[0]) == 'P' && szName[1] >= '1' && szName[1] <= '5');

	if (bPlayerInInfo || bPlayerInName)
	{
		if (pib->prid)
			free(pib->prid);
		pib->prid = (struct retro_input_descriptor *)malloc(sizeof(struct retro_input_descriptor));
		if (pib->prid == NULL)
			return 1;
		memset(pib->prid, 0, sizeof(struct retro_input_descriptor));

		prid = pib->prid;
		szInfoBase = szInfo + 3;

		prid->description = szName;
		prid->port = 0;

		if (bPlayerInName)
			prid->port = szName[1] - '1';
		else if (bPlayerInInfo)
			prid->port = szInfo[1] - '1';

		if (prid->port < 0 || prid->port >= nMaxPlayers)
			prid->port = 0;

		if (strncmp("coin", szInfoBase, 4) == 0)
		{
			prid->device = RETRO_DEVICE_JOYPAD;
			prid->index = 0;
			prid->id = RETRO_DEVICE_ID_JOYPAD_SELECT;
			return 0;
		}
		if (strncmp("select", szInfoBase, 6) == 0)
		{
			prid->device = RETRO_DEVICE_JOYPAD;
			prid->index = 0;
			prid->id = RETRO_DEVICE_ID_JOYPAD_L3;
			return 0;
		}
		if (strncmp("start", szInfoBase, 5) == 0)
		{
			prid->device = RETRO_DEVICE_JOYPAD;
			prid->index = 0;
			prid->id = RETRO_DEVICE_ID_JOYPAD_START;
			return 0;
		}
		if (strncmp("up", szInfoBase, 2) == 0)
		{
			prid->device = RETRO_DEVICE_JOYPAD;
			prid->index = 0;
			prid->id = RETRO_DEVICE_ID_JOYPAD_UP;
			return 0;
		}
		if (strncmp("down", szInfoBase, 4) == 0)
		{
			prid->device = RETRO_DEVICE_JOYPAD;
			prid->index = 0;
			prid->id = RETRO_DEVICE_ID_JOYPAD_DOWN;
			return 0;
		}
		if (strncmp("left", szInfoBase, 4) == 0)
		{
			prid->device = RETRO_DEVICE_JOYPAD;
			prid->index = 0;
			prid->id = RETRO_DEVICE_ID_JOYPAD_LEFT;
			return 0;
		}
		if (strncmp("right", szInfoBase, 5) == 0)
		{
			prid->device = RETRO_DEVICE_JOYPAD;
			prid->index = 0;
			prid->id = RETRO_DEVICE_ID_JOYPAD_RIGHT;
			return 0;
		}
		if (strncmp("x-axis", szInfoBase, 6) == 0)
		{
			prid->device = RETRO_DEVICE_ANALOG;
			prid->index = RETRO_DEVICE_INDEX_ANALOG_LEFT;
			prid->id = RETRO_DEVICE_ID_ANALOG_X;
			return 0;
		}
		if (strncmp("y-axis", szInfoBase, 6) == 0)
		{
			prid->device = RETRO_DEVICE_ANALOG;
			prid->index = RETRO_DEVICE_INDEX_ANALOG_LEFT;
			prid->id = RETRO_DEVICE_ID_ANALOG_Y;
			return 0;
		}
		if (strcmp("mouse x-axis", szInfo) == 0)
		{
			prid->device = RETRO_DEVICE_ANALOG;
			prid->index = RETRO_DEVICE_INDEX_ANALOG_LEFT;
			prid->id = RETRO_DEVICE_ID_ANALOG_X;
			return 0;
		}
		if (strcmp("mouse y-axis", szInfo) == 0)
		{
			prid->device = RETRO_DEVICE_ANALOG;
			prid->index = RETRO_DEVICE_INDEX_ANALOG_LEFT;
			prid->id = RETRO_DEVICE_ID_ANALOG_Y;
			return 0;
		}
		if (strcmp("mouse button 1", szInfo) == 0)
		{
			prid->device = RETRO_DEVICE_JOYPAD;
			prid->index = 0;
			prid->id = RETRO_DEVICE_ID_JOYPAD_B;
			return 0;
		}
		if (strcmp("mouse button 2", szInfo) == 0)
		{
			prid->device = RETRO_DEVICE_JOYPAD;
			prid->index = 0;
			prid->id = RETRO_DEVICE_ID_JOYPAD_A;
			return 0;
		}
		if (strncmp("fire ", szInfoBase, 5) == 0)
		{
			INT32 nButton = strtol(szInfoBase + 5, NULL, 10);
			prid->device = RETRO_DEVICE_JOYPAD;
			prid->index = 0;

			switch (nButton)
			{
			case 1:
			{
				prid->id = RETRO_DEVICE_ID_JOYPAD_B;
				break;
			}
			case 2:
			{
				prid->id = RETRO_DEVICE_ID_JOYPAD_A;
				break;
			}
			case 3:
			{
				prid->id = RETRO_DEVICE_ID_JOYPAD_Y;
				break;
			}
			case 4:
			{
				prid->id = RETRO_DEVICE_ID_JOYPAD_X;
				break;
			}
			case 5:
			{
				prid->id = RETRO_DEVICE_ID_JOYPAD_L;
				break;
			}
			case 6:
			{
				prid->id = RETRO_DEVICE_ID_JOYPAD_R;
				break;
			}
			case 7:
			{
				prid->id = RETRO_DEVICE_ID_JOYPAD_L2;
				break;
			}
			case 8:
			{
				prid->id = RETRO_DEVICE_ID_JOYPAD_R2;
				break;
			}
			default:
			{
				prid->device = RETRO_DEVICE_NONE;
				return 1;
			}
			}
			return 0;
		}
		return 1;
	}

	return 1;
}

static INT32 InputBindMapInit()
{
	struct InputBind *pib;
	struct BurnInputInfo *pbii;

	if (InputBindList == NULL)
		return 1;

	for (INT32 i = 0; i < nInputBindListCount; i++)
	{
		memset(&InputBindList[i], 0, sizeof(InputBind));

		pib = &InputBindList[i];
		pbii = &(pib->bii);

		if (BurnDrvGetInputInfo(pbii, i) != 0)
			continue;

		if (pbii->pVal == NULL)
			continue;

		if (pbii->szName == NULL)
			pbii->szName = "";

		if (pbii->szInfo == NULL)
			pbii->szInfo = "";

		if (pbii->nType & BIT_GROUP_CONSTANT)
			pib->nVal = *(pbii->pVal);

		InputBindAutoOne(pib);
	}

	return 0;
}

INT32 InputBindInit()
{
	if (bInputBindOkay)
		InputBindExit();

	nInputBindListCount = 0;
	while (BurnDrvGetInputInfo(NULL, nInputBindListCount) == 0)
		nInputBindListCount++;

	INT32 nSize = nInputBindListCount * sizeof(struct InputBind);
	InputBindList = (struct InputBind *)malloc(nSize);
	if (InputBindList == NULL)
		return 1;
	memset(InputBindList, 0, nSize);

	InputBindGetDIPOffset();
	InputBindGetResetOffset();
	InputBindGetDiagOffset();

	InputBindMapInit();
	InputBindDIPsInit();

	bInputBindOkay = true;

	return 0;
}

INT32 InputBindExit()
{
	if (InputBindList)
		InputBindFreeList(InputBindList, nInputBindListCount);
	InputBindList = NULL;
	nInputBindListCount = 0;
	nInputBindDIPOffset = 0;
	nInputBindResetOffset = -1;
	nInputBindDiagOffset = -1;

	bInputBindOkay = false;

	return 0;
}

INT32 InputBindCleanKeys()
{
	struct InputBind *pib;
	struct BurnInputInfo *pbii;

	if (InputBindList == NULL)
		return 1;

	for (INT32 i = 0; i < nInputBindListCount; i++)
	{
		pib = &InputBindList[i];
		pbii = &(pib->bii);

		if (pbii->pVal== NULL)
			continue;

		if (pbii->nType == BIT_DIGITAL)
		{
			pib->nVal = 0;
			*(pbii->pVal) = pib->nVal;
		}
	}

	return 0;
}
