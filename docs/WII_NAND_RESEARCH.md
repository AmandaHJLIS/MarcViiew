# Wii NAND, WiiWare and Virtual Console Research

**Last Updated:** 3rd of October 2026

This directory contains the ongoing research behind MarcViiew's Wii NAND and downloadable-title support.

The documentation separates format knowledge, MarcViiew implementation behaviour, hardware observations, and unresolved hypotheses.

## Documentation

- [NAND](NAND.md) — NAND filesystem, IOS/ISFS access, title directories, permissions and read-only constraints
- [Title System](TITLE_SYSTEM.md) — title IDs, ES, installed-title architecture and ticket/TMD relationships
- [TMD](TMD.md) — TMD structure, offsets, content records, boot index, types and hashes
- [Ticket](TICKET.md) — ticket role, title association and security-sensitive handling
- [Content](CONTENT.md) — normal, DLC and shared content, physical storage and `.app` research
- [WAD](WAD.md) — WAD structure and offline title/content investigation
- [WiiWare & Virtual Console](WIIWARE_VC.md) — WiiWare, VC, downloadable channels and title-specific observations
- [Research Log](RESEARCH_LOG.md) — MarcViiew implementation experiments, hardware results, regressions and research status

## Research status

This is an ongoing research effort. Confirmed format facts, hardware observations, strong inferences and hypotheses are deliberately distinguished.

The major unresolved problem is mapping every TMD content record to its actual installed physical storage location and internal content format.

## Safe research principle

Prefer offline WAD/content analysis before introducing additional live-NAND probing. MarcViiew's NAND browser is intended to remain read-only.
