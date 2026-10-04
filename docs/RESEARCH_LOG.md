# MarcViiew NAND Research Log

**Last Updated:** 3rd of October 2026

This document preserves the implementation and hardware-testing history that led to the current NAND research model. Failed experiments are retained because they explain what should not be repeated casually.

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

### Do not casually read /shared1/content.map

A TMD-viewer experiment that opened and parsed /shared1/content.map caused title directories to become inaccessible or marked unknown.

The experiment was reverted.

### Do not assume a failed ES operation means the directory is inaccessible

The browser has previously been able to obtain directory information even when title identification failed.

Filesystem access and ES title identification should remain conceptually separate.

### Preserve the working baseline

The user's locally stored working source should be treated as a golden backup before further NAND experiments.

## 23. Useful future offline research

Before adding more NAND functionality, investigate the following using extracted files, WADs, and content samples on a PC:

- parse several WiiWare TMDs;
- parse several VC TMDs;
- compare content counts and types;
- compare boot indices;
- compare content IDs and indices;
- compare TMD hashes against extracted content;
- inspect WAD included-content bitfields;
- inspect shared-content records;
- study content.map format from known tools/source;
- inspect .app headers and internal structures;
- compare titles with visible .app files against titles without them;
- determine whether the difference is title-specific or systematic.

Only after these relationships are understood should MarcViiew's live NAND browser be extended.

## 25. Research status

**Status: ongoing / experimental**

Current knowledge is sufficient to say that:

- Wii installed software is represented through the Wii title system;
- the TMD provides a logical inventory of title contents;
- content records contain IDs, indices, types, sizes, and SHA-1 hashes;
- the boot index identifies a content index;
- shared content is handled differently from ordinary title-local content;
- WiiWare/VC titles can have substantially different visible NAND layouts;
- MarcViiew can use TMD information to document content it cannot yet physically locate;
- offline WAD/content analysis is a safer next step than repeatedly experimenting with live NAND access.

The exact physical-storage mapping for every observed WiiWare/VC title remains an active research question.

