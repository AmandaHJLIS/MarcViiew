MarcViiew MARC21 Profile

**Status:** Current  
**Version:** 1.0  
**Last Updated:** 13 September 2026

## 1. Overview

This document defines the MARC21 field usage and conventions
currently used by MarcViiew.

These definitions describe how MarcViiew interprets and generates
MARC21 records. They do not replace or redefine the MARC21 standard.

MarcViiew uses a defined subset of MARC21 fields together with the
WiiExt vocabulary for game genre/form terms.

---

## 2. Field Definitions

### 001 — Control Number

**MarcViiew use:** Wii Game ID

The `001` field contains the unique Wii Game ID associated with
the game.

## 245 — Title Statement

MarcViiew use: Game title

The 245 field contains the title of the Wii game.

The second indicator specifies the number of initial characters
that should be ignored for filing purposes.

Example:

245 04 $a The Legend of Zelda: Twilight Princess

In this example, 4 indicates that "The " is treated as
non-filing text.

## 264 — Production, Publication, Distribution, Manufacture,

and Copyright Notice

MarcViiew use: Publication/distribution information and
release date

The 264 field contains publication/distribution information
and the release date used by MarcViiew.

## 300 — Physical Description

MarcViiew use: Wii disc description

The 300 field describes the physical Wii disc associated with
the record.

## 500 — General Note

MarcViiew use: Game synopsis

The 500 field contains the synopsis/description of the game.

## 542 — Information Relating to Copyright Status

MarcViiew use: Selected metadata provenance information

The 542 field contains selected provenance information used
by MarcViiew.

Only the portions defined by the MarcViiew profile are used.

## 655 — Genre/Form Terms

MarcViiew use: WiiExt genre/form terms

The 655 field contains genre/form terms from the WiiExt
vocabulary.

The $2 subfield identifies the source vocabulary.

Example:

655  7 $a Simulation games $2 Wiiext
655 $2 — Source of Term

MarcViiew use: WiiExt vocabulary identifier

The 655 $2 subfield identifies WiiExt as the source vocabulary
for the associated genre/form term.

## MarcViiew uses:

$2 Wiiext
3. WiiExt

WiiExt is an unofficial vocabulary/profile developed for use with
MarcViiew.

WiiExt is not an official Library of Congress extension,
vocabulary, or standard, and is not endorsed or maintained by the
Library of Congress.

## 4. LMS Compatibility

MarcViiew field definitions and WiiExt may not be compatible with
your LMS. Please refer to your LMS procedures for configuration.

MarcViiew does not provide LMS-specific configuration or
implementation instructions.

## 5. Scope

This document defines the current MarcViiew MARC21 profile.

Fields and conventions may be expanded or revised in future
versions. Changes to the profile should be documented through
version history.

| 1.0 | 13/09/2026 | Creation of documentation |
|---|---|---|
| 1.0 | 2026-09-13 | Initial MarcViiew MARC21 profile |
