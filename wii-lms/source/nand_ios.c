#include <gccore.h>
#include <string.h>

#include "nand_ios.h"

#define MEM_REG_BASE 0x0D8B4000

#define HW_AHBPROT ((volatile u32 *)0xCD800064)
#define MEM_PROT    ((volatile u16 *)0xCD8B420A)
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

static int nand_ios_access_enabled = 0;

static void disable_memory_protection(void)
{
    *MEM_PROT = 0;
}

static int apply_isfs_permission_patch(void)
{
    u8 *ptr = (u8 *)*IOS_ENTRY_PTR;
    u8 *end = IOS_PATCH_END;
    const size_t patch_size = sizeof(isfs_permissions_patch);
    int found = 0;

    /*
     * IOS publishes the start of its code in MEM1 at 0x80003134.
     * Do not assume every IOS starts at the same address; WiiFlow and
     * USB Loader GX use this pointer for their runtime patch scanner.
     */
    if ((u32)ptr < 0x90000000 || (u32)ptr >= (u32)end)
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
    if (*HW_AHBPROT != 0xFFFFFFFF)
        return -1;

    disable_memory_protection();

    found = apply_isfs_permission_patch();

    if (found <= 0)
        return 0;

    nand_ios_access_enabled = 1;
    return 1;
}
