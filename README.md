<img width="370" height="67" alt="21d000b48fc0ec2adb4efcf95f6ebed8" src="https://github.com/user-attachments/assets/a0dd8b27-50b7-4e6c-b303-47b6bf3110f0" />

MarcViiew is a free and open-source library management system (LMS) for the Nintendo Wii, designed for cataloguing, identifying, and viewing video game records.

MarcViiew combines Wii game detection with library and information science principles, allowing games stored on an SD card or USB device to be identified by their Wii Game ID and matched against a local metadata database. Game information can then be viewed for cataloguing purposes alongside MARC 21 bibliographic records.

The project is designed with libraries, librarians, library technicians, and library and information science students in mind. It explores how established library cataloguing standards and controlled vocabularies can be applied to video game collections within a Wii homebrew environment.

<img width="526" height="218.5" alt="image" src="https://github.com/user-attachments/assets/3495ca94-d0d8-4442-bed4-fc87754e3072" />

## MARC 21

<img width="500" height="108" alt="image" src="https://github.com/user-attachments/assets/11d4aa98-af50-4a5e-9f29-7e3c70fe9637" />

MarcViiew uses the MARC 21 bibliographic format as the basis for its cataloguing and MARC record functionality.

MARC 21 is maintained by the Library of Congress Network Development and MARC Standards Office. MarcViiew is intended to provide a practical way to view and experiment with MARC 21 records for video game collections.

For authoritative documentation and current field definitions, please refer to the MARC 21 Format for Bibliographic Data provided by the Library of Congress.

For Marcviiew authoritative documentation and current field definitions, please refer to below.

## Wiiext

<img width="289" height="`102" alt="Wii-Logo" src="https://github.com/user-attachments/assets/1077167d-ca67-45e8-bb7b-81f281b966ef" />

Wiiext is a controlled vocabulary and thesaurus developed specifically for describing Wii video games and related software.

MarcViiew uses Wiiext terms to provide consistent genre and form classification within its MARC 21 records. Wiiext is intended to provide a structured vocabulary for video game cataloguing, with preferred terms and thesaurus relationships designed for use within MarcViiew.

Wiiext is an unofficial extension/profile developed for use with MarcViiew. It is not an official Library of Congress extension, vocabulary, or standard, and is not endorsed or maintained by the Library of Congress.

## Game Metadata

MarcViiew currently uses a locally stored database generated from GameTDB data. The database contains information for over 10,000 Wii game records and provides the metadata used by MarcViiew to identify and describe games.

GameTDB provides information including game IDs, titles, publishers, developers, release dates, regions, genres, and synopses. The GameTDB data is processed into a format that can be used by MarcViiew on the Wii.

MarcViiew's GameTDB-derived database is currently a work in progress and may be expanded or refined as development continues.

## MarcViiew Cataloguing Rules

MarcViiew uses the MARC 21 bibliographic format as the foundation for its cataloguing system. Where source metadata requires normalisation or adaptation for use within MarcViiew, project-specific cataloguing conventions may be applied.

These conventions are intended to provide consistent representation of video game metadata within MarcViiew and do not replace the official MARC 21 standards.

### Metadata
- GameTDB is currently used as the primary source for game metadata.
- Wii Game IDs are used as the primary identifier for games.
- Dates supplied by GameTDB are normalised to YYYY-MM-DD.
- Publisher and developer information is retained from the source metadata.
- GameTDB genre terms are mapped to preferred Wiiext terms where an appropriate term exists.

### MARC 21
- 001 contains the Wii Game ID.
- 245 contains the game title.
- Non-filing characters are accounted for in the 245 second indicator.
- 264 contains publication/distribution information and the release date used by MarcViiew.
- 300 describes the physical Wii disc.
- 500 contains the game synopsis.
- 542 contains selected metadata provenance information.
- 655 contains Wiiext genre/form terms.
- 655 $2 identifies Wiiext as the source vocabulary.

For the complete field definitions and usage rules, see the
[MarcViiew MARC21 Profile](marcviiew-marc-profile.md).

These conventions may change as MarcViiew's cataloguing model develops.

## Current Features

- Detects Wii games stored on a USB device and SD card
- Identifies games using their Wii Game ID
- Matches games against the MarcViiew database
- Displays game metadata
- Displays game synopses
- Scrollable game information
- Nintendo Wii Homebrew Channel support
- Classic controller support
- Search bar for locating games
- MARC 21 record viewing
- Additional cataloguing metadata
- Controlled vocabulary/thesaurus 'Wiiext'
- Marcviiew game database generated from GameTDB data.
- Search for either MARC21 or normal game records with the search option.
- The option to reload the USB ports/SD card and databases

## Planned Features

- Series fields to be filled
- .mrc enconder
- .mrc importer

## Setup

1. Copy the apps folder contents to the apps folder on the SD card used by your Wii.
The application should be located at:
SD:/apps/marcviiew/

2. Copy marcviiew_games.txt and marcviiew_marc.txt to the root of the SD card:
SD:/marcviiew_games.txt SD:/marcviiew_marc.txt

3. Connect the USB storage device containing your Wii games.
4. Open the Homebrew Channel and launch MarcViiew.

## Note

MarcViiew remains a work in progress. The current release focuses on establishing the core catalogue, metadata and MARC 21 functionality.

MarcViiew field definitions and Wiiext may not be compatible with your LMS. Please refer to your LMS procedures for configuration.

## License

This project is licensed under the MIT License. See the (LICENSE) file for details.

The MIT License applies to MarcViiew's original source code; third-party data and resources remain subject to their respective terms.

## Credits

GameTDB — for providing Wii game information and metadata.

Library of Congress — for the MARC 21 bibliographic standards.

ISO/TC 46 — for ISO 2709, Information and documentation — Format for information exchange.
