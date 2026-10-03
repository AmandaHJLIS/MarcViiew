#ifndef MARCVIIEW_NAND_IOS_H
#define MARCVIIEW_NAND_IOS_H

/*
 * Enable read access to protected Wii NAND paths by patching the
 * currently running IOS in memory.
 *
 * This never writes an IOS file or NAND contents. The patch is lost
 * when IOS is unloaded/restarted.
 *
 * Return values:
 *   > 0  = permission patch applied
 *    0  = AHBPROT available, but expected IOS patch signature not found
 *   < 0  = AHBPROT is not available
 */
int nand_ios_enable_access(void);

#endif
