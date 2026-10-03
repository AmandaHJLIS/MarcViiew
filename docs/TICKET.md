# Ticket

A ticket is part of the Wii title-management/security metadata associated with an installed or packaged title.

## Role

At a high level, the ticket associates authorization and title-key information with a title. It is distinct from the TMD: the ticket provides authorization/key information, while the TMD provides title metadata and the content inventory.

## Relationship to the title ID

Tickets contain title-identifying information used by the Wii title system. When analysing a ticket, its title ID should be compared with the corresponding TMD title ID rather than assuming files belong together solely because they were found together.

## Relationship to content

The ticket does not replace the TMD's content records. MarcViiew obtains content IDs, indices, types, sizes and SHA-1 hashes from the TMD.

## Handling

MarcViiew's NAND browser is read-only. Ticket parsing should remain an inspection operation and must not modify, replace, install or otherwise alter tickets on NAND.

For offline research, tickets from WADs or extracted title data are preferable to introducing additional live-NAND access.

## Future research

Exact ticket offsets and additional fields should be documented once verified against reliable format documentation and multiple samples.
