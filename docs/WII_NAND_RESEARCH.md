
# Wii NAND, WiiWare and Virtual Console Research Notes

This document records what has been established during MarcViiew development and real Wii hardware testing, together with documented Wii file-format information relevant to WiiWare, Virtual Console (VC), channels, and installed title content.

This is research documentation, not a claim that every part of the Wii title system is fully understood. Hardware observations, documented format facts, strong inferences, and hypotheses should be kept distinct.

> **Important:** MarcViiew's NAND browser is intended to be read-only. It must not modify NAND files, IOS files, title metadata, tickets, or filesystem structures.

## 1. Scope

The research began because installed WiiWare and VC titles do not always expose their executable/content files as ordinary files beneath:

    /title/<title type>/<title id>/content/

A title may expose title.tmd while the content records described by that TMD do not appear as visible .app files in the same directory.

This is not necessarily an error in the TMD. The TMD describes title contents logically; physical storage depends on the content type and Wii title/filesystem rules.

The research therefore covers:

- the Wii NAND filesystem as exposed through IOS/ISFS;
- title directories and title IDs;
- TMD files and content records;
- tickets and title authorization;
- normal, DLC, and shared content;
- /shared1/content.map;
- WiiWare and VC title storage;
- SD-transferred content.bin files;
- WADs as an offline way to study title/content relationships;
- what MarcViiew can safely observe without making assumptions about inaccessible content.

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

## 5. Title identity and ES

A Wii title is identified by a 64-bit title ID.

MarcViiew commonly represents it as:

    high 32 bits / low 32 bits

Example:

    00010001 / 4E414C45

The ES title-management system is involved in identifying titles and their metadata.

The experimental browser uses ES title/TMD information when entering title-owned areas. ES_Identify was part of the access strategy, but its failure is not itself treated as proof that ISFS cannot read a directory.

Filesystem access and ES title identification should remain conceptually separate.

## 6. TMD (Title Metadata)

The TMD is one of the most useful structures discovered during MarcViiew's NAND research.

A TMD describes a title and its installed content records. It contains information including:

- title ID;
- title version;
- required system version/IOS;
- number of content records;
- boot index;
- content IDs;
- content indices;
- content types;
- content sizes;
- SHA-1 hashes.

Installed title TMDs are commonly encountered at:

    /title/<type>/<id>/content/title.tmd

### TMD fields used by MarcViiew

| Offset | Size | Meaning |
|---|---:|---|
| 0x18C | 8 | Title ID |
| 0x1D8 | 4 | Access rights |
| 0x1DC | 2 | Title version |
| 0x1DE | 2 | Number of contents |
| 0x1E0 | 2 | Boot index |
| 0x1E2 | 2 | Minor/reserved field |
| 0x1E4 | - | First content record |

Each content record is 0x24 (36) bytes.

| Record offset | Size | Meaning |
|---|---:|---|
| 0x00 | 4 | Content ID |
| 0x04 | 2 | Content index |
| 0x06 | 2 | Content type |
| 0x08 | 8 | Content size |
| 0x10 | 20 | SHA-1 hash |

Equivalent C-style structure:

    u32 content_id;
    u16 index;
    u16 type;
    u64 size;
    u8  hash[20];

A useful TMD-size sanity check is:

    0x1E4 + (number of contents × 0x24)

For seven records:

    0x1E4 + (7 × 0x24) = 736 bytes

This exactly matched the 736-byte TMD observed while examining Cave Story.

## 7. Boot index

The TMD boot index is a content index, not a content ID.

For example:

    Boot index: 6

means that the content record whose index is 6 is designated as boot content.

Therefore:

    Content ID != Content index

MarcViiew's TMD viewer currently marks the matching record as BOOT and content index 0 as INDEX0.

These are UI interpretations of existing TMD fields, not additional data stored in the TMD.

## 8. TMD content types

Documented common content types include:

| Type | Meaning |
|---|---|
| 0x0001 | Normal content |
| 0x4001 | DLC content |
| 0x8001 | Shared content |

The 0x4001 and 0x8001 types are particularly important when a record does not appear as an ordinary title-local .app file.

Documented Wii title-format information describes 0x4001 as DLC content that can be absent while installation can still complete, and 0x8001 as shared content installed under /shared1 rather than the title's content directory.

MarcViiew's UI labels have a documented format basis, but they should be understood as descriptions of the TMD type, not assumptions about every application's internal purpose.

## 9. Normal content versus physical storage

A TMD record should not automatically be translated into:

    /title/<type>/<id>/content/<content id>.app

That path is appropriate to investigate for normal title-local content, but it is not a universal rule.

MarcViiew safely probes the title-local .app path for NORMAL records.

For Cave Story, the TMD contained normal content records, but the corresponding title-local .app files were not exposed by the browser.

Possible explanations include:

- content exists elsewhere;
- content is handled through another title-management mechanism;
- the content is not represented by a straightforward visible filename;
- the browser's filesystem view is incomplete;
- the title has a different storage arrangement.

The evidence is not sufficient to choose one explanation for all WiiWare/VC titles.

## 10. Shared content

For a shared record:

    TMD type = 0x8001

the content is stored in:

    /shared1/

The shared filename is not simply the TMD content ID.

The documented relationship is effectively:

    TMD SHA-1
         |
         v
    /shared1/content.map
         |
         v
    shared .app filename

This is why looking for /shared1/<TMD content ID>.app is incorrect.

### MarcViiew shared-content experiment

MarcViiew briefly experimented with reading /shared1/content.map from the TMD viewer.

That experiment was rolled back because accessing the shared-content map caused NAND browsing regressions, including directories becoming inaccessible or marked unknown.

Therefore:

> The shared-content relationship is established by format documentation, but MarcViiew does not yet have a reliable shared-content reader.

The earlier experimental content.map reader should not be treated as a safe implementation.

## 11. Cave Story observation

Observed TMD information:

    Path: /title/80018001/57435645/content/title.tmd
    File size: 736 B
    Boot index: 6
    Number of content records: 7

The record count is independently confirmed:

    (736 - 0x1E4) / 0x24 = 7

Several records were observed as NORMAL content with large sizes and SHA-1 hashes.

However, corresponding title-local .app probes were all reported as missing.

The important conclusion is not that the TMD is broken.

The conclusion is:

> A valid TMD can describe content that is not exposed as a corresponding title-local .app file by MarcViiew's current ISFS view.

The TMD therefore gives us a logical content inventory even when physical files cannot yet be located.

## 12. Muscle March observation

Muscle March provided a contrasting example.

Its title-owned directories exposed files including:

    content/
        title.tmd
        <one or more .app files>

    data/
        save.dat
        banner.bin

This demonstrated that WiiWare/channel titles can expose ordinary content .app files through the title's content directory.

Comparison:

    Muscle March
        TMD
        visible .app content
        data/save/banner files

    Cave Story
        TMD
        no corresponding title-local .app files visible
        data/save/banner files

The difference is one of the main unresolved questions.

## 13. data/ contents

Title data directories can contain saves, banners, configuration, or other title-specific data.

Observed examples include:

    data/
        save.dat
        savedata.dat
        banner.bin

Exact filenames and meanings are title-dependent.

A file such as banner.bin should not automatically be interpreted as executable content.

MarcViiew currently treats the NAND browser primarily as a structural viewer rather than attempting to interpret every binary file.

## 14. content.bin and SD-transferred channels

The Wii SD menu can store transferred channels in:

    /private/wii/title/<low title ID>/

with a content.bin file.

This is a different storage/container representation from an installed title under /title/.

Documented content.bin research shows that it contains encrypted information and an embedded backup-WAD-style structure.

This makes content.bin useful for offline research into:

- title metadata;
- content records;
- content data;
- certificate/signature structures.

MarcViiew already knows about SD/USB digital-title discovery, but parsing content.bin is a separate future research task.

## 15. WAD files as an offline research tool

WADs are particularly useful because they package title-related structures into a form that can be examined on a PC without repeatedly accessing a live Wii NAND.

A WAD can contain:

- ticket;
- TMD;
- certificate chain;
- content data;
- an included-content bitfield.

The included-content bitfield identifies which TMD content records are present in the WAD.

Content data follows the order of the TMD content records, and decrypted content is checked against the SHA-1 in its corresponding TMD record.

This allows offline questions such as:

    TMD record
        |
        v
    Is this content included?
        |
        v
    What is its index, type, size and SHA-1?
        |
        v
    Where is the corresponding encrypted content in the WAD?

This should be preferred over repeated live-NAND experimentation when possible.

## 16. WiiWare and Virtual Console

WiiWare and Virtual Console titles are installed Wii titles rather than ordinary disc-image files.

Their title IDs identify them within the Wii title system.

A useful broad model is:

    Title ID
       |
       +-- Ticket
       |
       +-- TMD
       |     |
       |     +-- content records
       |           +-- content ID
       |           +-- index
       |           +-- type
       |           +-- size
       |           +-- SHA-1
       |
       +-- title-owned content
       |
       +-- shared content, when applicable
       |
       +-- title-specific data

Two titles both described as WiiWare or VC do not necessarily have identical physical layouts.

The TMD is therefore the safest starting point for investigating an individual title.

## 17. What MarcViiew currently knows

At catalogue level, MarcViiew can use a title ID to identify known software through its local metadata databases.

At NAND-research level, MarcViiew can additionally observe:

- raw title path;
- title type/high ID;
- title ID/low ID;
- title-owned directories;
- files exposed through ISFS;
- file sizes;
- TMD existence;
- TMD content count;
- TMD boot index;
- content IDs;
- content indices;
- content types;
- TMD content sizes;
- TMD SHA-1 hashes;
- whether a tested NORMAL-content .app path is visible.

The TMD viewer can therefore describe a title's logical content inventory even when physical files cannot yet be located.

## 18. What is still unknown

### Why do some TMD records have no visible title-local .app?

Cave Story demonstrates the phenomenon.

Possible explanations include storage/installation details, shared or special content handling, IOS/title-management behaviour, or limitations in the current filesystem view.

No single explanation should be assumed for all titles.

### Which content records are physically stored where?

The desired mapping is:

    TMD record
       |
       v
    physical storage location
       |
       v
    physical filename
       |
       v
    actual size and SHA-1

Normal and shared content can follow different rules.

### What exactly is inside each .app?

An .app file is a title-content file/container, not a guarantee that its first bytes are directly a DOL executable.

Different titles can use different internal formats and compression/encryption arrangements.

This should be investigated from offline samples first.

### How do VC titles differ internally?

VC titles can contain emulated-platform-specific data and executables. The TMD describes the installed Wii title's content records, but interpreting the contents requires another layer of format research.

### What does every content type/flag combination mean?

The common types are documented, but MarcViiew should not assume the four hexadecimal digits explain every possible content-storage situation.

### What additional metadata is available in TMDs?

The current viewer focuses on content records. Other useful TMD fields include:

- title version;
- system version;
- region;
- group ID;
- title type;
- access rights;
- ratings;
- vWii indicator.

These can be exposed later without touching additional NAND paths.

## 19. Safe research strategy

Preferred workflow:

1. Document the format first.
2. Analyse WADs and extracted files on a PC where possible.
3. Compare multiple titles offline.
4. Form a specific hypothesis.
5. Make one small MarcViiew change.
6. Keep NAND access read-only.
7. Avoid /shared1 experiments until their implementation is well understood.
8. Test once on hardware.
9. Preserve the known-good source if an experiment regresses NAND access.
10. Record the observation regardless of whether the experiment succeeds.

The goal is to avoid turning every unanswered format question into a hardware experiment.

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

## 21. Current research model

    Title ID
       |
       +----------------------+
       |                      |
     Ticket                  TMD
       |                      |
       |            +---------+---------+
       |            |                   |
       |      Content records      Title metadata
       |            |
       |       +----+----+----+
       |       |         |    |
       |     normal     DLC shared
       |       |         |    |
       |       |         |  /shared1
       |       |         |  content.map
       |       |         |
       |       +----+----+
       |            |
       |       title content
       |
       +-- title authorization/key data

This model is intentionally incomplete.

The major missing piece is the complete mapping from:

    TMD content record
            |
            v
    actual installed content
            |
            v
    actual storage location
            |
            v
    internal content format

## 22. Evidence levels

### Confirmed by format documentation

Examples:

- TMD offsets;
- TMD content-record size;
- boot-index meaning;
- documented content-type values;
- shared-content storage rules;
- WAD content ordering.

### Confirmed by MarcViiew hardware observation

Examples:

- a specific title path exists;
- title.tmd is readable;
- a particular file is visible;
- a particular file size;
- a title's TMD record count;
- a NORMAL-content .app probe succeeds or fails.

### Strong inference

A conclusion supported by both documentation and multiple observations, but not exhaustively tested.

### Hypothesis

A proposed explanation that still requires evidence.

MarcViiew should avoid presenting hypotheses as established file-format facts.

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

## 24. References

- WiiBrew — Title metadata / TMD structure:
  https://wiibrew.org/wiki/Tmd_file_structure
- WiiBrew — Title system and content types:
  https://wiibrew.org/wiki/Title
- WiiBrew — Shared content:
  https://www.wiibrew.org/wiki//shared1
- WiiBrew — Flash filesystem:
  https://wiibrew.org/wiki/Flash_filesystem
- WiiBrew — Wii NAND:
  https://wiibrew.org/wiki/Hardware/NAND
- WiiBrew — WAD files:
  https://wiibrew.org/wiki/WAD_files
- WiiBrew — content.bin:
  https://wiibrew.org/wiki/Content.bin_file_structure

These references describe Wii formats and filesystem behaviour; they are not claims about the current MarcViiew implementation.

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
