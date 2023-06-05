#include <string>

#include "burner.h"
#include "neogeo_bios.h"

struct BiosInfo NeogeoBiosInfos[] = {
    {"sp-s3.sp1", 0x91b64be3, 0x00, "MVS Asia/Europe ver. 6 (1 slot)", NEOGEO_MVS | NEOGEO_EUR | NEOGEO_ASI, 0},
    {"sp-s2.sp1", 0x9036d879, 0x01, "MVS Asia/Europe ver. 5 (1 slot)", NEOGEO_MVS | NEOGEO_EUR | NEOGEO_ASI, 0},
    {"sp-s.sp1", 0xc7f2fa45, 0x02, "MVS Asia/Europe ver. 3 (4 slot)", NEOGEO_MVS | NEOGEO_EUR | NEOGEO_ASI, 0},
    {"sp-u2.sp1", 0xe72943de, 0x03, "MVS USA ver. 5 (2 slot)", NEOGEO_MVS | NEOGEO_USA, 0},
    {"sp1-u2", 0x62f021f4, 0x04, "MVS USA ver. 5 (4 slot)", NEOGEO_MVS | NEOGEO_USA, 0},
    {"sp-e.sp1", 0x2723a5b5, 0x05, "MVS USA ver. 5 (6 slot)", NEOGEO_MVS | NEOGEO_USA, 0},
    {"sp1-u4.bin", 0x1179a30f, 0x06, "MVS USA (U4)", NEOGEO_MVS | NEOGEO_USA, 0},
    {"sp1-u3.bin", 0x2025b7a2, 0x07, "MVS USA (U3)", NEOGEO_MVS | NEOGEO_USA, 0},
    {"vs-bios.rom", 0xf0e8f27d, 0x08, "MVS Japan ver. 6 (? slot)", NEOGEO_MVS | NEOGEO_JAP, 0},
    {"sp-j2.sp1", 0xacede59C, 0x09, "MVS Japan ver. 5 (? slot)", NEOGEO_MVS | NEOGEO_JAP, 0},
    {"sp1.jipan.1024", 0x9fb0abe4, 0x0a, "MVS Japan ver. 3 (4 slot)", NEOGEO_MVS | NEOGEO_JAP, 0},
    {"sp-45.sp1", 0x03cc9f6a, 0x0b, "NEO-MVH MV1C (Asia)", NEOGEO_MVS | NEOGEO_ASI, 0},
    {"sp-j3.sp1", 0x486cb450, 0x0c, "NEO-MVH MV1C (Japan)", NEOGEO_MVS | NEOGEO_JAP, 0},
    {"japan-j3.bin", 0xdff6d41f, 0x0d, "MVS Japan (J3)", NEOGEO_MVS | NEOGEO_JAP, 0},
    {"sp1-j3.bin", 0xfbc6d469, 0x0e, "MVS Japan (J3, alt)", NEOGEO_MVS | NEOGEO_JAP, 0},
    {"neo-po.bin", 0x16d0c132, 0x0f, "AES Japan", NEOGEO_AES | NEOGEO_JAP, 0},
    {"neo-epo.bin", 0xd27a71f1, 0x10, "AES Asia", NEOGEO_AES | NEOGEO_ASI, 0},
    {"uni-bios_4_0.rom", 0xa7aab458, 0x13, "Universe BIOS ver. 4.0", NEOGEO_UNI, 0},
    {"uni-bios_3_3.rom", 0x24858466, 0x14, "Universe BIOS ver. 3.3", NEOGEO_UNI, 0},
    {"uni-bios_3_2.rom", 0xa4e8b9b3, 0x15, "Universe BIOS ver. 3.2", NEOGEO_UNI, 0},
    {"uni-bios_3_1.rom", 0x0c58093f, 0x16, "Universe BIOS ver. 3.1", NEOGEO_UNI, 0},
    {"uni-bios_3_0.rom", 0xa97c89a9, 0x17, "Universe BIOS ver. 3.0", NEOGEO_UNI, 0},
    {"uni-bios_2_3.rom", 0x27664eb5, 0x18, "Universe BIOS ver. 2.3", NEOGEO_UNI, 0},
    {"uni-bios_2_3o.rom", 0x601720ae, 0x19, "Universe BIOS ver. 2.3 (alt)", NEOGEO_UNI, 0},
    {"uni-bios_2_2.rom", 0x2d50996a, 0x1a, "Universe BIOS ver. 2.2", NEOGEO_UNI, 0},
    {"uni-bios_2_1.rom", 0x8dabf76b, 0x1b, "Universe BIOS ver. 2.1", NEOGEO_UNI, 0},
    {"uni-bios_2_0.rom", 0x0c12c2ad, 0x1c, "Universe BIOS ver. 2.0", NEOGEO_UNI, 0},
    {"uni-bios_1_3.rom", 0xb24b44a0, 0x1d, "Universe BIOS ver. 1.3", NEOGEO_UNI, 0},
    {"uni-bios_1_2.rom", 0x4fa698e9, 0x1e, "Universe BIOS ver. 1.2", NEOGEO_UNI, 0},
    {"uni-bios_1_2o.rom", 0xe19d3ce9, 0x1f, "Universe BIOS ver. 1.2 (alt)", NEOGEO_UNI, 0},
    {"uni-bios_1_1.rom", 0x5dda0d84, 0x20, "Universe BIOS ver. 1.1", NEOGEO_UNI, 0},
    {"uni-bios_1_0.rom", 0x0ce453a0, 0x21, "Universe BIOS ver. 1.0", NEOGEO_UNI, 0},
    {NULL, 0, 0, NULL, 0},
};

extern UINT8 NeoSystem;

BiosInfo *GetNeogeoBiosInfo(UINT32 nCateg)
{
    for (INT32 i = 0; NeogeoBiosInfos[i].szName != NULL; i++)
    {
        if (NeogeoBiosInfos[i].nCateg == nCateg && NeogeoBiosInfos[i].nAvailable == 1)
            return &NeogeoBiosInfos[i];
    }

    return NULL;
}

void ResetNeogeoBiosInfos()
{
    for (INT32 i = 0; NeogeoBiosInfos[i].szName != NULL; i++)
    {
        NeogeoBiosInfos[i].nAvailable = 0;
    }
}

void SetNeogeoBiosAvailability(char *szName, UINT32 nCrc)
{
    for (INT32 i = 0; NeogeoBiosInfos[i].szName != NULL; i++)
    {
        if (NeogeoBiosInfos[i].nCrc == nCrc || strcasecmp(NeogeoBiosInfos[i].szName, szName) == 0)
        {
            NeogeoBiosInfos[i].nAvailable = 1;
            return;
        }
    }
}

void SetNeoSystem(UINT32 nCateg)
{
    if (nCateg == 0)
    {
        // Nothing to do in DIPSWITCH mode because the NeoSystem variable is changed by the DIP Switch core option
        log_cb(RETRO_LOG_INFO, "DIPSWITCH NeoGeo Mode selected => NeoSystem: 0x%02x\n", NeoSystem);
        return;
    }

    BiosInfo *pbi = GetNeogeoBiosInfo(nCateg);
    if (pbi)
    {
        UINT8 nMask = 0x3f;
        NeoSystem = (NeoSystem & ~nMask) | (pbi->nSetting & nMask);
        log_cb(RETRO_LOG_INFO, "Requested NeoGeo mode selected => NeoSystem: 0x%02x\n", NeoSystem);
    }
    else
    {
        log_cb(RETRO_LOG_INFO, "Requested NeoGeo mode failed => NeoSystem: 0x%02x\n", NeoSystem);
    }
}
