# WiiExt Vocabulary Profile

**Status:** Current / Development
**Version:** 0.1
**Vocabulary identifier:** `Wiiext`
**Application:** MarcViiew

---

## 1. Overview

WiiExt is an unofficial controlled vocabulary developed for use with MarcViiew and related Wii game cataloguing projects.

The vocabulary provides terminology for describing video game genres, forms, themes, settings, and other related characteristics.

WiiExt is not an official Library of Congress vocabulary, extension, or standard.

WiiExt is not endorsed, maintained, or administered by the Library of Congress.

---

## 2. Purpose

WiiExt provides controlled terminology for describing characteristics of Wii games in MarcViiew.

The vocabulary is intended to support:

* consistent cataloguing
* genre and form classification
* thematic classification
* game discovery
* searching and filtering
* MARC 21 representation

---

## 3. MARC 21 Usage

WiiExt terms are recorded using MARC 21 field `655`.

The `$2` subfield identifies WiiExt as the source vocabulary.

Example:

```text
655  _7 $a Simulation games $2 Wiiext
```

The value `Wiiext` is the vocabulary identifier used by MarcViiew.

---

## 4. Term Types

WiiExt currently contains several types of terminology.

### Genre

Genre terms describe the type or style of gameplay.

Examples include:

* Action games
* Adventure games
* Fighting games
* Platform games
* Puzzle games
* Racing games
* Role-playing games
* Simulation games
* Sports games
* Strategy games
* Horror games

### Form

Form terms describe the type or nature of the software.

Examples include:

* Compilations
* Demos
* Samplers
* Educational software
* Multimedia software
* Emulators
* Software

### Theme

Theme terms describe thematic characteristics of a game.

Examples include:

* Fantasy
* Science fiction

### Setting

Setting terms describe the temporal or environmental setting of a game.

Examples include:

* Historic settings
* Modern-day settings

---

## 5. Vocabulary Structure

Each WiiExt term may contain the following properties:

| Property       | Description                                       |
| -------------- | ------------------------------------------------- |
| Preferred term | The primary WiiExt term                           |
| Type           | Genre, form, theme, or other term type            |
| Source terms   | Terms used to associate or map source terminology |
| BT             | Broader term                                      |
| NT             | Narrower term                                     |

Example:

```text
Term: Platform games
Type: genre
BT: Action games
NT:
- 2D platform games
- 3D platform games
```

---

## 6. Broader and Narrower Relationships

WiiExt may define hierarchical relationships between terms.

**BT (Broader Term)** identifies a broader concept.

**NT (Narrower Term)** identifies a more specific concept.

For example:

```text
Action games
└── Platform games
    ├── 2D platform games
    └── 3D platform games
```

These relationships are intended to support vocabulary navigation, organization, and discovery.

### Hierarchy Status

The WiiExt hierarchy is currently under development.

Some BT and NT relationships may be provisional, incomplete, or subject to revision. Relationships represented in the current vocabulary should therefore not necessarily be considered a final semantic hierarchy.

Future versions of WiiExt may add missing concepts, revise broader/narrower relationships, or otherwise restructure the vocabulary.

---

## 7. Source Terms

WiiExt terms may contain associated source terms.

Source terms represent terminology used to identify or map concepts during catalogue creation.

For example:

```text
Preferred term:
Action games

Source term:
action
```

Source terms are not necessarily alternative preferred terms for display in a catalogue.

---

## 8. Vocabulary Terms

The authoritative machine-readable vocabulary is maintained in the MarcViiew source code.

The current implementation defines WiiExt terms and their associated properties, including:

* term type
* source terms
* broader relationships
* narrower relationships

The documentation describes the structure and intended use of the vocabulary; the machine-readable source remains the authoritative representation of the current term set.

---

## 9. Non-Official Status

WiiExt is an unofficial vocabulary developed for use with MarcViiew.

It is not an official Library of Congress vocabulary, extension, or standard.

It is not endorsed, maintained, or administered by the Library of Congress.

References to MARC 21 or Library of Congress terminology within MarcViiew documentation do not imply endorsement of WiiExt by the Library of Congress.

---

## 10. LMS Compatibility

WiiExt may not be supported by all library management systems or catalogue platforms.

Systems using MarcViiew records may require local configuration to support WiiExt terminology and the associated MARC 21 fields.

Please refer to the procedures and documentation provided by the relevant LMS when configuring controlled vocabulary fields.

---

## 11. Version History

| Version | Date       | Changes                              |
| ------- | ---------- | ------------------------------------ |
| 0.1     | 2026-09-13 | Initial documented WiiExt vocabulary |
