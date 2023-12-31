#include "burnint.h"
#include "m68000_intf.h"
#include "z80_intf.h"
#include "nec_intf.h"
#include "sh2_intf.h"
/*
#include "m6502_intf.h"
#include "m6809_intf.h"
#include "hd6309_intf.h"
#include "m6800_intf.h"
#include "s2650_intf.h"
*/
// A hiscore.dat support module for FBA - written by Treble Winner, Feb 2009
// At some point we really need a CPU interface to track CPU types and numbers,
// to make this module and the cheat engine foolproof

#define MAX_CONFIG_LINE_SIZE 		48

#define HISCORE_MAX_RANGES		20

UINT32 nHiscoreNumRanges;

#define APPLIED_STATE_NONE		0
#define APPLIED_STATE_ATTEMPTED		1
#define APPLIED_STATE_CONFIRMED		2

struct _HiscoreMemRange
{
	UINT32 Loaded, nCpu, Address, NumBytes, StartValue, EndValue, ApplyNextFrame, Applied;
	UINT8 *Data;
};

_HiscoreMemRange HiscoreMemRange[HISCORE_MAX_RANGES];

INT32 EnableHiscores;
static INT32 HiscoresInUse;
static INT32 WriteCheck1;

static INT32 nCpuType;
extern INT32 nSekCount;

static void set_cpu_type()
{
	if (nSekCount > -1)
	{
		nCpuType = 1;			// Motorola 68000
	}
	else if (nVezCount)
	{
		nCpuType = 2;			// NEC V30 / V33
	}
	else if (has_sh2)
	{
		nCpuType = 3;			// Hitachi SH2
	}
/*
	else if (nHD6309Count)
	{
		nCpuType = 7;			// HD6309
	}
	else if (nM6809Count)
	{
		nCpuType = 6;			// M6809
	}
	else if (nHasZet > -1)
	{
		nCpuType = 5;			// Zilog Z80
	}
	else if (nM6502Count)
	{
		nCpuType = 4;			// MOS 6502
	}
	else if (nM6800Count)
	{
		nCpuType = 8;			// M6800 & Family
	}
	else if (s2650Count)
	{
		nCpuType = 9;			// S2650
	}
*/
	else
	{
		nCpuType = 0;			// Unknown (don't use cheats)
	}
}

static void cpu_open(INT32 nCpu)
{
	switch (nCpuType)
	{
		case 1:	
			SekOpen(nCpu);
		break;

		case 2:	
			VezOpen(nCpu);	
		break;

		case 3:	
			Sh2Open(nCpu);
		break;
/*
		case 4:
			M6502Open(nCpu);
		break;

		case 5:
			ZetOpen(nCpu);
		break;
			
		case 6:
			M6809Open(nCpu);
		break;
			
		case 7:
			HD6309Open(nCpu);
		break;
		
		case 8:
		break;
		
		case 9: 
			s2650Open(nCpu);
		break;
*/
	}
}

static void cpu_close()
{
	switch (nCpuType)
	{
		case 1:
			SekClose();
		break;

		case 2:
			VezClose();	
		break;

		case 3:
			Sh2Close();
		break;
/*
		case 4:
			M6502Close();
		break;

		case 5:
			ZetClose();
		break;
		
		case 6:
			M6809Close();
		break;
		
		case 7:
			HD6309Close();
		break;
		
		case 8:
		break;
		
		case 9:
			s2650Close();
		break;
*/
	}
}

/*static INT32 cpu_get_active()
{
	switch (nCpuType) {
		case 1: {
			return SekGetActive();
		}
		
		case 2: {
			return VezGetActive();
		}
		
		case 3: {
			return Sh2GetActive();
		}
		
		case 4: {
			return m6502GetActive();
		}
		
		case 5: {
			return ZetGetActive();
		}
		
		case 6: {
			return M6809GetActive();
		}
		
		case 7: {
			return HD6309GetActive();
		}
		
		case 8: {
			return -1;
		}
		
		case 9: {
			return nActiveS2650;
		}
	}
}
*/
static UINT8 cpu_read_byte(UINT32 a)
{
	switch (nCpuType)
	{
		case 1:
			return SekReadByte(a);

		case 2:
			return cpu_readmem20(a);	

		case 3:
			return Sh2ReadByte(a);
/*
		case 4:
			return M6502ReadByte(a);

		case 5:
			return ZetReadByte(a);
			
		case 6:
			return M6809ReadByte(a);
		
		case 7:
			return HD6309ReadByte(a);
			
		case 8:
			return M6800ReadByte(a);
			
		case 9:
			return s2650Read(a);
*/
	}

	return 0;
}

static void cpu_write_byte(UINT32 a, UINT8 d)
{
	switch (nCpuType)
	{
		case 1:
			SekWriteByteROM(a, d);
		break;

		case 2:
			cpu_writemem20(a, d);
		break;

		case 3:
			Sh2WriteByte(a, d);
		break;
/*
		case 4:
			M6502WriteByte(a, d);
		break;

		case 5:
			ZetWriteByte(a, d);
		break;
		
		case 6:
			M6809WriteByte(a, d);
		break;
		
		case 7:
			HD6309WriteByte(a, d);
		break;
		
		case 8:
			M6800WriteByte(a, d);
		break;
		
		case 9:
			s2650Write(a, d);
		break;
*/
	}
}

static UINT32 hexstr2num (const char **pString)
{
	const char *string = *pString;
	UINT32 result = 0;
	if (string)
	{
		for(;;)
		{
			char c = *string++;
			INT32 digit;

			if (c>='0' && c<='9')
			{
				digit = c-'0';
			}
			else if (c>='a' && c<='f')
			{
				digit = 10+c-'a';
			}
			else if (c>='A' && c<='F')
			{
				digit = 10+c-'A';
			}
			else
			{
				if (!c) string = NULL;
				break;
			}
			result = result*16 + digit;
		}
		*pString = string;
	}
	return result;
}

static INT32 is_mem_range (const char *pBuf)
{
	char c;
	for(;;)
	{
		c = *pBuf++;
		if (c == 0) return 0; /* premature EOL */
		if (c == ':') break;
	}
	c = *pBuf; /* character following first ':' */

	return	(c>='0' && c<='9') ||
			(c>='a' && c<='f') ||
			(c>='A' && c<='F');
}

static INT32 is_mem_range_new (const char *pBuf)
{
	char c;
	for(;;)
	{
		c = *pBuf++;
		if (c == 0) return 0; /* premature EOL */
		if (c == ':') break;
	}
	c = *pBuf; /* character following first ':' */
	
	// cpu id - ignore for now
	for(;;)
	{
		c = *pBuf++;
		if (c == 0) return 0; /* premature EOL */
		if (c == ',') break;
	}
	c = *pBuf; /* character following first ',' */
	
	// address space - ignore for now
	for(;;)
	{
		c = *pBuf++;
		if (c == 0) return 0; /* premature EOL */
		if (c == ',') break;
	}
	c = *pBuf; /* character following second ',' */
	
	return	(c>='0' && c<='9') ||
			(c>='a' && c<='f') ||
			(c>='A' && c<='F');
}

static INT32 cpustr2num(char *pCpu)
{ // all known versions of the first cpu as of May 15, 2017
	return (strstr(pCpu, "maincpu") ||
		    strstr(pCpu, "cpu1") ||
			strstr(pCpu, "master") ||
			strstr(pCpu, "fgcpu") ||
			strstr(pCpu, "cpua") ||
			strstr(pCpu, "master_cpu")) ? 0 : 1;
}

static INT32 matching_game_name (const char *pBuf, const char *name)
{
	while (*name)
	{
		if (*name++ != *pBuf++) return 0;
	}
	return (*pBuf == ':');
}

static INT32 CheckHiscoreAllowed()
{
	INT32 Allowed = 1;
	
	if (!EnableHiscores) Allowed = 0;
	if (!(BurnDrvGetFlags() & BDF_HISCORE_SUPPORTED)) Allowed = 0;
	
	return Allowed;
}

void HiscoreInit()
{
	Debug_HiscoreInitted = 1;
	
	if (!CheckHiscoreAllowed()) return;
	
	HiscoresInUse = 0;
	
	TCHAR szDatFilename[MAX_PATH];
	_stprintf(szDatFilename, _T("%shiscore.dat"), szAppHiscorePath);

	FILE *fp = _tfopen(szDatFilename, _T("r"));
	if (fp) {
		char buffer[MAX_CONFIG_LINE_SIZE];
		enum { FIND_NAME, FIND_DATA, FETCH_DATA } mode;
		mode = FIND_NAME;

		while (fgets(buffer, MAX_CONFIG_LINE_SIZE, fp)) {
			if (mode == FIND_NAME) {
				if (matching_game_name(buffer, BurnDrvGetTextA(DRV_NAME))) {
					mode = FIND_DATA;
				}
			} else {
				if (buffer[0] == '@' && buffer[1] == ':') {
					if (is_mem_range_new(buffer)) {
						if (nHiscoreNumRanges < HISCORE_MAX_RANGES) {
							const char *pBuf = buffer;
							char cCpu[80];
							char *pCpu = &cCpu[0];

							// increment pBuf to the address
							char c;
							
							for (;;) {
								c = *pBuf++;
								if (c == ':') break;
							}
							
							c = *pBuf; /* character following first ':' */
	
							// cpu id -> cCpu
							for(;;)
							{
								c = *pBuf++;
								if (c == ',') { *pCpu++ = '\0'; break; }
								else *pCpu++ = c;
							}
							c = *pBuf; /* character following first ',' */
							
							// address space - ignore for now
							for(;;)
							{
								c = *pBuf++;
								if (c == ',') break;
							}
							c = *pBuf; /* character following second ',' */

							// now set the high score data
							HiscoreMemRange[nHiscoreNumRanges].Loaded = 0;
							HiscoreMemRange[nHiscoreNumRanges].nCpu = cpustr2num(&cCpu[0]);
							HiscoreMemRange[nHiscoreNumRanges].Address = hexstr2num(&pBuf);
							HiscoreMemRange[nHiscoreNumRanges].NumBytes = hexstr2num(&pBuf);
							HiscoreMemRange[nHiscoreNumRanges].StartValue = hexstr2num(&pBuf);
							HiscoreMemRange[nHiscoreNumRanges].EndValue = hexstr2num(&pBuf);
							HiscoreMemRange[nHiscoreNumRanges].ApplyNextFrame = 0;
							HiscoreMemRange[nHiscoreNumRanges].Applied = 0;
							HiscoreMemRange[nHiscoreNumRanges].Data = (UINT8*)BurnMalloc(HiscoreMemRange[nHiscoreNumRanges].NumBytes);
							memset(HiscoreMemRange[nHiscoreNumRanges].Data, 0, HiscoreMemRange[nHiscoreNumRanges].NumBytes);
							
#if 1 && defined FBA_DEBUG
							bprintf(PRINT_IMPORTANT, _T("Hi Score Memory Range %i Loaded (New Format) - CPU %i (%S), Address %x, Bytes %02x, Start Val %x, End Val %x\n"), nHiscoreNumRanges, HiscoreMemRange[nHiscoreNumRanges].nCpu, cCpu, HiscoreMemRange[nHiscoreNumRanges].Address, HiscoreMemRange[nHiscoreNumRanges].NumBytes, HiscoreMemRange[nHiscoreNumRanges].StartValue, HiscoreMemRange[nHiscoreNumRanges].EndValue);
#endif
							
							nHiscoreNumRanges++;
							
							mode = FETCH_DATA;
						} else {
							break;
						}
					} else {
						if (mode == FETCH_DATA) break;
					}
				} else {
					if (is_mem_range(buffer)) {
						if (nHiscoreNumRanges < HISCORE_MAX_RANGES) {
							const char *pBuf = buffer;
						
							HiscoreMemRange[nHiscoreNumRanges].Loaded = 0;
							HiscoreMemRange[nHiscoreNumRanges].nCpu = hexstr2num(&pBuf);
							HiscoreMemRange[nHiscoreNumRanges].Address = hexstr2num(&pBuf);
							HiscoreMemRange[nHiscoreNumRanges].NumBytes = hexstr2num(&pBuf);
							HiscoreMemRange[nHiscoreNumRanges].StartValue = hexstr2num(&pBuf);
							HiscoreMemRange[nHiscoreNumRanges].EndValue = hexstr2num(&pBuf);
							HiscoreMemRange[nHiscoreNumRanges].ApplyNextFrame = 0;
							HiscoreMemRange[nHiscoreNumRanges].Applied = 0;
							HiscoreMemRange[nHiscoreNumRanges].Data = (UINT8*)BurnMalloc(HiscoreMemRange[nHiscoreNumRanges].NumBytes);
							memset(HiscoreMemRange[nHiscoreNumRanges].Data, 0, HiscoreMemRange[nHiscoreNumRanges].NumBytes);
						
#if 1 && defined FBA_DEBUG
							bprintf(PRINT_IMPORTANT, _T("Hi Score Memory Range %i Loaded - CPU %i, Address %x, Bytes %02x, Start Val %x, End Val %x\n"), nHiscoreNumRanges, HiscoreMemRange[nHiscoreNumRanges].nCpu, HiscoreMemRange[nHiscoreNumRanges].Address, HiscoreMemRange[nHiscoreNumRanges].NumBytes, HiscoreMemRange[nHiscoreNumRanges].StartValue, HiscoreMemRange[nHiscoreNumRanges].EndValue);
#endif
						
							nHiscoreNumRanges++;
						
							mode = FETCH_DATA;
						} else {
							break;
						}
					} else {
						if (mode == FETCH_DATA) break;
					}
				}
			}
		}
		
		fclose(fp);
	}
	
	if (nHiscoreNumRanges) HiscoresInUse = 1;
	
	TCHAR szFilename[MAX_PATH];
#ifndef __LIBRETRO__
 	_stprintf(szFilename, _T("%s%s.hi"), szAppHiscorePath, BurnDrvGetText(DRV_NAME));
#else
	_stprintf(szFilename, _T("%s%s.hi"), szAppEEPROMPath, BurnDrvGetText(DRV_NAME));
#endif

	fp = _tfopen(szFilename, _T("rb"));
	INT32 Offset = 0;
	if (fp) {
		UINT32 nSize = 0;
		
		while (!feof(fp)) {
			fgetc(fp);
			nSize++;
		}
		
		UINT8 *Buffer = (UINT8*)BurnMalloc(nSize);
		fseek(fp, 0, SEEK_SET);

		fread((char *)Buffer, 1, nSize, fp);

		for (UINT32 i = 0; i < nHiscoreNumRanges; i++) {
			for (UINT32 j = 0; j < HiscoreMemRange[i].NumBytes; j++) {
				HiscoreMemRange[i].Data[j] = Buffer[j + Offset];
			}
			Offset += HiscoreMemRange[i].NumBytes;
			
			HiscoreMemRange[i].Loaded = 1;
			
#if 1 && defined FBA_DEBUG
			bprintf(PRINT_IMPORTANT, _T("Hi Score Memory Range %i Loaded from file\n"), i);
#endif
		}
		
		BurnFree(Buffer);

		fclose(fp);
	}

	WriteCheck1 = 0;
	nCpuType = -1;
}

void HiscoreReset()
{
#if defined FBA_DEBUG
	if (!Debug_HiscoreInitted) bprintf(PRINT_ERROR, _T("HiscoreReset called without init\n"));
#endif

	if (!CheckHiscoreAllowed() || !HiscoresInUse) return;
	
	if (nCpuType == -1) set_cpu_type();

	WriteCheck1 = 0;

	for (UINT32 i = 0; i < nHiscoreNumRanges; i++) {
		HiscoreMemRange[i].ApplyNextFrame = 0;
		HiscoreMemRange[i].Applied = APPLIED_STATE_NONE;
		
		/*if (HiscoreMemRange[i].Loaded)*/ {
			cpu_open(HiscoreMemRange[i].nCpu);
			cpu_write_byte(HiscoreMemRange[i].Address, (UINT8)~HiscoreMemRange[i].StartValue);
			if (HiscoreMemRange[i].NumBytes > 1) cpu_write_byte(HiscoreMemRange[i].Address + HiscoreMemRange[i].NumBytes - 1, (UINT8)~HiscoreMemRange[i].EndValue);
			cpu_close();
			
#if 1 && defined FBA_DEBUG
			bprintf(PRINT_IMPORTANT, _T("Hi Score Memory Range %i Initted\n"), i);
#endif
		}
	}
}

INT32 HiscoreOkToWrite()
{ // Check if it is ok to write the hiscore data aka. did we apply it at least?
	INT32 Ok = 1;

	for (UINT32 i = 0; i < nHiscoreNumRanges; i++) {
		if (!(HiscoreMemRange[i].Loaded && HiscoreMemRange[i].Applied == APPLIED_STATE_CONFIRMED)) {
			Ok = 0;
		}
	}

#if 1 && defined FBA_DEBUG
	bprintf(0, _T("Hiscore Write-Check #1 - Applied data: %X\n"), Ok);
#endif

	if (Ok)
		return 1; // Passed check #1 - already applied hiscore?

	// Check #2 - didn't apply high score, but verified the memory locations
#if 1 && defined FBA_DEBUG
	bprintf(0, _T("Hiscore Write-Check #2 - Memory verified: %X\n"), WriteCheck1);
#endif

	return WriteCheck1;
}

INT32 HiscoreOkToApplyAll()
{ // All of the memory locations in the game's entry must be verfied, then applied when they _all_ match up
	INT32 Ok = 1;

	for (UINT32 i = 0; i < nHiscoreNumRanges; i++) {
		if (!(HiscoreMemRange[i].Loaded && HiscoreMemRange[i].Applied == APPLIED_STATE_NONE && HiscoreMemRange[i].ApplyNextFrame)) {
			Ok = 0;
		}
	}

	return Ok;
}

void HiscoreApply()
{
#if defined FBA_DEBUG
	if (!Debug_HiscoreInitted) bprintf(PRINT_ERROR, _T("HiscoreApply called without init\n"));
#endif

	if (!CheckHiscoreAllowed() || !HiscoresInUse) return;

	if (nCpuType == -1) set_cpu_type();

	UINT8 WriteCheckOk = 0;
	
	for (UINT32 i = 0; i < nHiscoreNumRanges; i++) {
		if (HiscoreMemRange[i].Loaded && HiscoreMemRange[i].Applied == APPLIED_STATE_ATTEMPTED) {
			INT32 Confirmed = 1;
			cpu_open(HiscoreMemRange[i].nCpu);
			for (UINT32 j = 0; j < HiscoreMemRange[i].NumBytes; j++) {
				if (cpu_read_byte(HiscoreMemRange[i].Address + j) != HiscoreMemRange[i].Data[j]) {
					Confirmed = 0;
				}
			}
			cpu_close();
			
			if (Confirmed == 1) {
				HiscoreMemRange[i].Applied = APPLIED_STATE_CONFIRMED;
#if 1 && defined FBA_DEBUG
				bprintf(PRINT_IMPORTANT, _T("Applied Hi Score Memory Range %i on frame number %i\n"), i, GetCurrentFrame());
#endif
			} else {
				HiscoreMemRange[i].Applied = APPLIED_STATE_NONE;
				HiscoreMemRange[i].ApplyNextFrame = 1;
#if 1 && defined FBA_DEBUG
				bprintf(PRINT_IMPORTANT, _T("Failed attempt to apply Hi Score Memory Range %i on frame number %i\n"), i, GetCurrentFrame());
#endif
			}
		}
		
		if (HiscoreMemRange[i].Loaded && HiscoreMemRange[i].Applied == APPLIED_STATE_NONE) {
			cpu_open(HiscoreMemRange[i].nCpu);
			if (cpu_read_byte(HiscoreMemRange[i].Address) == HiscoreMemRange[i].StartValue && cpu_read_byte(HiscoreMemRange[i].Address + HiscoreMemRange[i].NumBytes - 1) == HiscoreMemRange[i].EndValue) {
				HiscoreMemRange[i].ApplyNextFrame = 1;
			}
			cpu_close();
		}

		if (!HiscoreMemRange[i].Loaded && !WriteCheck1) {
			cpu_open(HiscoreMemRange[i].nCpu);
			if (cpu_read_byte(HiscoreMemRange[i].Address) == HiscoreMemRange[i].StartValue && cpu_read_byte(HiscoreMemRange[i].Address + HiscoreMemRange[i].NumBytes - 1) == HiscoreMemRange[i].EndValue) {
				WriteCheckOk++;
			}
			cpu_close();
		}
	}

	if (WriteCheckOk == nHiscoreNumRanges) {
#if 1 && defined FBA_DEBUG
		bprintf(0, _T("Memory Verified - OK to write Hiscore data!\n"));
#endif
		WriteCheck1 = 1; // It's OK to write hi-score data for the first time.
	}

	if (HiscoreOkToApplyAll()) {
		for (UINT32 i = 0; i < nHiscoreNumRanges; i++) {
			cpu_open(HiscoreMemRange[i].nCpu);
			for (UINT32 j = 0; j < HiscoreMemRange[i].NumBytes; j++) {
				cpu_write_byte(HiscoreMemRange[i].Address + j, HiscoreMemRange[i].Data[j]);				
			}
			cpu_close();
			
			HiscoreMemRange[i].Applied = APPLIED_STATE_ATTEMPTED;
			HiscoreMemRange[i].ApplyNextFrame = 0;
		}
	}

}

void HiscoreExit()
{
#if defined FBA_DEBUG
	if (!Debug_HiscoreInitted) bprintf(PRINT_ERROR, _T("HiscoreExit called without init\n"));
#endif

	if (!CheckHiscoreAllowed() || !HiscoresInUse) {
		Debug_HiscoreInitted = 0;
		return;
	}

	if (HiscoreOkToWrite()) {
		if (nCpuType == -1) set_cpu_type();

		TCHAR szFilename[MAX_PATH];
#ifndef __LIBRETRO__
 		_stprintf(szFilename, _T("%s%s.hi"), szAppHiscorePath, BurnDrvGetText(DRV_NAME));
#else
		_stprintf(szFilename, _T("%s%s.hi"), szAppEEPROMPath, BurnDrvGetText(DRV_NAME));
#endif

		FILE *fp = _tfopen(szFilename, _T("wb"));
		if (fp) {
			for (UINT32 i = 0; i < nHiscoreNumRanges; i++) {
				UINT8 *Buffer = (UINT8*)BurnMalloc(HiscoreMemRange[i].NumBytes + 10);
				memset(Buffer, 0, HiscoreMemRange[i].NumBytes + 10);

				cpu_open(HiscoreMemRange[i].nCpu);
				for (UINT32 j = 0; j < HiscoreMemRange[i].NumBytes; j++) {
					Buffer[j] = cpu_read_byte(HiscoreMemRange[i].Address + j);
				}
				cpu_close();

				fwrite(Buffer, 1, HiscoreMemRange[i].NumBytes, fp);

				BurnFree(Buffer);
			}
			fclose(fp);
		}
	} else {
#if 1 && defined FBA_DEBUG
		bprintf(0, _T("HiscoreExit(): -NOT- ok to write Hiscore data!\n"));
#endif
	}

	nCpuType = -1;
	nHiscoreNumRanges = 0;
	WriteCheck1 = 0;

	for (UINT32 i = 0; i < HISCORE_MAX_RANGES; i++) {
		HiscoreMemRange[i].Loaded = 0;
		HiscoreMemRange[i].nCpu = 0;
		HiscoreMemRange[i].Address = 0;
		HiscoreMemRange[i].NumBytes = 0;
		HiscoreMemRange[i].StartValue = 0;
		HiscoreMemRange[i].EndValue = 0;
		HiscoreMemRange[i].ApplyNextFrame = 0;
		HiscoreMemRange[i].Applied = 0;

		BurnFree(HiscoreMemRange[i].Data);
	}

	Debug_HiscoreInitted = 0;
}
