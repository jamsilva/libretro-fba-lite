#include "burner.h"

INT32 kNetGame = 0;
INT32 bRunPause = 0;
INT32 nAudSegLen = 0;
#ifdef USE_CYCLONE
// 0 - c68k, 1 - m68k
// we don't use cyclone by default because it breaks savestates cross-platform compatibility (including netplay)
int nSekCpuCore = 1;
#endif

char szAppBurnVer[16];

INT32 nBurnDrvOkay = 0;
INT32 nBurnGameType = 0;
INT32 nBurnGameWidth = 0;
INT32 nBurnGameHeight = 0;
INT32 nBurnColorDepth = 16;
UINT32 nBurnRotation = 0;

UINT32 nFrameskipNum = 0;
INT32 nAudSampleRate = 48000;
INT32 nAnalogSpeed = 0x0100;
UINT8 *pDiagActivatedKey = NULL;

retro_log_printf_t log_cb = NULL;
retro_environment_t environ_cb = NULL;
retro_video_refresh_t video_cb = NULL;
retro_audio_sample_batch_t audio_batch_cb = NULL;
retro_input_state_t input_state_cb = NULL;
retro_input_poll_t input_poll_cb = NULL;

static void log_dummy(enum retro_log_level level, const char *fmt, ...)
{
}

static INT32 __cdecl libretro_bprintf(INT32 nStatus, TCHAR *szFormat, ...)
{
	char buf[512];
	va_list vp;
	va_start(vp, szFormat);
	int rc = vsnprintf(buf, 512, szFormat, vp);
	va_end(vp);

	if (rc >= 0)
	{
		retro_log_level level = RETRO_LOG_DEBUG;
		if (nStatus == PRINT_UI)
			level = RETRO_LOG_INFO;
		else if (nStatus == PRINT_IMPORTANT)
			level = RETRO_LOG_WARN;
		else if (nStatus == PRINT_ERROR)
			level = RETRO_LOG_ERROR;

		log_cb(level, buf);
	}

	return rc;
}

INT32(__cdecl *bprintf)
(INT32 nStatus, TCHAR *szFormat, ...) = libretro_bprintf;

static void UpdateGeometry()
{
	BurnDrvGetFullSize(&nBurnGameWidth, &nBurnGameHeight);
	nBurnPitch = nBurnGameWidth * nBurnBpp;
	struct retro_system_av_info av_info;
	retro_get_system_av_info(&av_info);
	environ_cb(RETRO_ENVIRONMENT_SET_SYSTEM_AV_INFO, &av_info);
}

static void RunUpdateVariables()
{
	INT32 nOldColorDepth = nBurnColorDepth;
	BurnUpdateVariables();
	if (nBurnColorDepth != nOldColorDepth)
	{
		SetBurnHighCol(nBurnColorDepth);
		UpdateGeometry();
	}
}

static void SetBurnRotation()
{
	switch (BurnDrvGetFlags() & (BDF_ORIENTATION_FLIPPED | BDF_ORIENTATION_VERTICAL))
	{
	case BDF_ORIENTATION_VERTICAL:
		nBurnRotation = 1;
		break;
	case BDF_ORIENTATION_FLIPPED:
		nBurnRotation = 2;
		break;
	case BDF_ORIENTATION_VERTICAL | BDF_ORIENTATION_FLIPPED:
		nBurnRotation = 3;
		break;
	default:
		nBurnRotation = 0;
		break;
	}
	environ_cb(RETRO_ENVIRONMENT_SET_ROTATION, &nBurnRotation);
}

static void SetFrameskip()
{
	if (nFrameskipNum > 0)
		bBurnDraw = (nCurrentFrame % (nFrameskipNum + 1) == 0);
	else
		bBurnDraw = true;
}

void Reinitialise()
{
	// Update the geometry, some games (sfiii2) and systems (megadrive) need it.
	UpdateGeometry();
}

void retro_set_environment(retro_environment_t cb)
{
	environ_cb = cb;
}

void retro_set_video_refresh(retro_video_refresh_t cb)
{
	video_cb = cb;
}

void retro_set_audio_sample(retro_audio_sample_t)
{
}

void retro_set_audio_sample_batch(retro_audio_sample_batch_t cb)
{
	audio_batch_cb = cb;
}

void retro_set_input_state(retro_input_state_t cb)
{
	input_state_cb = cb;
}

void retro_set_input_poll(retro_input_poll_t cb)
{
	input_poll_cb = cb;
}

unsigned retro_get_region()
{
	return RETRO_REGION_NTSC;
}

unsigned retro_api_version()
{
	return RETRO_API_VERSION;
}

void retro_get_system_info(struct retro_system_info *info)
{
	info->library_name = APP_TITLE;
#ifndef GIT_VERSION
#define GIT_VERSION ""
#endif
	info->library_version = APP_VERSION GIT_VERSION;
	info->need_fullpath = true;
	info->block_extract = true;
	info->valid_extensions = "zip|7z";
}

void retro_get_system_av_info(struct retro_system_av_info *info)
{
	INT32 nXAspect, nYAspect;
	BurnDrvGetAspect(&nXAspect, &nYAspect);
	INT32 nMaximum = nBurnGameWidth > nBurnGameHeight ? nBurnGameWidth : nBurnGameHeight;

	info->geometry.base_width = (unsigned)nBurnGameWidth;
	info->geometry.base_height = (unsigned)nBurnGameHeight;
	info->geometry.max_width = (unsigned)nMaximum;
	info->geometry.max_height = (unsigned)nMaximum;
	info->geometry.aspect_ratio = (float)nXAspect / (float)nYAspect;

	info->timing.fps = nBurnFPS / 100.0;
	info->timing.sample_rate = nBurnSoundRate;
}

void retro_init()
{
	struct retro_log_callback log;
	struct retro_vfs_interface_info vfs_iface_info;

	if (environ_cb(RETRO_ENVIRONMENT_GET_LOG_INTERFACE, &log))
		log_cb = log.log;
	else
		log_cb = log_dummy;

	vfs_iface_info.required_interface_version = FILESTREAM_REQUIRED_VFS_VERSION;
	vfs_iface_info.iface = NULL;
	if (environ_cb(RETRO_ENVIRONMENT_GET_VFS_INTERFACE, &vfs_iface_info))
		filestream_vfs_init(&vfs_iface_info);

	snprintf(szAppBurnVer, sizeof(szAppBurnVer), "%x.%x.%x.%02x", nBurnVer >> 20, (nBurnVer >> 16) & 0x0F, (nBurnVer >> 8) & 0xFF, nBurnVer & 0xFF);
	BurnLibInit();
}

void retro_deinit()
{
	BurnLibExit();
}

void retro_reset()
{
	if (InputBindList == NULL || nInputBindResetOffset < 0)
		return;

	RunUpdateVariables();

	struct InputBind *pib = InputBindList + nInputBindResetOffset;
	pib->nVal = 1;
	if (pib->bii.pVal)
		*(pib->bii.pVal) = pib->nVal;
	BurnDrvFrame();
	InputBindCleanKeys();
}

void retro_run()
{
	nCurrentFrame++;

	SetFrameskip();
	BurnInputFrame();
	BurnDrvFrame();
	BurnAVPlayFrame();

	bool bUpdated = false;
	if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE, &bUpdated) && bUpdated)
		RunUpdateVariables();
}

static bool retro_load_game_common(const char *szRomPath)
{
	retro_unload_game();

	nBurnDrvActive = ~0U;
	bBurnThreadDraw = false;
	nCurrentFrame = 0;

	BurnMakeAppPaths(szRomPath);

	if (BurnCheckFileExist(szAppExtDrvDatPath))
		nBurnDrvActive = BurnDrvLoadExtDriver(szAppExtDrvDatPath);

	if (nBurnDrvActive == ~0U)
		nBurnDrvActive = BurnDrvGetIndexByName(szAppDrvName);

	if (nBurnDrvActive == ~0U)
	{
		log_cb(RETRO_LOG_ERROR, "[FBA] Can't launch this game, it's unset\n");
		return false;
	}
	if (!(BurnDrvIsWorking()))
	{
		log_cb(RETRO_LOG_ERROR, "[FBA] Can't launch this game, it's marked as not working\n");
		return false;
	}
	if ((BurnDrvGetFlags() & BDF_BOARDROM))
	{
		log_cb(RETRO_LOG_ERROR, "[FBA] Board rom isn't meant to be launched this way\n");
		return false;
	}

	const char *szFullName = BurnDrvGetTextA(DRV_FULLNAME);
	if (szFullName)
		log_cb(RETRO_LOG_ERROR, "[FBA] Full name: %s\n", szFullName);

	if (BzipOpen() != 0)
	{
		log_cb(RETRO_LOG_INFO, "[FBA] Launch game %s failed at check rom!\n", szAppRomName);
		return false;
	}

	nBurnLayer = 0xff;
	nMaxPlayers = BurnDrvGetMaxPlayers();

	BurnInputInit();

	BurnSetVariables();
	BurnUpdateVariables();

	if (BurnDrvInit() != 0)
	{
		BurnDrvExit();
		log_cb(RETRO_LOG_INFO, "[FBA] Launch game %s failed at init driver!\n", szAppRomName);
		return false;
	}

	BurnCheevosInit();

	// Loading minimal savestate (not exactly sure why it is needed)
	snprintf(szAppAutoFsPath, MAX_PATH, "%s%s.fs", szAppDataPath, BurnDrvGetText(DRV_NAME));
	BurnStateLoad(szAppAutoFsPath, 0, NULL);

	SetBurnHighCol(nBurnColorDepth);
	SetBurnRotation();
	UpdateGeometry();

	BurnAVPlayInit();

	log_cb(RETRO_LOG_INFO, "[FBA] Launch game %s OK!\n", szAppRomName);
	nBurnDrvOkay = 1;

	return true;
}

bool retro_load_game(const struct retro_game_info *info)
{
	if (!info)
		return false;

	nBurnGameType = RETRO_GAME_TYPE_DEF;

	return retro_load_game_common(info->path);
}

bool retro_load_game_special(unsigned game_type, const struct retro_game_info *info, size_t num_info)
{
	if (!info)
		return false;

	nBurnGameType = game_type;

	return retro_load_game_common(info->path);
}

void retro_unload_game(void)
{
	if (nBurnDrvOkay)
	{
		BurnStateSave(szAppAutoFsPath, 0);
		BurnDrvExit();
	}

	if (bBurnExtDrvMode)
		BurnDrvUnloadExtDriver();

	BzipClose();
	BurnInputExit();
	BurnCheevosExit();
	BurnAVPlayExit();

	nBurnDrvOkay = 0;
}
