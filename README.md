# MarcViiew Experimental branch

<img width="249" height="233" alt="MarcViiewExperimental" src="https://github.com/user-attachments/assets/8a00d282-8c43-45c1-a35a-62942503e5f0" />

MarcViiew is a free and open-source library management system (LMS) for the Nintendo Wii, designed for cataloguing, identifying, and viewing video game records.

MarcViiew combines Wii software discovery with library and information science principles. It identifies Wii software using title IDs, matches discovered titles against a local metadata database, displays catalogue metadata, and provides MARC 21 record viewing and ISO 2709 (.mrc) export.

MarcViiew also includes a NAND (read only) navigational tool for viewing data regarding Wiiware and Virtual console games, not limited but also including Wii channels and 3rd party channels (Homebrew, usbloadergx, etc). This navigational tool is meant to apply WDRD Wii-specific metadata framework standards to digital only resources to describe relevant resources, including installed representations on the Wii's NAND.

The project is designed as an experimental library and information science project for libraries, librarians, library technicians, and LIS students. It explores how established cataloguing standards, controlled vocabularies, metadata practices, and digital-library workflows can be adapted to a Wii homebrew environment.

For additional information, please visit:
https://amandahjlis.github.io/

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

## Beta Status

MarcViiew is currently in **beta development**.

The current experimental state corresponds to the upcoming **0.8.0-beta development milestone**, following the feature-focused 0.7.0-beta milestone.

The `imported-records-template` branch is an experimental development branch rather than a stable release. Features on this branch should be considered provisional until they have been built and tested on real Wii hardware.

MarcViiew contains functional MARC 21 and ISO 2709 support through ViiewLib, including `.mrc` encoding and Wii hardware testing. However, the MARC 21 and ISO 2709 implementation is still under active development and should not be considered a complete implementation of either standard or production-ready LMS software.

## License

This project is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

The MIT License applies to MarcViiew's original source code; third-party data and resources remain subject to their respective terms.

## Credits

GameTDB — for providing Wii game information and metadata.

Library of Congress — for the MARC 21 bibliographic standards.

ISO/TC 46 — for ISO 2709, Information and documentation — Format for information exchange.

dborth/libgui - for providing the necessary UI framework, later being used as a compatibility layer.

Lilyflower — for extensively testing MarcViiew on vWii and providing external `.mrc` files.
