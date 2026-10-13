# WDRD Metadata framework Profile

**Status:** Planned  

**Version:** Draft 0.1 

**Last Updated:** 4th of October 2026

## Scope and status

WDRD is a proposed, Wii-specific technical metadata framework. Its primary purpose is to describe Wii title identifiers and platform metadata, TMD records, content inventories, and observed relationships between a title and its installed or distributed content. The framework is being developed to support systematic documentation and technical research into Wii software, including WiiWare and Virtual Console reverse-engineering and modding.

WDRD is not a PREMIS implementation or a digital-preservation profile. The planned section ranges below are a working organisational outline, not a claim that every category is required or will be implemented. Evidence and provenance may be recorded where they help distinguish documented platform behaviour from direct observation, inference, or an unresolved hypothesis.

## Relationship to existing standards and references

WDRD is an independent framework and does not replace MARC 21, RDA, AACR2, or other bibliographic cataloguing standards. Those standards address bibliographic description and related resource relationships; WDRD is intended to describe Wii-specific technical structures that are not adequately represented by those general-purpose models.

**PREMIS (Preservation Metadata: Implementation Strategies) is consulted as a comparative reference only.** PREMIS is designed for digital-preservation metadata. Its documentation can provide examples of formal metadata modelling, including the definition of entities, properties, relationships, and element-level rules. Those examples may inform how WDRD documents its own fields, but they do not establish WDRD's data model or field definitions. WDRD does not claim PREMIS conformance, does not adopt the PREMIS Data Dictionary wholesale, and does not require PREMIS preservation-event or preservation-rights records as part of its core scope.

Any WDRD field should be justified by Wii platform evidence, a clear technical-description requirement, or an explicitly documented research use case. Similarity to a PREMIS concept is not, by itself, sufficient reason to include a field.

## 000-099 Control / Identification 

### 001 Title ID

Definition:

     The identifier assigned to an installed Wii title.

Data type:

     Alphanumeric identifier

Repeatability:

     Not repeatable

$a  Title identifier:

     The complete title identifier.

Indicators:

1st — Identifier recognition:

     0  Unrecognised / unresolved
     1  Recognised

2nd — Identifier length:

     4  Four-character identifier
     5  Five-character identifier
     6  Six-character identifier

3rd — Title type:

     1  Wiiware identifier
     2  VC identifier
     3  Wii Channel identifier


Validation:

     $a should contain the identifier represented by the
     corresponding Wii title directory.

Example:

     001  142 
          $a NALE

## 000–099  Control / identification

## 100–199  Descriptive metadata

## 200–299  Wii title metadata

## 300–399  Physical / carrier representation

## 400–499  Digital representation

## 500–599  Content structure

## 600–699  Relationships

## 700–799  Technical metadata

## 800–899  Preservation

## 900–949  Evidence / provenance

## 950–999  Local / research extensions
