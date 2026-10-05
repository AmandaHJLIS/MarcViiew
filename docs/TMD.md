# TMD — Title Metadata

**Last Updated:** 5th of October 2026

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



## 9. Physical content resolution

The TMD provides the logical identity of each content object, while the content type determines how MarcViiew should attempt to locate the physical file.

For a NORMAL record (0x0001), the first location to investigate is the title's own content directory:

    /title/<type>/<id>/content/<content id>.app

For a SHARED record (0x8001), the content ID is not used as the /shared1/ filename. MarcViiew instead uses the record's SHA-1 with /shared1/content.map to resolve the shared-content identifier.

Conceptually:

    NORMAL:
        TMD content ID -> title content/<ID>.app

    SHARED:
        TMD SHA-1 -> /shared1/content.map -> shared/<ID>.app

This is a physical-location investigation model, not a claim that every NORMAL record is currently visible through MarcViiew's live ISFS browser.

## 10. Hardware validation: mixed normal/shared TMDs

The first successful live tests demonstrate that the TMD/content.map relationship is useful for real installed VC titles.

Observed:

- The Legend of Zeldia Ocarina of Time VC: 7 TMD content records; 3 records were type 0x8001 and all 3 SHA-1 values matched entries in /shared1/content.map.
- Super Mario 64 VC: 7 TMD content records; 3 shared records were observed and all 3 matched /shared1/content.map.
- Cave Story: 7 TMD content records; all observed records were type 0x0001, so there were no shared TMD records to resolve through content.map.

These results demonstrate that the shared-content lookup should only be applied to records whose TMD type is 0x8001.
