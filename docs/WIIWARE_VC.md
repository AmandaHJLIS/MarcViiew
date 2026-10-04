# WiiWare and Virtual Console

**Last Updated:** 3rd of October 2026

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

