# Wii Content

**Last Updated:** 5th of October 2026

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

The current branch may contain a read-only research parser; its results must still be treated as hardware research data, not as permission to write or alter content.map.

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



## 11. Resolving a TMD record to a physical .app

MarcViiew's long-term goal is to turn a TMD content record into a physical NAND location.

The two common paths are:

    NORMAL (0x0001)
        TMD content ID
            |
            v
        /title/<type>/<id>/content/<content ID>.app

    SHARED (0x8001)
        TMD SHA-1
            |
            v
        /shared1/content.map
            |
            v
        /shared1/<shared identifier>.app

The shared path is necessary because a shared-content filename does not have to equal the TMD content ID.

The normal path should be treated as the first physical-location probe, while the shared path requires SHA-1 resolution.

## 12. Hardware observations

Three useful title tests have now been recorded:

### Cave Story

The observed TMD contains 7 normal (0x0001) records and no shared (0x8001) records. Therefore content.map is not expected to resolve any of its TMD records.

### The Legend of Zelda Ocarina of Time VC

The observed TMD contains 7 records, including 3 shared (0x8001) records. All 3 shared records produced SHA-1 matches in /shared1/content.map.

### Super Mario 64 VC

The observed TMD contains 7 records, including 3 shared records. All 3 shared records produced SHA-1 matches in /shared1/content.map.

These observations are hardware-test results for the installed titles and should remain separate from general format claims.

