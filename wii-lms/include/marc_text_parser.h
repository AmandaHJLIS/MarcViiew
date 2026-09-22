#ifndef MARC_TEXT_PARSER_H
#define MARC_TEXT_PARSER_H

#include <stdio.h>

#include <viiewlib/marc.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct
{
    FILE *file;

    int initialized;
    int eof;
    int error;

    /*
     * Set when the parser encounters the [RECORD] marker
     * belonging to the next record.
     *
     * The marker itself has already been consumed, so the
     * next call starts directly with that record's contents.
     */
    int pending_record;

} MarcViiewParser;


/*
 * Initialise a parser for an already-open MarcViiew MARC file.
 *
 * Returns:
 *     0   success
 *    -1   invalid arguments
 */
int marcviiew_parser_init(
    MarcViiewParser *parser,
    FILE *file
);


/*
 * Parse the next MarcViiew record.
 *
 * Returns:
 *      1   record successfully parsed
 *      0   end of file
 *     -1   parse error
 *
 * On success, *record receives a newly allocated MARC_Record.
 * The caller owns the record and must call marc_record_free().
 */
int marcviiew_parser_next(
    MarcViiewParser *parser,
    MARC_Record **record
);


/*
 * Convenience wrapper around marcviiew_parser_next().
 *
 * Returns:
 *     MARC_Record * on success
 *     NULL          on EOF or parse error
 *
 * Use marcviiew_parser_next() when EOF and parse errors
 * need to be distinguished.
 */
MARC_Record *marcviiew_parse_next_record(
    MarcViiewParser *parser
);


#ifdef __cplusplus
}
#endif

#endif