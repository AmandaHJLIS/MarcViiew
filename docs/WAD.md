# WAD Files

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

