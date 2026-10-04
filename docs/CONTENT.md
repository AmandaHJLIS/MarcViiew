# Wii Content

**Last Updated:** 3rd of October 2026

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

