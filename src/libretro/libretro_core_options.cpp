#include <string>
#include <vector>

#include "libretro_core_options.h"
#include "burner.h"
#include "neogeo_bios.h"

struct DipOptionValue
{
    char szValue[MAX_NAME];
    struct BurnDIPInfo bdi;
};

struct DipOption
{
    char szKey[MAX_NAME];
    char szDesc[MAX_NAME];
    struct BurnDIPInfo bdi;
    std::vector<struct DipOptionValue> vecValues;
    char szDefaultValue[MAX_NAME];
};

static UINT8 DiagKey_start[] = {RETRO_DEVICE_ID_JOYPAD_START, RETRO_DEVICE_ID_JOYPAD_EMPTY};
static UINT8 DiagKey_start_a_b[] = {RETRO_DEVICE_ID_JOYPAD_START, RETRO_DEVICE_ID_JOYPAD_A, RETRO_DEVICE_ID_JOYPAD_B, RETRO_DEVICE_ID_JOYPAD_EMPTY};
static UINT8 DiagKey_start_l_r[] = {RETRO_DEVICE_ID_JOYPAD_START, RETRO_DEVICE_ID_JOYPAD_L, RETRO_DEVICE_ID_JOYPAD_R, RETRO_DEVICE_ID_JOYPAD_EMPTY};
static UINT8 DiagKey_select[] = {RETRO_DEVICE_ID_JOYPAD_SELECT, RETRO_DEVICE_ID_JOYPAD_EMPTY};
static UINT8 DiagKey_select_a_b[] = {RETRO_DEVICE_ID_JOYPAD_SELECT, RETRO_DEVICE_ID_JOYPAD_A, RETRO_DEVICE_ID_JOYPAD_B, RETRO_DEVICE_ID_JOYPAD_EMPTY};
static UINT8 DiagKey_select_l_r[] = {RETRO_DEVICE_ID_JOYPAD_SELECT, RETRO_DEVICE_ID_JOYPAD_L, RETRO_DEVICE_ID_JOYPAD_R, RETRO_DEVICE_ID_JOYPAD_EMPTY};

static std::vector<struct DipOption> vecDipOptions;
static UINT32 nNeoBiosCateg = 0;
static bool bNeoSystemSetEnabled = false;
static bool bColorDepth32Enabled = false;
#ifdef USE_CYCLONE
static bool bCycloneEnabled = false;
#endif

static void CheckNeoSystemSetEnabled()
{
    struct BurnDIPInfo bdi;

    bNeoSystemSetEnabled = false;

    for (INT32 i = 0; BurnDrvGetDIPInfo(&bdi, i) == 0; i++)
    {
        if (bdi.nFlags != 0xFE && bdi.nFlags != 0xFD)
            continue;

        if (strcasecmp(bdi.szText, "BIOS") == 0 && bdi.nSetting > 0)
        {
            bNeoSystemSetEnabled = true;
            break;
        }
    }
}

static void CreateDipVariables()
{
    char szTempText[MAX_NAME];
    struct BurnDIPInfo bdi, bdi2;

    vecDipOptions.clear();

    if (InputBindList == NULL || nInputBindDIPOffset < 0)
        return;

    for (INT32 i = 0; BurnDrvGetDIPInfo(&bdi, i) == 0; i++)
    {
        /* 0xFE is the beginning label for a DIP switch entry */
        /* 0xFD are region DIP switches */
        if ((bdi.nFlags != 0xFE && bdi.nFlags != 0xFD) || bdi.nSetting < 1)
            continue;

        if (bBurnNeogeoGame && !bNeoSystemSetEnabled && bdi.szText && strcasecmp(bdi.szText, "BIOS") == 0)
            continue;

        vecDipOptions.push_back(DipOption());
        struct DipOption *pOption = &(vecDipOptions.back());
        memcpy(&pOption->bdi, &bdi, sizeof(pOption->bdi));

        // Some dipswitch has no name...
        if (bdi.szText)
        {
            strncpy(szTempText, bdi.szText, sizeof(szTempText));
        }
        else // ... so, to not hang, we will generate a name based on the position of the dip (DIPSWITCH 1, DIPSWITCH 2...)
        {
            sprintf(szTempText, "DIPSWITCH %d", vecDipOptions.size());
            log_cb(RETRO_LOG_WARN, "Error in DIPList : The DIPSWITCH '%d' has no name. '%s' name has been generated\n", vecDipOptions.size(), szTempText);
        }

        strncpy(pOption->szDesc, szTempText, sizeof(pOption->szDesc));
        // log_cb(RETRO_LOG_INFO, "[DipOption]szDesc: %s\n", pOption->szDesc);

        StrReplace(szTempText, ' ', '_');
        StrReplace(szTempText, '=', '_');
        snprintf(pOption->szKey, sizeof(pOption->szKey), "fba-dips-%s", szTempText);

        // Search for duplicate name, and add number to make them unique in the core-options file
        INT32 nSubKey = 2;
        for (INT32 j = 0; j < vecDipOptions.size() - 1; j++) // - 1 to exclude the current one
        {
            if (strcmp(pOption->szKey, vecDipOptions[j].szKey) == 0)
            {
                snprintf(pOption->szKey, sizeof(pOption->szKey), "fba-dips-%s_%d", szTempText, nSubKey);
                nSubKey++;
            }
        }

        for (INT32 j = 0; j < bdi.nSetting; j++)
        {
            INT32 k = i + 1 + j;

            if (BurnDrvGetDIPInfo(&bdi2, k) != 0)
            {
                log_cb(RETRO_LOG_WARN, "Error in DIPList for DIPSWITCH '%s': End of the struct was reached too early\n", pOption->szDesc);
                break;
            }

            if (bdi2.nFlags == 0xFE || bdi2.nFlags == 0xFD)
            {
                log_cb(RETRO_LOG_WARN, "Error in DIPList for DIPSWITCH '%s': Start of next DIPSWITCH is too early\n", pOption->szDesc);
                break;
            }

            INT32 nOffset = nInputBindDIPOffset + bdi2.nInput;
            if (nOffset >= nInputBindListCount)
                continue;

            struct InputBind *pib = InputBindList + nOffset;
            struct BurnInputInfo *pbii = &(pib->bii);

            // When the pVal of one value is NULL => the DIP switch is unusable. So skip it by removing it from the list
            if (!pbii->pVal)
            {
                vecDipOptions.pop_back();
                log_cb(RETRO_LOG_WARN, "Error in DIPList for DIPSWITCH '%s': the line '%d' is unusable\n", pOption->szDesc, k);
                break;
            }

            // Filter away NULL entries
            if (bdi2.nFlags == 0)
            {
                log_cb(RETRO_LOG_WARN, "Error in DIPList for DIPSWITCH '%s': the line '%d' is useless\n", pOption->szDesc, k);
                continue;
            }

            pOption->vecValues.push_back(DipOptionValue());
            struct DipOptionValue *pValue = &(pOption->vecValues.back());

            memcpy(&pValue->bdi, &bdi2, sizeof(pValue->bdi));
            if (bdi2.szText)
                strncpy(pValue->szValue, bdi2.szText, sizeof(pValue->szValue));
            else
                sprintf(pValue->szValue, "Setting %d", j + 1);

            UINT8 nVal = (pib->nVal & ~bdi2.nMask) | (bdi2.nSetting & bdi2.nMask);
            if (nVal == pib->nDefVal)
                strncpy(pOption->szDefaultValue, pValue->szValue, sizeof(pOption->szDefaultValue));
        }
    }
}

void BurnSetVariables()
{
    std::vector<const retro_core_option_definition *> vecSysOptions;

    if (bBurnNeogeoGame)
        CheckNeoSystemSetEnabled();

    CreateDipVariables();

    // Add the Global core options
    vecSysOptions.push_back(&var_fba_allow_depth_32);
    vecSysOptions.push_back(&var_fba_sound_out);
    vecSysOptions.push_back(&var_fba_frameskip);
    vecSysOptions.push_back(&var_fba_cpu_speed_adjust);
    if (BurnDrvGetFlags() & BDF_HISCORE_SUPPORTED)
        vecSysOptions.push_back(&var_fba_hiscores);
    if (nBurnGameType != RETRO_GAME_TYPE_NEOCD)
        vecSysOptions.push_back(&var_fba_samplerate);
    vecSysOptions.push_back(&var_fba_sample_interpolation);
    vecSysOptions.push_back(&var_fba_fm_interpolation);
    // vecSysOptions.push_back(&var_fba_analog_speed);
#ifdef USE_CYCLONE
    if (bBurnPgmGame)
        var_fba_cyclone.default_value = enabled_value;
    else
        var_fba_cyclone.default_value = disabled_value;
    vecSysOptions.push_back(&var_fba_cyclone);
#endif

    if (nInputBindDiagOffset >= 0)
        vecSysOptions.push_back(&var_fba_diagnostic_input);

    if (bBurnNeogeoGame && bNeoSystemSetEnabled)
        vecSysOptions.push_back(&var_fba_neogeo_mode);

    INT32 nSysOptionsCount = vecSysOptions.size();
    INT32 nDipOptionsCount = vecDipOptions.size();

    option_defs_us = (struct retro_core_option_definition *)calloc(nSysOptionsCount + nDipOptionsCount + 1, sizeof(struct retro_core_option_definition));

    INT32 nOptionsIdx = 0;

    // Add the System core options
    for (INT32 i = 0; i < nSysOptionsCount; i++)
    {
        option_defs_us[nOptionsIdx] = *vecSysOptions[i];
        nOptionsIdx++;
    }

    // Add the DIP switches core options
    for (INT32 i = 0; i < nDipOptionsCount; i++)
    {
        option_defs_us[nOptionsIdx].key = vecDipOptions[i].szKey;
        option_defs_us[nOptionsIdx].desc = vecDipOptions[i].szDesc;
        option_defs_us[nOptionsIdx].default_value = vecDipOptions[i].szDefaultValue;

        INT32 nValuesCount = vecDipOptions[i].vecValues.size();
        for (INT32 j = 0; j < nValuesCount; j++)
        {
            option_defs_us[nOptionsIdx].values[j].value = vecDipOptions[i].vecValues[j].szValue;
        }
        option_defs_us[nOptionsIdx].values[nValuesCount].value = NULL;

        nOptionsIdx++;
    }

    option_defs_us[nOptionsIdx] = var_empty;

    libretro_set_core_options(environ_cb);

    free(option_defs_us);
    option_defs_us = NULL;
}

static void CheckVariables(void)
{
    struct retro_variable var = {0};

    var.key = var_fba_allow_depth_32.key;
    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var))
    {
        if (strcmp(var.value, "enabled") == 0)
            bColorDepth32Enabled = true;
        else
            bColorDepth32Enabled = false;
    }

    var.key = var_fba_sound_out.key;
    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var))
    {
        if (strcmp(var.value, "disabled") == 0)
            bBurnSound = false;
        else
            bBurnSound = true;
    }

    var.key = var_fba_frameskip.key;
    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var))
    {
        if (strcmp(var.value, "0") == 0)
            nFrameskipNum = 0;
        else if (strcmp(var.value, "1") == 0)
            nFrameskipNum = 1;
        else if (strcmp(var.value, "2") == 0)
            nFrameskipNum = 2;
        else if (strcmp(var.value, "3") == 0)
            nFrameskipNum = 3;
        else if (strcmp(var.value, "4") == 0)
            nFrameskipNum = 4;
        else if (strcmp(var.value, "5") == 0)
            nFrameskipNum = 5;
        else
            nFrameskipNum = 0;
    }

    var.key = var_fba_cpu_speed_adjust.key;
    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var))
    {
        if (strcmp(var.value, "110") == 0)
            nBurnCPUSpeedAdjust = 0x0110;
        else if (strcmp(var.value, "120") == 0)
            nBurnCPUSpeedAdjust = 0x0120;
        else if (strcmp(var.value, "130") == 0)
            nBurnCPUSpeedAdjust = 0x0130;
        else if (strcmp(var.value, "140") == 0)
            nBurnCPUSpeedAdjust = 0x0140;
        else if (strcmp(var.value, "150") == 0)
            nBurnCPUSpeedAdjust = 0x0150;
        else if (strcmp(var.value, "160") == 0)
            nBurnCPUSpeedAdjust = 0x0160;
        else if (strcmp(var.value, "170") == 0)
            nBurnCPUSpeedAdjust = 0x0170;
        else if (strcmp(var.value, "180") == 0)
            nBurnCPUSpeedAdjust = 0x0180;
        else if (strcmp(var.value, "190") == 0)
            nBurnCPUSpeedAdjust = 0x0190;
        else if (strcmp(var.value, "200") == 0)
            nBurnCPUSpeedAdjust = 0x0200;
        else
            nBurnCPUSpeedAdjust = 0x0100;
    }

    if (BurnDrvGetFlags() & BDF_HISCORE_SUPPORTED)
    {
        var.key = var_fba_hiscores.key;
        if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var))
        {
            if (strcmp(var.value, "enabled") == 0)
                EnableHiscores = true;
            else
                EnableHiscores = false;
        }
    }

    if (nBurnGameType == RETRO_GAME_TYPE_NEOCD)
    {
        // src/burn/drv/neogeo/neo_run.cpp is mentioning issues with ngcd cdda playback if samplerate isn't 44100
        nAudSampleRate = 44100;
    }
    else
    {
        var.key = var_fba_samplerate.key;
        if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var))
        {
            if (strcmp(var.value, "48000") == 0)
                nAudSampleRate = 48000;
            else if (strcmp(var.value, "44100") == 0)
                nAudSampleRate = 44100;
            else if (strcmp(var.value, "22050") == 0)
                nAudSampleRate = 22050;
            else if (strcmp(var.value, "11025") == 0)
                nAudSampleRate = 11025;
            else
                nAudSampleRate = 48000;
        }
    }

    var.key = var_fba_sample_interpolation.key;
    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var))
    {
        if (strcmp(var.value, "4-point 3rd order") == 0)
            nInterpolation = 3;
        else if (strcmp(var.value, "2-point 1st order") == 0)
            nInterpolation = 1;
        else if (strcmp(var.value, "disabled") == 0)
            nInterpolation = 0;
        else
            nInterpolation = 0;
    }

    var.key = var_fba_fm_interpolation.key;
    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var))
    {
        if (strcmp(var.value, "4-point 3rd order") == 0)
            nFMInterpolation = 3;
        else if (strcmp(var.value, "disabled") == 0)
            nFMInterpolation = 0;
        else
            nFMInterpolation = 0;
    }

    var.key = var_fba_analog_speed.key;
    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var))
    {
        if (strcmp(var.value, "10") == 0)
            nAnalogSpeed = 0x0100;
        else if (strcmp(var.value, "9") == 0)
            nAnalogSpeed = 0x00F0;
        else if (strcmp(var.value, "8") == 0)
            nAnalogSpeed = 0x00E0;
        else if (strcmp(var.value, "7") == 0)
            nAnalogSpeed = 0x00C0;
        else if (strcmp(var.value, "6") == 0)
            nAnalogSpeed = 0x00B0;
        else if (strcmp(var.value, "5") == 0)
            nAnalogSpeed = 0x00A0;
        else if (strcmp(var.value, "4") == 0)
            nAnalogSpeed = 0x0090;
        else if (strcmp(var.value, "3") == 0)
            nAnalogSpeed = 0x0080;
        else if (strcmp(var.value, "2") == 0)
            nAnalogSpeed = 0x0070;
        else if (strcmp(var.value, "1") == 0)
            nAnalogSpeed = 0x0060;
        else
            nAnalogSpeed = 0x0100;
    }

#ifdef USE_CYCLONE
    var.key = var_fba_cyclone.key;
    if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var))
    {
        if (strcmp(var.value, enabled_value) == 0)
            bCycloneEnabled = true;
        else
            bCycloneEnabled = false;
    }
#endif

    if (nInputBindDiagOffset >= 0)
    {
        var.key = var_fba_diagnostic_input.key;
        if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var))
        {
            if (strcmp(var.value, "Hold Start") == 0)
            {
                pDiagActivatedKey = DiagKey_start;
                nDiagKeyActivateHoldCount = 60;
            }
            else if (strcmp(var.value, "Start + A + B") == 0)
            {
                pDiagActivatedKey = DiagKey_start_a_b;
                nDiagKeyActivateHoldCount = 1;
            }
            else if (strcmp(var.value, "Hold Start + A + B") == 0)
            {
                pDiagActivatedKey = DiagKey_start_a_b;
                nDiagKeyActivateHoldCount = 60;
            }
            else if (strcmp(var.value, "Start + L + R") == 0)
            {
                pDiagActivatedKey = DiagKey_start_l_r;
                nDiagKeyActivateHoldCount = 1;
            }
            else if (strcmp(var.value, "Hold Start + L + R") == 0)
            {
                pDiagActivatedKey = DiagKey_start_l_r;
                nDiagKeyActivateHoldCount = 60;
            }
            else if (strcmp(var.value, "Hold Select") == 0)
            {
                pDiagActivatedKey = DiagKey_select;
                nDiagKeyActivateHoldCount = 60;
            }
            else if (strcmp(var.value, "Select + A + B") == 0)
            {
                pDiagActivatedKey = DiagKey_select_a_b;
                nDiagKeyActivateHoldCount = 1;
            }
            else if (strcmp(var.value, "Hold Select + A + B") == 0)
            {
                pDiagActivatedKey = DiagKey_select_a_b;
                nDiagKeyActivateHoldCount = 60;
            }
            else if (strcmp(var.value, "Select + L + R") == 0)
            {
                pDiagActivatedKey = DiagKey_select_l_r;
                nDiagKeyActivateHoldCount = 1;
            }
            else if (strcmp(var.value, "Hold Select + L + R") == 0)
            {
                pDiagActivatedKey = DiagKey_select_l_r;
                nDiagKeyActivateHoldCount = 60;
            }
            else
            {
                pDiagActivatedKey = NULL;
                nDiagKeyActivateHoldCount = 0;
            }
        }
    }

    if (bBurnNeogeoGame && bNeoSystemSetEnabled)
    {
        var.key = var_fba_neogeo_mode.key;
        if (environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var))
        {
            if (strcmp(var.value, "MVS_EUR_ASI") == 0)
                nNeoBiosCateg = NEOGEO_MVS | NEOGEO_EUR | NEOGEO_ASI;
            else if (strcmp(var.value, "MVS_USA") == 0)
                nNeoBiosCateg = NEOGEO_MVS | NEOGEO_USA;
            else if (strcmp(var.value, "MVS_ASI") == 0)
                nNeoBiosCateg = NEOGEO_MVS | NEOGEO_ASI;
            else if (strcmp(var.value, "MVS_JAP") == 0)
                nNeoBiosCateg = NEOGEO_MVS | NEOGEO_JAP;
            else if (strcmp(var.value, "AES_ASI") == 0)
                nNeoBiosCateg = NEOGEO_AES | NEOGEO_ASI;
            else if (strcmp(var.value, "AES_JAP") == 0)
                nNeoBiosCateg = NEOGEO_AES | NEOGEO_JAP;
            else if (strcmp(var.value, "UNIBIOS") == 0)
                nNeoBiosCateg = NEOGEO_UNI;
            else
                nNeoBiosCateg = 0;
        }
    }
}

static bool ApplyDipVariables()
{
    bool bDipChanged = false;
    struct retro_variable var = {0};
    struct DipOption *pOption;
    struct DipOptionValue *pValue;
    struct BurnDIPInfo *pbdi;

    if (!InputBindList || nInputBindDIPOffset < 0)
        return false;

    for (INT32 i = 0; i < vecDipOptions.size(); i++)
    {
        pOption = &vecDipOptions[i];

        var.key = pOption->szKey;
        if (!environ_cb(RETRO_ENVIRONMENT_GET_VARIABLE, &var))
            continue;

        for (INT32 j = 0; j < pOption->vecValues.size(); j++)
        {
            pValue = &(pOption->vecValues[j]);
            pbdi = &(pValue->bdi);

            if (strcmp(var.value, pValue->szValue) == 0)
            {
                INT32 nOffset = nInputBindDIPOffset + pbdi->nInput;
                if (nOffset >= nInputBindListCount)
                    continue;

                struct InputBind *pib = InputBindList + nOffset;
                struct BurnInputInfo *pbii = &(pib->bii);
                UINT8 nOldVal = pib->nVal;
                pib->nVal = (pib->nVal & ~pbdi->nMask) | (pbdi->nSetting & pbdi->nMask);
                *(pbii->pVal) = pib->nVal;
                if (pib->nVal != nOldVal)
                    bDipChanged = true;
                break;
            }
        }
    }

    return bDipChanged;
}

void BurnUpdateVariables()
{
    CheckVariables();

    // Just set befor BurnDrvInit
    if (!nBurnDrvOkay)
    {
        nBurnSoundRate = nAudSampleRate;
        nBurnSoundLen = (nAudSampleRate * 100 + (6000 >> 1)) / 6000;
#ifdef USE_CYCLONE
        nSekCpuCore = (bCycloneEnabled ? 0 : 1);
#endif
    }
    if ((BurnDrvGetFlags() & BDF_16BIT_ONLY) || !bColorDepth32Enabled || bBurnPgmGame)
        nBurnColorDepth = 16;
    else
        nBurnColorDepth = 32;

    ApplyDipVariables();

    // Override the NeoGeo bios DIP Switch by the option (for the moment)
    if (bBurnNeogeoGame && bNeoSystemSetEnabled)
        SetNeoSystem(nNeoBiosCateg);
}