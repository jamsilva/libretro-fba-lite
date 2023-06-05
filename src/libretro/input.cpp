#include "burner.h"

#define RETROPAD_CLASSIC RETRO_DEVICE_ANALOG
#define RETROPAD_MODERN RETRO_DEVICE_SUBCLASS(RETRO_DEVICE_ANALOG, 1)
#define RETROMOUSE_BALL RETRO_DEVICE_MOUSE
#define RETROMOUSE_FULL RETRO_DEVICE_SUBCLASS(RETRO_DEVICE_MOUSE, 1)

static bool bInputOkay = false;

UINT32 nDiagKeyActivateHoldCount = 1;
static UINT32 nDiagKeyCurrentHoldCount = 0;

static void SetControllerInfo()
{
	static const struct retro_controller_description controller_description[] = {
		{"Classic", RETROPAD_CLASSIC},
		{"Modern", RETROPAD_MODERN},
		{"Mouse (ball only)", RETROMOUSE_BALL},
		{"Mouse (full)", RETROMOUSE_FULL},
		{"Pointer", RETRO_DEVICE_POINTER},
		{"Lightgun", RETRO_DEVICE_LIGHTGUN},
	};
	INT32 num_controller_description = sizeof(controller_description) / sizeof(controller_description[0]);

	if (nMaxPlayers <= 0)
		return;

	struct retro_controller_info *controller_infos = (struct retro_controller_info *)calloc(nMaxPlayers + 1, sizeof(struct retro_controller_info));
	if (!controller_infos)
		return;

	for (int i = 0; i < nMaxPlayers; i++)
	{
		controller_infos[i].types = controller_description;
		controller_infos[i].num_types = num_controller_description;
	}
	controller_infos[nMaxPlayers].types = NULL;
	controller_infos[nMaxPlayers].num_types = 0;

	environ_cb(RETRO_ENVIRONMENT_SET_CONTROLLER_INFO, controller_infos);
	free(controller_infos);
}

static void SetInputDescriptors()
{
	INT32 nDescriptorsCount = 0;
	INT32 nDescriptorsIndex = 0;

	for (INT32 i = 0; i < nInputBindListCount; i++)
	{
		if (InputBindList[i].prid)
			nDescriptorsCount++;
	}

	if (nDescriptorsCount == 0)
		return;

	struct retro_input_descriptor *input_descriptors = (struct retro_input_descriptor *)calloc(nDescriptorsCount + 1, sizeof(struct retro_input_descriptor));
	if (!input_descriptors)
		return;

	for (INT32 i = 0; i < nInputBindListCount; i++)
	{
		if (InputBindList[i].prid && nDescriptorsIndex < nDescriptorsCount)
		{
			input_descriptors[nDescriptorsIndex] = *InputBindList[i].prid;
			nDescriptorsIndex++;
		}
	}
	input_descriptors[nDescriptorsIndex].description = NULL;

	environ_cb(RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS, input_descriptors);
	free(input_descriptors);
}

static bool InputStateDiagActivated()
{
	bool bPressed;

	if (InputBindList == NULL || nInputBindDiagOffset < 0 || !pDiagActivatedKey)
		return false;

	bPressed = true;

	for (INT32 i = 0; pDiagActivatedKey[i] != RETRO_DEVICE_ID_JOYPAD_EMPTY; i++)
	{
		if (input_state_cb(0, RETRO_DEVICE_JOYPAD, 0, pDiagActivatedKey[i]) == 0)
		{
			bPressed = false;
			break;
		}
	}

	if (bPressed)
		nDiagKeyCurrentHoldCount++;
	else
		nDiagKeyCurrentHoldCount = 0;

	if (nDiagKeyCurrentHoldCount == nDiagKeyActivateHoldCount)
	{
		struct InputBind *pib = InputBindList + nInputBindDiagOffset;
		struct BurnInputInfo *pbii = &(pib->bii);
		pib->nVal = 1;
		*(pbii->pVal) = pib->nVal;
		BurnDrvFrame();
		InputBindCleanKeys();
		return true;
	}

	return false;
}

void BurnInputInit()
{
	if (bInputOkay)
		BurnInputExit();

	nDiagKeyCurrentHoldCount = 0;

	InputBindInit();
	SetControllerInfo();
	SetInputDescriptors();

	bInputOkay = true;
}

void BurnInputExit()
{
	InputBindExit();
	bInputOkay = false;
}

void BurnInputFrame()
{
	struct InputBind *pib;
	struct BurnInputInfo *pbii;
	struct retro_input_descriptor *prid;
	bool bPressed;

	input_poll_cb();

	if (InputStateDiagActivated())
		return;

	for (INT32 i = 0; i < nInputBindListCount; i++)
	{
		pib = &InputBindList[i];
		pbii = &(pib->bii);
		prid = pib->prid;
		if (!prid || !pbii->pVal)
			continue;

		bPressed = (input_state_cb(prid->port, prid->device, prid->index, prid->id) != 0);

		if (pbii->nType == BIT_DIGITAL)
		{
			pib->nVal = bPressed ? 1 : 0;
			*(pbii->pVal) = pib->nVal;
		}
	}
}

void retro_set_controller_port_device(unsigned port, unsigned device)
{
}
