#include <gccore.h>
#include <string.h>
#include <stdio.h>

#include "nand_ios.h"

#define MEM_REG_BASE 0x0D8B4000

#define HW_AHBPROT ((volatile u32 *)0xCD800064)
#define MEM_PROT    (0x0D8B420A)
#define IOS_ENTRY_PTR ((volatile u32 *)0x80003134)
#define IOS_PATCH_END ((u8 *)0x94000000)

/*
 * Standard Wii IOS ISFS permission patch used by established Wii
 * homebrew such as WiiFlow Lite and USB Loader GX.
 */
static const u8 isfs_permissions_old[] =
{
    0x42, 0x8B, 0xD0, 0x01, 0x25, 0x66
};

static const u8 isfs_permissions_patch[] =
{
    0x42, 0x8B, 0xE0, 0x01, 0x25, 0x66
};

/* ES_Identify: replace the conditional branch at offset 2 with NOPs. */
static const u8 hash_old[] =
{
    0x20, 0x07, 0x23, 0xA2
};

static const u8 new_hash_old[] =
{
    0x20, 0x07, 0x4B, 0x0B
};

static const u8 hash_patch[] =
{
    0x00
};
static const u8 es_identify_old[] =
{
    0x28, 0x03, 0xD1, 0x23
};

static const u8 es_identify_patch[] =
{
    0x00, 0x00
};

/* ES_SetUID: bypass the caller/UID check so the browser can inspect
 * title-owned ISFS directories without modifying NAND. */
static const u8 setuid_old[] =
{
    0xD1, 0x2A, 0x1C, 0x39
};

static const u8 setuid_patch[] =
{
    0x46, 0xC0
};

static int nand_ios_access_enabled = 0;
static int nand_patch_isfs = 0;
static int nand_patch_setuid = 0;
static int nand_patch_identify = 0;
static int nand_patch_hash = 0;
static int nand_patch_new_hash = 0;

static void disable_memory_protection(void)
{
    write32(MEM_PROT, read32(MEM_PROT) & 0x0000FFFF);
}

static int apply_patch(const u8 *old, size_t old_size,
                      const u8 *patch, size_t patch_size,
                      size_t patch_offset)
{
    u8 *ptr = (u8 *)*IOS_ENTRY_PTR;
    u8 *end = IOS_PATCH_END;
    int found = 0;

    if ((u32)ptr < 0x90000000 || (u32)ptr >= (u32)end)
        return 0;

    while (ptr + old_size <= end)
    {
        if (memcmp(ptr, old, old_size) == 0)
        {
            memcpy(ptr + patch_offset, patch, patch_size);
            DCFlushRange(ptr + patch_offset, patch_size);
            ICInvalidateRange(ptr + patch_offset, patch_size);
            found++;
        }
        ptr++;
    }

    return found;
}

static int apply_isfs_permission_patch(void)
{
    return apply_patch(
        isfs_permissions_old,
        sizeof(isfs_permissions_old),
        isfs_permissions_patch,
        sizeof(isfs_permissions_patch),
        0
    );
}

static int apply_es_identify_patch(void)
{
    return apply_patch(
        es_identify_old,
        sizeof(es_identify_old),
        es_identify_patch,
        sizeof(es_identify_patch),
        2
    );
}

static int apply_hash_patches(void)
{
    int found = 0;

    nand_patch_hash = apply_patch(
        hash_old,
        sizeof(hash_old),
        hash_patch,
        sizeof(hash_patch),
        1
    );
    nand_patch_new_hash = apply_patch(new_hash_old, sizeof(new_hash_old), hash_patch, sizeof(hash_patch), 1);

    nand_patch_setuid = apply_patch(setuid_old, sizeof(setuid_old), setuid_patch, sizeof(setuid_patch), 0);

    found += nand_patch_hash + nand_patch_new_hash + nand_patch_setuid;

    return found;
}

int nand_ios_enable_access(void)
{
    int found;

    if (nand_ios_access_enabled)
        return nand_patch_isfs + nand_patch_identify + nand_patch_hash + nand_patch_new_hash + nand_patch_setuid;

    if (*HW_AHBPROT != 0xFFFFFFFF)
        return -1;

    disable_memory_protection();

    nand_patch_isfs = apply_isfs_permission_patch();
    nand_patch_identify = apply_es_identify_patch();
    nand_patch_hash = 0;
    nand_patch_new_hash = 0;
    nand_patch_setuid = 0;
    found = nand_patch_isfs + nand_patch_identify + apply_hash_patches();

    if (found <= 0)
        return 0;

    nand_ios_access_enabled = 1;
    return found;
}

void nand_ios_get_patch_status(char *buffer, size_t size)
{
    if (buffer == NULL || size == 0)
        return;

    snprintf(buffer, size,
             "ISFS:%d UID:%d ID:%d HASH:%d NEW:%d",
             nand_patch_isfs, nand_patch_setuid, nand_patch_identify,
             nand_patch_hash, nand_patch_new_hash);
}
