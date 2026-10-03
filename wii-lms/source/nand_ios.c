#include <gccore.h>
#include <string.h>

#include "nand_ios.h"

#define MEM_REG_BASE 0x0D8B4000

#define HW_AHBPROT ((volatile u32 *)0xCD800064)
#define MEM_PROT    ((volatile u32 *)0xCD8B420A)
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
static const u8 es_identify_old[] =
{
    0x28, 0x03, 0xD1, 0x23
};

static const u8 es_identify_patch[] =
{
    0x00, 0x00
};

static int nand_ios_access_enabled = 0;

static void disable_memory_protection(void)
{
    *MEM_PROT = *MEM_PROT & 0x0000FFFF;
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


int nand_ios_enable_access(void)
{
    int found;

    if (nand_ios_access_enabled)
        return 1;

    if (*HW_AHBPROT != 0xFFFFFFFF)
        return -1;

    disable_memory_protection();

    found = apply_isfs_permission_patch();
    found += apply_es_identify_patch();

    if (found <= 0)
        return 0;

    nand_ios_access_enabled = 1;
    return found;
}
