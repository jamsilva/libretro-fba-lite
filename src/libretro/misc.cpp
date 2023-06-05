// Misc functions module
#include "burner.h"

TCHAR *LabelCheck(TCHAR *s, TCHAR *pszLabel)
{
	INT32 nLen;
	if (s == NULL || pszLabel == NULL)
		return NULL;

	nLen = _tcslen(pszLabel);

	SKIP_WS(s); // Skip whitespace

	if (_tcsncmp(s, pszLabel, nLen))
	{ // Doesn't match
		return NULL;
	}
	return s + nLen;
}

INT32 QuoteRead(TCHAR **ppszQuote, TCHAR **ppszEnd, TCHAR *pszSrc) // Read a (quoted) string from szSrc and poINT32 to the end
{
	static TCHAR szQuote[QUOTE_MAX];
	TCHAR *s = pszSrc;
	TCHAR *e;

	// Skip whitespace
	SKIP_WS(s);

	e = s;

	if (*s == _T('\"'))
	{ // Quoted string
		s++;
		e++;
		// Find end quote
		FIND_QT(e);
		_tcsncpy(szQuote, s, e - s);
		// Zero-terminate
		szQuote[e - s] = _T('\0');
		e++;
	}
	else
	{ // Non-quoted string
		// Find whitespace
		FIND_WS(e);
		_tcsncpy(szQuote, s, e - s);
		// Zero-terminate
		szQuote[e - s] = _T('\0');
	}

	if (ppszQuote)
	{
		*ppszQuote = szQuote;
	}
	if (ppszEnd)
	{
		*ppszEnd = e;
	}

	return 0;
}

TCHAR *ExtractFilename(TCHAR *fullname)
{
	TCHAR *filename = fullname + _tcslen(fullname);

	do
	{
		filename--;
	} while (filename >= fullname && *filename != _T('\\') && *filename != _T('/') && *filename != _T(':'));

	return filename;
}

TCHAR *StrReplace(TCHAR *str, TCHAR find, TCHAR replace)
{
	INT32 length = _tcslen(str);

	for (INT32 i = 0; i < length; i++)
	{
		if (str[i] == find)
			str[i] = replace;
	}

	return str;
}

TCHAR *StrLower(TCHAR *str)
{
	static TCHAR szBuffer[256] = _T("");
	INT32 length = _tcslen(str);

	if (length > 255)
		length = 255;

	for (INT32 i = 0; i < length; i++)
	{
		if (str[i] >= _T('A') && str[i] <= _T('Z'))
			szBuffer[i] = (str[i] + _T(' '));
		else
			szBuffer[i] = str[i];
	}
	szBuffer[length] = 0;

	return &szBuffer[0];
}

TCHAR *FileExt(TCHAR *str)
{
	TCHAR *dot = strrchr(str, _T('.'));

	return (dot) ? StrLower(dot) : str;
}

bool IsFileExt(TCHAR *str, TCHAR *ext)
{
	return (_tcsicmp(ext, FileExt(str)) == 0);
}

//------------------------------------------------------------------------------

#ifdef ANDROID
#include <wchar.h>

size_t mbstowcs(wchar_t *pwcs, const char *s, size_t n)
{
	if (pwcs == NULL)
		return strlen(s);
	return mbsrtowcs(pwcs, &s, n, NULL);
}

size_t wcstombs(char *s, const wchar_t *pwcs, size_t n)
{
	return wcsrtombs(s, &pwcs, n, NULL);
}
#endif

char *TCHARToANSI(const TCHAR *pszInString, char *pszOutString, int nOutSize)
{
	if (pszOutString)
	{
		strcpy(pszOutString, pszInString);
		return pszOutString;
	}

	return (char *)pszInString;
}

INT32 BurnAddEndSlash(char *szPath, size_t nSize)
{
	INT32 nLen = strlen(szPath);
	if (nLen < nSize - 2)
	{
		if (szPath[nLen - 1] != PATH_DEFAULT_SLASH_C())
		{
			szPath[nLen] = PATH_DEFAULT_SLASH_C();
			szPath[nLen + 1] = '\0';
			return 1;
		}
	}
	return 0;
}

void BurnMakeFilename(char *szName, const char *szPath, size_t nSize)
{
	const char *p = strrchr(szPath, PATH_DEFAULT_SLASH_C());
	if (!p++)
		p = szPath;
	strncpy(szName, p, nSize - 1);
	szName[nSize - 1] = '\0';
}

void BurnMakeBasename(char *szName, const char *szPath, size_t nSize)
{
	BurnMakeFilename(szName, szPath, nSize);
	char *p = strrchr(szName, '.');
	if (p)
		*p = '\0';
}

void BurnMakeBaseDir(char *szDir, const char *szPath, size_t nSize)
{
	strncpy(szDir, szPath, nSize - 1);
	szDir[nSize - 1] = '\0';

	char *p = strrchr(szDir, PATH_DEFAULT_SLASH_C());
	if (p++)
		*p = '\0';
	BurnAddEndSlash(szDir, nSize);
}

bool BurnCheckFileExist(const char *szPath)
{
	FILE *fp = fopen(szPath, "r");
	if (!fp)
		return false;
	fclose(fp);
	return true;
}

char *BurnDrvGetNameByIndex(UINT32 nDrv)
{
	char *szName = NULL;
	UINT32 nOldDrv;

	if (nDrv >= nBurnDrvCount)
		return NULL;

	nOldDrv = nBurnDrvActive;
	nBurnDrvActive = nDrv;
	szName = BurnDrvGetText(DRV_NAME);
	nBurnDrvActive = nOldDrv;

	return szName;
}

UINT32 BurnDrvGetIndexByName(const char *szName)
{
	UINT32 nOldDrv = nBurnDrvActive;
	UINT32 nDrv = 0;

	for (nBurnDrvActive = 0; nBurnDrvActive < nBurnDrvCount; nBurnDrvActive++)
	{
		if (strcasecmp(szName, BurnDrvGetText(DRV_NAME)) == 0)
			break;
	}
	nDrv = nBurnDrvActive;
	if (nDrv >= nBurnDrvCount)
		nDrv = ~0U;

	nBurnDrvActive = nOldDrv;

	return nDrv;
}

// 16-bit RGB565
static UINT32 __cdecl HighCol16(INT32 r, INT32 g, INT32 b, INT32 /* i */)
{
	UINT32 t;
	t = (r << 8) & 0xf800;
	t |= (g << 3) & 0x07e0;
	t |= (b >> 3) & 0x001f;
	return t;
}

// 32-bit XRGB8888
static UINT32 __cdecl HighCol32(INT32 r, INT32 g, INT32 b, INT32 /* i */)
{
	UINT32 t;
	t = (r << 16) & 0xff0000;
	t |= (g << 8) & 0x00ff00;
	t |= (b)&0x0000ff;

	return t;
}

INT32 SetBurnHighCol(INT32 nDepth)
{
	enum retro_pixel_format fmt;

	BurnRecalcPal();

	if (nDepth == 16)
	{
		fmt = RETRO_PIXEL_FORMAT_RGB565;
		BurnHighCol = HighCol16;
		nBurnBpp = 2;
	}
	else
	{
		fmt = RETRO_PIXEL_FORMAT_XRGB8888;
		BurnHighCol = HighCol32;
		nBurnBpp = 4;
	}

	environ_cb(RETRO_ENVIRONMENT_SET_PIXEL_FORMAT, &fmt);

	return 0;
}
