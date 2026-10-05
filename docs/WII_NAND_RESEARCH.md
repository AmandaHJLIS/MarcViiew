# Wii NAND, WiiWare and Virtual Console Research

**Last Updated:** 5th of October 2026

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


## Research update: TMD to physical content mapping

Recent hardware tests have confirmed an important part of the downloadable-title content model.

A TMD record is a logical content description. The physical location depends on its content type:

    0x0001 NORMAL
        -> investigate the title's content/<content ID>.app

    0x8001 SHARED
        -> SHA-1 lookup in /shared1/content.map
        -> /shared1/<shared identifier>.app

Observed hardware results:

- Cave Story: 7 records, 0 shared records.
- The Legend of Zeldia Ocarina of Time VC: 7 records, 3 shared records; 3/3 shared SHA-1 matches.
- Super Mario 64 VC: 7 records, 3 shared records; 3/3 shared SHA-1 matches.
- Super Smash Bros. 64 VC: current TMD inspection causes a DSI exception; cause unresolved.

These observations support the current implementation model but do not yet establish a universal physical layout for every WiiWare/VC title.

The next research goal is to make MarcViiew resolve and display the physical .app location for each TMD record where that location can be confirmed.

## Additional hardware observations

The downloadable-title tests have since produced two further observations:

- **Internet Channel:** 4 TMD SHA-1 values matched entries in `/shared1/content.map`. This is another positive hardware result supporting SHA-1-based shared-content resolution.
- **Wii no Ma:** current TMD inspection produces a DSI exception. The cause is unresolved, so the exception is recorded as an implementation/test failure rather than evidence of a particular physical content layout.

These results reinforce the need to distinguish confirmed hardware observations from explanations that have not yet been established.
