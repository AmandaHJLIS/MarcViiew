# Wii Title System

## 5. Title identity and ES

A Wii title is identified by a 64-bit title ID.

MarcViiew commonly represents it as:

    high 32 bits / low 32 bits

Example:

    00010001 / 4E414C45

The ES title-management system is involved in identifying titles and their metadata.

The experimental browser uses ES title/TMD information when entering title-owned areas. ES_Identify was part of the access strategy, but its failure is not itself treated as proof that ISFS cannot read a directory.

Filesystem access and ES title identification should remain conceptually separate.

## 16. WiiWare and Virtual Console

WiiWare and Virtual Console titles are installed Wii titles rather than ordinary disc-image files.

Their title IDs identify them within the Wii title system.

A useful broad model is:

    Title ID
       |
       +-- Ticket
       |
       +-- TMD
       |     |
       |     +-- content records
       |           +-- content ID
       |           +-- index
       |           +-- type
       |           +-- size
       |           +-- SHA-1
       |
       +-- title-owned content
       |
       +-- shared content, when applicable
       |
       +-- title-specific data

Two titles both described as WiiWare or VC do not necessarily have identical physical layouts.

The TMD is therefore the safest starting point for investigating an individual title.

