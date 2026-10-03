# MarcViiew

<img width="370" height="67" alt="MarcViiew logo" src="https://github.com/user-attachments/assets/a0dd8b27-50b7-4e6c-b303-47b6bf3110f0" />

MarcViiew is a free and open-source library management system (LMS) for the Nintendo Wii, designed for cataloguing, identifying, and viewing video game records.

MarcViiew combines Wii software discovery with library and information science principles. It identifies Wii software using title IDs, matches discovered titles against a local metadata database, displays catalogue metadata, and provides MARC 21 record viewing and ISO 2709 (.mrc) export.

The project is designed as an experimental library and information science project for libraries, librarians, library technicians, and LIS students. It explores how established cataloguing standards, controlled vocabularies, metadata practices, and digital-library workflows can be adapted to a Wii homebrew environment.

> **Branch note:** This README describes the **`imported-records-template` experimental branch**. It is ahead of `main` and is used for developing, testing, and validating catalogue, storage-discovery, imported-record, and Wii-specific functionality before changes are considered for the stable branch.

## Experimental Branch

The `imported-records-template` branch is a development and hardware-testing branch.

Its purpose is to provide a safe place to work on larger MarcViiew changes without changing the stable `main` branch. This includes experimental changes to the Wii application source, catalogue discovery, imported MARC records, metadata handling, and user-interface behaviour.

Current branch work includes:

* Imported MARC record management
* Automatic handling of successfully imported `.mrc` files
* NAND title discovery
* SD and USB digital-title discovery where a reliable Wii title ID is available
* Integration of discovered NAND/SD/USB titles into the catalogue
* Source and distribution metadata
* Duplicate-title source merging
* MARC 21 viewing and ISO 2709 export for discovered catalogue records
* Catalogue and record scrolling improvements
* Settings-based NAND and database reload functionality

Changes on this branch should be treated as **experimental until they have been built and tested on real Wii hardware**.

### Branch workflow

The branch is intentionally kept separate from `main` while development is underway.

* `main` represents the stable project baseline.
* `imported-records-template` is the experimental development branch.
* Wii application `.c` source changes are developed on this experimental branch first.
* Hardware testing is performed before experimental functionality is considered ready to merge.
* Finished changes can later be reviewed and selectively merged into `main`.

This branch is not intended to be a permanent fork of the project; it is a working area for developing the next stable iteration of MarcViiew.

## MARC 21

<img width="500" height="108" alt="MARC 21" src="https://github.com/user-attachments/assets/11d4aa98-af50-4a5e-9f29-7e3c70fe9637" />

MarcViiew uses the MARC 21 bibliographic format as the basis for its cataloguing and MARC record functionality.

MARC 21 is maintained by the Library of Congress Network Development and MARC Standards Office. MarcViiew is intended to provide a practical way to view and experiment with MARC 21 records for video game collections.

For authoritative documentation and current field definitions, please refer to the MARC 21 Format for Bibliographic Data provided by the Library of Congress.

For MarcViiew's project-specific MARC usage and field definitions, see the [MarcViiew MARC21 Profile](marcviiew-marc-profile.md).

## Wiiext

<img width="289" height="102" alt="Wii-Logo" src="https://github.com/user-attachments/assets/1077167d-ca67-45e8-bb7b-81f281b966ef" />

Wiiext is a controlled vocabulary and thesaurus developed specifically for describing Wii video games and related software.

MarcViiew uses Wiiext terms to provide consistent genre and form classification within its MARC 21 records. Wiiext is intended to provide a structured vocabulary for video game cataloguing, with preferred terms and thesaurus relationships designed for use within MarcViiew.

Wiiext is an unofficial extension/profile developed for use with MarcViiew. It is not an official Library of Congress extension, vocabulary, or standard, and is not endorsed or maintained by the Library of Congress.

For the controlled vocabulary and thesaurus profile, see the [MarcViiew Controlled Vocabulary and Thesaurus Profile](marcviiew-wiiext-profile.md).

## ViiewLib Integration

<img width="305" height="66" alt="ViiewLib" src="https://github.com/user-attachments/assets/2d3b7b36-bab9-48c5-ae43-f910357ccc1f" />

MarcViiew uses **ViiewLib**, a lightweight C library developed separately for MARC 21 record handling and ISO 2709 encoding and decoding.

ViiewLib provides the underlying MARC 21 and ISO 2709 functionality used by MarcViiew while allowing the Wii-specific catalogue and user-interface code to remain separate from the MARC implementation.

Current integration includes:

* MARC 21 record creation and manipulation
* Control fields, variable fields, indicators, and subfields
* Repeated MARC fields with preserved field order
* MARC 21 field lookup
* ISO 2709 encoding and decoding
* MARC 21 ↔ ISO 2709 round-trip support
* Validation of malformed and truncated ISO 2709 records
* Encoding of individual game records into separate `.mrc` files
* devkitPPC/Wii integration
* MARC 21 and ISO 2709 hardware testing on Nintendo Wii

The current data flow is:

```text
MarcViiew MARC database
        │
        ▼
MarcViiew text parser
        │
        ▼
    ViiewLib API
        │
        ├── MARC 21 records
        │
        └── ISO 2709 (.mrc)
```

The `.mrc` files generated by MarcViiew are ISO 2709 export files. The local `marcviiew_marc.txt` database remains the source used by MarcViiew's normal MARC record viewer and MARC21 search functionality.

ViiewLib remains under active development and does not currently claim complete MARC 21 or ISO 2709 standards conformance or production readiness.

**ViiewLib:**

https://github.com/AmandaHJLIS/ViiewLib

## Game Metadata

MarcViiew currently uses a locally stored database generated from GameTDB data. The database contains information for over 10,000 Wii game records and provides the metadata used by MarcViiew to identify and describe known games.

GameTDB provides information including game IDs, titles, publishers, developers, release dates, regions, genres, and synopses. The GameTDB data is processed into a format that can be used by MarcViiew on the Wii.

The database is used for **metadata matching**, not as a requirement for every discovered title to be recognised. Wii titles discovered from NAND or storage devices may still appear when no matching database record exists; in those cases MarcViiew retains the discovered title ID and available discovery metadata.

## Catalogue Discovery

The experimental branch can discover software through several Wii storage mechanisms.

### WBFS storage

MarcViiew scans:

```text
sd:/wbfs/
usb:/wbfs/
```

For recognised WBFS entries, the Wii Game ID is extracted from the filename and used to match the local metadata database.

### NAND

MarcViiew can enumerate installed Wii titles through the Wii ES title system.

NAND discovery is intentionally broad. It can identify installed compatible software such as:

* WiiWare
* Virtual Console titles
* Wii Channels
* Other compatible installed title types
* Homebrew WAD-installed software

When a discovered NAND title matches the local metadata database, its normal catalogue metadata is populated. Unknown title IDs can still be displayed with limited metadata rather than being silently discarded.

NAND entries are marked with:

```text
Source: NAND
```

### SD/USB digital-title storage

MarcViiew also checks the Nintendo SD-style digital-title structure:

```text
sd:/private/wii/title/<TITLE ID>/
usb:/private/wii/title/<TITLE ID>/
```

A title directory is only considered when it has the expected four-character title ID and a `content.bin` file.

This allows transferred WiiWare, Virtual Console, and compatible channel software to be discovered when its title ID can be reliably obtained from the directory structure.

MarcViiew does **not** attempt to guess digital titles from arbitrary filenames or folders.

### Duplicate discovery and Source

A title discovered through multiple storage mechanisms is represented as one catalogue entry rather than several duplicates.

For example:

```text
Title: Muscle March
Game ID: WMMP
Source: SD + NAND
Distribution: DIGITAL
```

**Source** describes where MarcViiew discovered the software.

**Distribution** describes the release/distribution form represented by the catalogue metadata.

These are deliberately separate concepts. A digital title installed on NAND can therefore have `Source: NAND` and `Distribution: DIGITAL`.

## Imported MARC Records

The experimental branch includes an imported-record workflow for external `.mrc` files.

Incoming records are treated separately from the persistent imported-record collection:

```text
sd:/marcviiew_import/
        │
        ▼
   import process
        │
        ▼
sd:/marcviiew/imported/
```

Successfully imported records are moved into the persistent imported collection. Duplicate incoming records can be detected using the imported record's game ID and/or filename, preventing an already-imported record from being retained as a second copy.

This functionality is still experimental and is subject to further refinement.

## MarcViiew Cataloguing Rules

MarcViiew uses the MARC 21 bibliographic format as the foundation of its cataloguing system. Where source metadata requires normalisation or adaptation for use within MarcViiew, project-specific cataloguing conventions may be applied.

These conventions are intended to provide consistent representation of video game metadata within MarcViiew and do not replace the official MARC 21 standards.

### Metadata

* GameTDB is currently used as the primary source for known game metadata.
* Wii Game IDs are used as the primary identifier for games.
* Dates supplied by GameTDB are normalised to YYYY-MM-DD.
* Publisher and developer information is retained from the source metadata.
* GameTDB genre terms are mapped to preferred Wiiext terms where an appropriate term exists.
* Discovery source is maintained separately from release/distribution type.
* Unknown metadata is not inferred when MarcViiew cannot establish it reliably.

### MARC 21

* `001` contains the Wii Game ID.
* `245` contains the game title.
* Non-filing characters are accounted for in the 245 second indicator.
* `264` contains publication/distribution information and the release date used by MarcViiew.
* `300` describes physical Wii disc information where the MarcViiew profile calls for physical-description data.
* `338` identifies the carrier for physical Wii releases where applicable under the MarcViiew profile.
* `500` contains the game synopsis.
* `542` contains selected metadata provenance information.
* `655` contains Wiiext genre/form terms.
* `655 $2` identifies Wiiext as the source vocabulary.

Digital releases are not automatically assigned physical-description fields merely because they are present in the Wii catalogue. Physical-description treatment is controlled by the MarcViiew MARC profile.

For the complete field definitions and usage rules, see the [MarcViiew MARC21 Profile](marcviiew-marc-profile.md).

These conventions may change as MarcViiew's cataloguing model develops.

## Current Features

* Detects Wii software stored on SD and USB WBFS storage
* Discovers compatible installed titles from Wii NAND
* Discovers supported SD/USB digital-title structures when a reliable title ID is available
* Uses Wii Game IDs as the primary catalogue identifier
* Matches discovered titles against the local MarcViiew metadata database
* Merges duplicate discoveries from multiple sources
* Displays title, ID, platform, region, release date, publisher, developer, genre, series, source, distribution, and synopsis metadata where available
* Scrollable game information
* Nintendo Wii Homebrew Channel support
* Classic Controller support
* Search functionality for locating games
* MARC 21 record viewing
* MARC 21 search functionality
* Database and storage-device reload functionality
* NAND reload functionality
* Imported MARC record management
* MARC 21 text-database parsing
* ISO 2709 `.mrc` encoding
* Encoding of individual game records to `.mrc`
* ViiewLib MARC 21 and ISO 2709 integration
* Wiiext controlled vocabulary and thesaurus support

## Current Development Focus

The major catalogue-discovery and beta feature work for this experimental branch is substantially implemented and has been tested on real Wii hardware.

Current development areas include:

* Further MARC 21 validation and interoperability work
* Further ISO 2709 compatibility and edge-case testing
* Further imported-record workflow refinement
* Catalogue and Wii user-interface refinement
* Additional metadata research and cataloguing-profile research
* Wii performance and memory optimisation
* Continued hardware regression testing
* Reviewing experimental changes before they are considered for `main`

## Long-Term Planned Features

The following ideas are planned as longer-term research and development areas rather than requirements for the current beta or initial stable release.

### Wii User Interface Research

* Further refinement of MarcViiew's Wii user interface and reusable UI components
* Research into UI architecture and interaction patterns used by newer Wii homebrew projects, including **RiftWii**, to inform future MarcViiew UI development
* Improved screen layout, navigation, text rendering, scrolling, and input handling

### Wii/WiiWare File Structure Research

* A file-structure viewer for exploring Wii and WiiWare content
* Directory and file inspection, file sizes, binary/hexadecimal inspection, and other low-level file metadata
* Tools for documenting unknown file structures, formats, relationships, and research observations
* Comparative analysis of original and modified game files to support reproducible research
* Research into WiiWare game file formats and modification workflows, with an initial focus on documenting and experimenting with individual titles
* Application of library and information science principles to the preservation, organisation, metadata, and documentation of Wii and WiiWare reverse-engineering research

These projects are intended to complement MarcViiew's existing catalogue and LIS focus. They are exploratory long-term goals and are not part of the current stable-release requirements.

## Beta Status

MarcViiew is currently in **beta development**.

The current experimental state corresponds to the **0.6.0-beta development milestone**, following the feature-focused 0.5.0-beta milestone.

The `imported-records-template` branch is an experimental development branch rather than a stable release. Features on this branch should be considered provisional until they have been built and tested on real Wii hardware.

MarcViiew contains functional MARC 21 and ISO 2709 support through ViiewLib, including `.mrc` encoding and Wii hardware testing. However, the MARC 21 and ISO 2709 implementation is still under active development and should not be considered a complete implementation of either standard or production-ready LMS software.

## Setup

The experimental branch uses the same general Wii application setup as the stable project.

1. Build the Wii application using the project's devkitPPC/libogc development environment.

2. Copy the application to:

   ```text
   sd:/apps/marcviiew/
   ```

3. Copy the database files to the root of the SD card:

   ```text
   sd:/marcviiew_games.txt
   sd:/marcviiew_marc.txt
   ```

4. Connect any SD/USB storage containing the Wii software you want MarcViiew to scan.

5. Open the Homebrew Channel and launch MarcViiew.

Because this branch contains experimental source changes, a successful build should be followed by hardware testing before the branch is treated as stable.

## Development Notes

MarcViiew is a Wii homebrew/LIS project rather than a production LMS.

The project intentionally separates:

* software discovery
* catalogue metadata
* release/distribution information
* MARC 21 records
* ISO 2709 encoding
* imported-record storage
* Wii user-interface code

This separation allows individual parts of the catalogue workflow to be developed and tested without making the Wii application responsible for assumptions that belong in the metadata or MARC generation pipeline.

The experimental branch is specifically where these boundaries are tested and refined before stable integration.

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

The MIT License applies to MarcViiew's original source code; third-party data and resources remain subject to their respective terms.

## Credits

GameTDB — for providing Wii game information and metadata.

Library of Congress — for the MARC 21 bibliographic standards.

ISO/TC 46 — for ISO 2709, Information and documentation — Format for information exchange.

Lilyflower - for extensively testing MarcViiew on VWii, and providing external .mrc files.
