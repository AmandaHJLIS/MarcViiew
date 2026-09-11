# MarcViiew
<img width="370" height="67" alt="21d000b48fc0ec2adb4efcf95f6ebed8" src="https://github.com/user-attachments/assets/a0dd8b27-50b7-4e6c-b303-47b6bf3110f0" />

MarcViiew is a Wii-based library management system for cataloguing and viewing video game records, including MARC21 metadata.

MarcViiew is a free and open-source LMS (Library management system) for the Wii. Using either a USB or SD card, it can display game information for cataloguing purposes and MARC21 metadata information if you are adding it into your own LMS. This Github page includes a database txt file for all 10,170 games designed for MarcViiew and MARC21.

This homebrew application was designed in mind for libraries, librarians, library technicians and library & information science students. 

Please reference 'MARC 21 Format for Bibliographic Data' by the Library of Congress when accessing MARC21 metadata records.
https://www.loc.gov/marc/bibliographic/

The current WIP version uses a locally stored database generated from GameTDB data. The database contains information for over 10,000 Wii game records and is designed to provide the metadata used by MarcViiew.

<img width="371.5" height="229" alt="image" src="https://github.com/user-attachments/assets/d2f20238-7758-4c1a-8615-52425e828136" />

## Current Features

- Detects Wii games stored on a USB device
- Identifies games using their Wii Game ID
- Matches games against the MarcViiew database
- Displays game metadata
- Displays game synopses
- Scrollable game information
- Nintendo Wii Homebrew Channel support

## Planned Features
- Game searching
- MARC 21 record viewing
- Additional cataloguing metadata
- Improved catalogue navigation
-Settings
- Additional library management features

## Setup

1. Copy the apps folder contents to the apps folder on the SD card used by your Wii.
The application should be located at:
SD:/apps/marcviiew/

2. Copy marcviiew_games.txt to the root of the SD card:
SD:/marcviiew_games.txt

3. Connect the USB storage device containing your Wii games.
4. Open the Homebrew Channel and launch MarcViiew.

Note: MarcViiew is currently a work in progress. Some menu options and planned features are not yet implemented.

## License

This project is licensed under the MIT License. See the (LICENSE) file for details.

The MIT License applies to MarcViiew's original source code; third-party data and resources remain subject to their respective terms.

## Credits

GameTDB — for providing Wii game information and metadata.

Library of Congress — for the MARC 21 bibliographic standards.

ISO/TC 46 — for ISO 2709, Information and documentation — Format for information exchange.
