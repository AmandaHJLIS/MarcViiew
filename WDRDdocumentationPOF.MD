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

Validation:

$a should contain the identifier represented by the
corresponding Wii title directory.

Example:

`001  14 $a NALE`
