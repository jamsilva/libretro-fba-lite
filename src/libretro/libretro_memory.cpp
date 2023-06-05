#include "burner.h"

static void* pMainRamData = NULL;
static size_t nMainRamSize = 0;
static bool bMainRamFound = false;

static int StateGetMainRamAcb(BurnArea *pba)
{
	int nHardwareCode = BurnDrvGetHardwareCode();

	if(!pba->szName)
		return 0;

	// Neogeo / PGM
	if ((nHardwareCode & HARDWARE_PUBLIC_MASK) == HARDWARE_SNK_NEOGEO
	 || (nHardwareCode & HARDWARE_PUBLIC_MASK) & HARDWARE_PREFIX_IGS_PGM) {
		if (strcmp(pba->szName, "68K RAM") == 0) {
			pMainRamData = pba->Data;
			nMainRamSize = pba->nLen;
			bMainRamFound = true;
			return 0;
		}
	}

	// Psikyo (psikyosh and psikyo4 uses "All RAM")
	if ((nHardwareCode & HARDWARE_PUBLIC_MASK) & HARDWARE_PREFIX_PSIKYO) {
		if ((strcmp(pba->szName, "All RAM") == 0) || (strcmp(pba->szName, "68K RAM") == 0)) {
			pMainRamData = pba->Data;
			nMainRamSize = pba->nLen;
			bMainRamFound = true;
			return 0;
		}
	}

	// Cave (gaia driver uses "68K RAM")
	if ((nHardwareCode & HARDWARE_PUBLIC_MASK) & HARDWARE_PREFIX_CAVE) {
		if ((strcmp(pba->szName, "RAM") == 0) || (strcmp(pba->szName, "68K RAM") == 0)) {
			pMainRamData = pba->Data;
			nMainRamSize = pba->nLen;
			bMainRamFound = true;
			return 0;
		}
	}

	// CPS1 / CPS2
	if ((nHardwareCode & HARDWARE_PUBLIC_MASK) & HARDWARE_PREFIX_CAPCOM) {
		if (strcmp(pba->szName, "CpsRamFF") == 0) {
			pMainRamData = pba->Data;
			nMainRamSize = pba->nLen;
			bMainRamFound = true;
			return 0;
		}
	}

	// CPS3
	if ((nHardwareCode & HARDWARE_PUBLIC_MASK) == HARDWARE_CAPCOM_CPS3) {
		if (strcmp(pba->szName, "Main RAM") == 0) {
			pMainRamData = pba->Data;
			nMainRamSize = pba->nLen;
			bMainRamFound = true;
			return 0;
		}
	}

	// For all other systems (?), main ram seems to be identified by either "All Ram" or "All RAM"
	if ((strcmp(pba->szName, "All Ram") == 0) || (strcmp(pba->szName, "All RAM") == 0)) {
		pMainRamData = pba->Data;
		nMainRamSize = pba->nLen;
		bMainRamFound = true;
		return 0;
	}

	return 0;
}

void BurnCheevosInit()
{
	INT32 nMin = 0;
	BurnAcb = StateGetMainRamAcb;
	BurnAreaScan(ACB_FULLSCAN, &nMin);
}

void BurnCheevosExit()
{
	pMainRamData = NULL;
	nMainRamSize = 0;
	bMainRamFound = false;
}

void *retro_get_memory_data(unsigned id)
{
	return id == RETRO_MEMORY_SYSTEM_RAM ? pMainRamData : NULL;
}

size_t retro_get_memory_size(unsigned id)
{
	return id == RETRO_MEMORY_SYSTEM_RAM ? nMainRamSize : 0;
}
