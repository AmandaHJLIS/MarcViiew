#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "marc_text_parser.h"


/*
 * Read one complete line from a FILE.
 *
 * Unlike fgets(), this does not impose a fixed maximum line
 * length. This matters for long MARC 500 notes.
 *
 * Returns:
 *     1   line read successfully
 *     0   EOF before any characters were read
 *    -1   allocation or I/O failure
 *
 * The returned string is heap allocated and must be freed.
 */
static int read_line(
    FILE *file,
    char **line_out
)
{
    size_t capacity;
    size_t length;
    char *line;

    int character;

    if (file == NULL || line_out == NULL)
        return -1;

    *line_out = NULL;

    capacity = 256;
    length = 0;

    line = malloc(capacity);

    if (line == NULL)
        return -1;

    while ((character = fgetc(file)) != EOF)
    {
        /*
         * Stop at either newline style.
         */
        if (character == '\n')
            break;

        if (character == '\r')
        {
            /*
             * Handle CRLF without leaving the LF behind.
             */
            int next = fgetc(file);

            if (next != '\n' && next != EOF)
                ungetc(next, file);

            break;
        }

        /*
         * Leave space for the terminating NUL.
         */
        if (length + 1 >= capacity)
        {
            char *new_line;

            capacity *= 2;

            new_line = realloc(
                line,
                capacity
            );

            if (new_line == NULL)
            {
                free(line);
                return -1;
            }

            line = new_line;
        }

        line[length++] =
            (char)character;
    }

    if (character == EOF &&
        length == 0)
    {
        free(line);

        if (ferror(file))
            return -1;

        return 0;
    }

    line[length] = '\0';

    *line_out = line;

    return 1;
}


/*
 * Test whether a line is the beginning of a record.
 */
static int is_record_marker(
    const char *line
)
{
    if (line == NULL)
        return 0;

    return strcmp(
        line,
        "[RECORD]"
    ) == 0;
}


/*
 * Parse:
 *
 *     [245]
 *
 * into:
 *
 *     "245"
 */
static int parse_field_heading(
    const char *line,
    char tag[4]
)
{
    if (line == NULL ||
        tag == NULL)
        return -1;

    if (strlen(line) != 5)
        return -1;

    if (line[0] != '[' ||
        line[4] != ']')
        return -1;

    if (!isdigit(
            (unsigned char)line[1]) ||
        !isdigit(
            (unsigned char)line[2]) ||
        !isdigit(
            (unsigned char)line[3]))
    {
        return -1;
    }

    tag[0] = line[1];
    tag[1] = line[2];
    tag[2] = line[3];
    tag[3] = '\0';

    return 0;
}


/*
 * MARC 21 control fields are 001-009.
 */
static int is_control_tag(
    const char *tag
)
{
    if (tag == NULL)
        return 0;

    return (
        tag[0] == '0' &&
        tag[1] == '0' &&
        tag[2] >= '1' &&
        tag[2] <= '9'
    );
}


/*
 * Return the portion after the first '='.
 */
static const char *value_after_equals(
    const char *line
)
{
    const char *equals;

    if (line == NULL)
        return NULL;

    equals = strchr(
        line,
        '='
    );

    if (equals == NULL)
        return NULL;

    return equals + 1;
}


/*
 * Parse:
 *
 *     IND1=#
 *
 * or:
 *
 *     IND1=0
 *
 * '#' is MarcViiew's representation of a blank MARC indicator.
 */
static int parse_indicator(
    const char *line,
    char *indicator
)
{
    const char *value;

    if (line == NULL ||
        indicator == NULL)
        return -1;

    value =
        value_after_equals(line);

    if (value == NULL)
        return -1;

    if (value[0] == '\0' ||
        value[1] != '\0')
        return -1;

    if (value[0] == '#')
        *indicator = ' ';
    else
        *indicator = value[0];

    return 0;
}


/*
 * Parse:
 *
 *     $a=Auto Erase Disc
 *
 * into:
 *
 *     code  = 'a'
 *     value = "Auto Erase Disc"
 */
static int parse_subfield_line(
    const char *line,
    char *code,
    const char **value
)
{
    if (line == NULL ||
        code == NULL ||
        value == NULL)
        return -1;

    if (line[0] != '$' ||
        line[1] == '\0' ||
        line[2] != '=')
        return -1;

    *code = line[1];
    *value = line + 3;

    return 0;
}


/*
 * Parser state for the field currently being read.
 */
typedef struct
{
    int active;

    char tag[4];

    int control;

    int control_value_seen;

    char indicator1;
    char indicator2;

    int indicator1_seen;
    int indicator2_seen;

    MARC_Field *field;

} CurrentField;


/*
 * Reset the current-field state.
 */
static void reset_current_field(
    CurrentField *current
)
{
    if (current == NULL)
        return;

    current->active = 0;

    current->tag[0] = '\0';

    current->control = 0;

    current->control_value_seen = 0;

    current->indicator1 = ' ';
    current->indicator2 = ' ';

    current->indicator1_seen = 0;
    current->indicator2_seen = 0;

    current->field = NULL;
}


/*
 * Finish the current field.
 *
 * Control fields have already been inserted into the record
 * when their VALUE= line was read.
 *
 * Data fields are created when the first subfield appears.
 * If a data field has zero subfields, it is still created here.
 */
static int finish_current_field(
    MARC_Record *record,
    CurrentField *current
)
{
    MARC_Field *field;

    if (record == NULL ||
        current == NULL)
        return -1;

    if (!current->active)
        return 0;

    /*
     * Control field.
     */
    if (current->control)
    {
        if (!current->control_value_seen)
            return -1;

        reset_current_field(current);

        return 0;
    }

    /*
     * Data field with no subfields yet.
     *
     * This still represents a legitimate empty variable field.
     */
    if (current->field == NULL)
    {
        field =
            marc_field_create(
                current->tag,
                current->indicator1,
                current->indicator2
            );

        if (field == NULL)
            return -1;

        current->field = field;
    }

    /*
     * Transfer ownership to the MARC record.
     */
    if (marc_record_add_field(
            record,
            current->field
        ) != 0)
    {
        marc_field_free(
            current->field
        );

        current->field = NULL;

        return -1;
    }

    current->field = NULL;

    reset_current_field(current);

    return 0;
}


/*
 * Parse the contents of one record.
 *
 * The [RECORD] marker has already been consumed.
 */
static int parse_record_body(
    MarcViiewParser *parser,
    MARC_Record **record_out
)
{
    MARC_Record *record;

    CurrentField current;

    char *line = NULL;

    int result;

    if (parser == NULL ||
        record_out == NULL)
        return -1;

    record = marc_record_create();

    if (record == NULL)
        return -1;

    reset_current_field(
        &current
    );

    while (1)
    {
        char tag[4];

        result =
            read_line(
                parser->file,
                &line
            );

        if (result < 0)
        {
            marc_record_free(record);
            return -1;
        }

        /*
         * End of file.
         */
        if (result == 0)
        {
            if (finish_current_field(
                    record,
                    &current
                ) != 0)
            {
                if (current.field != NULL)
                    marc_field_free(
                        current.field
                    );

                marc_record_free(record);
                return -1;
            }

            if (marc_record_get_field_count(
                    record
                ) == 0)
            {
                marc_record_free(record);
                return -1;
            }

            parser->eof = 1;

            *record_out = record;

            return 1;
        }

        /*
         * A new [RECORD] begins.
         */
        if (is_record_marker(line))
        {
            free(line);
            line = NULL;

            if (finish_current_field(
                    record,
                    &current
                ) != 0)
            {
                if (current.field != NULL)
                    marc_field_free(
                        current.field
                    );

                marc_record_free(record);
                return -1;
            }

            if (marc_record_get_field_count(
                    record
                ) == 0)
            {
                marc_record_free(record);
                return -1;
            }

            /*
             * The marker belongs to the next record.
             * We have consumed it, so tell the next call
             * to begin directly with that record's body.
             */
            parser->pending_record = 1;

            *record_out = record;

            return 1;
        }

        /*
         * Empty lines are allowed between sections.
         */
        if (line[0] == '\0')
        {
            free(line);
            line = NULL;
            continue;
        }

        /*
         * MarcViiew-specific metadata.
         *
         * GAME_ID duplicates 001 in the current database.
         * It is used by MarcViiew's text database but is not
         * itself a MARC field, so it is intentionally not
         * inserted into the MARC_Record.
         */
        if (strncmp(
                line,
                "GAME_ID=",
                8
            ) == 0)
        {
            free(line);
            line = NULL;
            continue;
        }

        /*
         * Field heading.
         */
        if (parse_field_heading(
                line,
                tag
            ) == 0)
        {
            /*
             * Finish the preceding field first.
             */
            if (finish_current_field(
                    record,
                    &current
                ) != 0)
            {
                free(line);

                if (current.field != NULL)
                    marc_field_free(
                        current.field
                    );

                marc_record_free(record);
                return -1;
            }

            current.active = 1;

            strcpy(
                current.tag,
                tag
            );

            current.control =
                is_control_tag(tag);

            current.control_value_seen = 0;

            current.indicator1 = ' ';
            current.indicator2 = ' ';

            current.indicator1_seen = 0;
            current.indicator2_seen = 0;

            current.field = NULL;

            free(line);
            line = NULL;

            continue;
        }

        /*
         * Control field value.
         */
        if (strncmp(
                line,
                "VALUE=",
                6
            ) == 0)
        {
            const char *value;

            if (!current.active ||
                !current.control ||
                current.control_value_seen)
            {
                free(line);

                if (current.field != NULL)
                    marc_field_free(
                        current.field
                    );

                marc_record_free(record);
                return -1;
            }

            value = line + 6;

            if (marc_record_set_control_field(
                    record,
                    current.tag,
                    value
                ) != 0)
            {
                free(line);

                marc_record_free(record);
                return -1;
            }

            current.control_value_seen = 1;

            free(line);
            line = NULL;

            continue;
        }

        /*
         * First indicator.
         */
        if (strncmp(
                line,
                "IND1=",
                5
            ) == 0)
        {
            if (!current.active ||
                current.control ||
                current.field != NULL)
            {
                free(line);

                if (current.field != NULL)
                    marc_field_free(
                        current.field
                    );

                marc_record_free(record);
                return -1;
            }

            if (parse_indicator(
                    line,
                    &current.indicator1
                ) != 0)
            {
                free(line);

                marc_record_free(record);
                return -1;
            }

            current.indicator1_seen = 1;

            free(line);
            line = NULL;

            continue;
        }

        /*
         * Second indicator.
         */
        if (strncmp(
                line,
                "IND2=",
                5
            ) == 0)
        {
            if (!current.active ||
                current.control ||
                current.field != NULL)
            {
                free(line);

                if (current.field != NULL)
                    marc_field_free(
                        current.field
                    );

                marc_record_free(record);
                return -1;
            }

            if (parse_indicator(
                    line,
                    &current.indicator2
                ) != 0)
            {
                free(line);

                marc_record_free(record);
                return -1;
            }

            current.indicator2_seen = 1;

            free(line);
            line = NULL;

            continue;
        }

        /*
         * Subfield.
         */
        if (line[0] == '$')
        {
            char code;
            const char *value;

            if (!current.active ||
                current.control)
            {
                free(line);

                if (current.field != NULL)
                    marc_field_free(
                        current.field
                    );

                marc_record_free(record);
                return -1;
            }

            if (parse_subfield_line(
                    line,
                    &code,
                    &value
                ) != 0)
            {
                free(line);

                if (current.field != NULL)
                    marc_field_free(
                        current.field
                    );

                marc_record_free(record);
                return -1;
            }

            /*
             * Create the field only when the first subfield
             * arrives, after both indicators have been read.
             */
            if (current.field == NULL)
            {
                current.field =
                    marc_field_create(
                        current.tag,
                        current.indicator1,
                        current.indicator2
                    );

                if (current.field == NULL)
                {
                    free(line);

                    marc_record_free(record);
                    return -1;
                }
            }

            if (marc_field_add_subfield(
                    current.field,
                    code,
                    value
                ) != 0)
            {
                free(line);

                marc_field_free(
                    current.field
                );

                current.field = NULL;

                marc_record_free(record);
                return -1;
            }

            free(line);
            line = NULL;

            continue;
        }

        /*
         * Anything else is malformed MarcViiew input.
         */
        free(line);
        line = NULL;

        if (current.field != NULL)
            marc_field_free(
                current.field
            );

        marc_record_free(record);

        return -1;
    }
}


int marcviiew_parser_init(
    MarcViiewParser *parser,
    FILE *file
)
{
    if (parser == NULL ||
        file == NULL)
        return -1;

    parser->file = file;

    parser->initialized = 1;
    parser->eof = 0;
    parser->error = 0;
    parser->pending_record = 0;

    return 0;
}


int marcviiew_parser_next(
    MarcViiewParser *parser,
    MARC_Record **record
)
{
    char *line = NULL;
    int result;

    if (parser == NULL ||
        record == NULL ||
        !parser->initialized ||
        parser->file == NULL)
    {
        return -1;
    }

    *record = NULL;

    if (parser->error)
        return -1;

    if (parser->eof)
        return 0;

    /*
     * If the previous record consumed the next [RECORD]
     * marker, begin immediately with that record's body.
     */
    if (parser->pending_record)
    {
        parser->pending_record = 0;

        return parse_record_body(
            parser,
            record
        );
    }

    /*
     * Otherwise find the first [RECORD] marker.
     */
    while (1)
    {
        result =
            read_line(
                parser->file,
                &line
            );

        if (result < 0)
        {
            parser->error = 1;
            return -1;
        }

        if (result == 0)
        {
            parser->eof = 1;
            return 0;
        }

        if (is_record_marker(line))
        {
            free(line);
            line = NULL;

            return parse_record_body(
                parser,
                record
            );
        }

        /*
         * Ignore anything before the first [RECORD].
         */
        free(line);
        line = NULL;
    }
}


MARC_Record *marcviiew_parse_next_record(
    MarcViiewParser *parser
)
{
    MARC_Record *record = NULL;

    if (marcviiew_parser_next(
            parser,
            &record
        ) != 1)
    {
        return NULL;
    }

    return record;
}