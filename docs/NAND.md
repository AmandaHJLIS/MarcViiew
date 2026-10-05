# Wii NAND

## 2. Wii NAND filesystem overview

The Wii contains a 512 MiB NAND flash device used for system software, installed channels, WiiWare, Virtual Console software, saves, and system configuration.

The NAND filesystem exposed through IOS is not an ordinary FAT filesystem. Wii system memory uses Nintendo's encrypted filesystem and an IOS filesystem service.

Important top-level locations include:

    /
    ├── dev/
    ├── import/
    ├── meta/
    ├── shared1/
    ├── shared2/
    ├── sys/
    ├── ticket/
    ├── title/
    └── tmp/

### /title

Contains installed title data.

Typical title paths have the form:

    /title/<high 32 bits>/<low 32 bits>/

Example:

    /title/00010001/4E414C45/

The first directory is the high portion/title type of the 64-bit title ID. The second is the lower portion.

The lower 32 bits are often represented as four ASCII characters. For example:

    4E414C45 = NALE

MarcViiew deliberately preserves the raw hexadecimal title path rather than assuming every title is a normal disc game.

### /ticket

Tickets are associated with installed titles and contain authorization/key information used by the Wii title system.

### /shared1

Contains shared content used by multiple titles.

The important file is:

    /shared1/content.map

Shared .app files use names such as:

    /shared1/000000xx.app

The shared filename does not directly equal the TMD content ID. The expected content is identified using its SHA-1 hash.

### /sys

Contains IOS/system-management files and other system state. These files are not interchangeable with title content.

### Filesystem implementation

The Wii NAND uses an internal filesystem structure rather than a normal PC filesystem. Research into its underlying format describes filesystem-tree, allocation, and filesystem-metadata structures.

For MarcViiew, the practical rule is to use IOS/ISFS APIs rather than attempting to manipulate raw NAND structures.

## 3. MarcViiew NAND browser

The experimental NAND browser was created to investigate installed Wii software without requiring the software to be recognised by the normal game metadata database.

It can expose paths such as:

    /title/
    /title/<type>/
    /title/<type>/<id>/
    /title/<type>/<id>/content/
    /title/<type>/<id>/data/

A NAND-discovered title does not require a GameTDB entry.

### Read-only design

The intended operations are:

- enumerate directories;
- obtain file statistics;
- open files read-only;
- read metadata;
- inspect TMD records;
- correlate known local files with TMD records.

The browser must not:

- write to NAND;
- replace title files;
- modify tickets or TMDs;
- modify content.map;
- modify IOS files on NAND;
- install or uninstall titles.

## 4. Why NAND access required IOS work

The Wii IOS filesystem service applies access restrictions. A normal homebrew application cannot necessarily enumerate every title-owned directory simply by calling ISFS functions.

MarcViiew's experimental access layer therefore uses a RAM-only IOS patching approach.

The important distinction is:

> MarcViiew patches the currently running IOS in memory; it does not patch an IOS file stored on NAND.

The experimental implementation has used patches associated with:

- ISFS permission handling;
- ES_Identify;
- hash/signature-related checks;
- ES_SetUID.

The exact patch set is implementation-specific and sensitive. Small changes have previously caused the NAND browser to lose access to otherwise readable directories.

### Observed behaviour

During hardware testing:

1. A basic ISFS directory read could show directories such as content and data but report zero entries.
2. This did not mean the directories were empty; access to their contents was being denied.
3. Runtime IOS permission work allowed the browser to access title-owned files.
4. WiiWare and VC titles could then expose title.tmd, and some titles exposed additional .app files.
5. Experiments involving extra access to /shared1/content.map caused NAND-browser regressions and were rolled back.

Therefore, successful access to one NAND path must not be treated as proof that every other path can safely be probed.

## 20. Current implementation cautions

### Do not casually change UID handling

An attempt to refresh the current title UID with ES_SetUID during NAND-browser initialization caused the previously working browser to become inaccessible.

The experiment was reverted.

The experimental branch now has a narrower read-only parser. It reads the existing 28-byte map records into memory, closes the NAND file, and does not write anything back. It should still be treated as a hardware research feature and tested carefully.

### Do not casually modify /shared1/content.map

A TMD-viewer experiment that opened and parsed /shared1/content.map caused title directories to become inaccessible or marked unknown.

The experiment was reverted.

### Do not assume a failed ES operation means the directory is inaccessible

The browser has previously been able to obtain directory information even when title identification failed.

Filesystem access and ES title identification should remain conceptually separate.

### Preserve the working baseline

The user's locally stored working source should be treated as a golden backup before further NAND experiments.



### Shared-content inventory export

While browsing /shared1/, pressing 1 creates sd:/marcviiew/shared1_inventory.txt containing the visible entry names, basic type, and file size. This is an SD-side report; it does not copy or modify NAND.
