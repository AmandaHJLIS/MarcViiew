#include <gccore.h>
#include <ogc/machine/processor.h>
#include <string.h>

#include "nand_ios.h"

#define MEM_REG_BASE 0x0D8B4000
#define MEM_PROT     (MEM_REG_BASE + 0x20A)

#define IOS_PATCH_START_PTR ((u8 *)(*((volatile u32 *)0x80003134)))
#define IOS_PATCH_END_PTR   ((u8 *)0x94000000)

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

static int nand_ios_access_enabled = 0;

static void disable_memory_protection(void)
{
    write32(
        MEM_PROT,
        read32(MEM_PROT) & 0x0000FFFF
    );
}

static int apply_isfs_permission_patch(void)
{
    u8 *ptr = IOS_PATCH_START_PTR;
    u8 *end = IOS_PATCH_END_PTR;
    const size_t patch_size = sizeof(isfs_permissions_patch);
    int found = 0;

    if (ptr == NULL || ptr >= end)
        return 0;

    while (ptr + sizeof(isfs_permissions_old) <= end)
    {
        if (memcmp(
                ptr,
                isfs_permissions_old,
                sizeof(isfs_permissions_old)
            ) == 0)
        {
            size_t i;

            for (i = 0; i < patch_size; ++i)
                ptr[i] = isfs_permissions_patch[i];

            DCFlushRange(ptr, patch_size);
            ICInvalidateRange(ptr, patch_size);

            found++;
            ptr += patch_size;
        }
        else
        {
            ptr++;
        }
    }

    return found;
}

int nand_ios_enable_access(void)
{
    int found;

    if (nand_ios_access_enabled)
        return 1;

    /*
     * AHBPROT is intentionally required. Without it, Broadway cannot
     * safely modify the running IOS memory region.
     */
    if (!AHBPROT_DISABLED)
        return -1;

    disable_memory_protection();

    found = apply_isfs_permission_patch();

    if (found <= 0)
        return 0;

    nand_ios_access_enabled = 1;
    return 1;
}
