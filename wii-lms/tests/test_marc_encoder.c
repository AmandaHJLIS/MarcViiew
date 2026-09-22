#include <stdio.h>
#include <string.h>

#include <viiewlib/marc.h>

#include "marc_encoder.h"
#include "marc_text_parser.h"


static const char *DATABASE_PATH =
    "C:\\Users\\docte\\Desktop\\MarcViiew\\"
    "marcviiewmarc21\\marcviiew_marc.txt";

static const char *DATABASE_OUTPUT =
    "tests/encoder_test.mrc";

static const char *GAME_OUTPUT =
    "tests/encoder_game_test.mrc";


static int check_record(
    MARC_Record *record,
    const char *expected_id
)
{
    const char *id;

    if (record == NULL) {
        printf("FAIL: Record is NULL.\n");
        return -1;
    }

    id = marc_record_get_control_field(
        record,
        "001"
    );

    if (id == NULL) {
        printf("FAIL: Record has no 001 field.\n");
        return -1;
    }

    if (strcmp(id, expected_id) != 0) {
        printf(
            "FAIL: Expected 001=%s, got 001=%s\n",
            expected_id,
            id
        );
        return -1;
    }

    printf(
        "PASS: Found record %s\n",
        id
    );

    return 0;
}


/*
 * Test that the real MarcViiew database can be opened.
 */
static int test_database_access(void)
{
    FILE *file;

    printf("Checking database file access...\n");

    file = fopen(
        DATABASE_PATH,
        "rb"
    );

    if (file == NULL) {
        printf(
            "FAIL: Could not open database:\n"
            "%s\n",
            DATABASE_PATH
        );

        return -1;
    }

    printf(
        "PASS: Database opened successfully.\n"
    );

    fclose(file);

    return 0;
}


/*
 * Test the text parser against the first ten records.
 */
static int test_parser(void)
{
    FILE *input;
    MarcViiewParser parser;
    MARC_Record *record;
    int result;
    size_t count = 0;

    printf("\nTesting MarcViiew text parser...\n");

    input = fopen(
        DATABASE_PATH,
        "rb"
    );

    if (input == NULL) {
        printf(
            "FAIL: Parser could not open database.\n"
        );
        return -1;
    }

    if (marcviiew_parser_init(
            &parser,
            input
        ) != 0) {

        printf(
            "FAIL: Parser initialisation failed.\n"
        );

        fclose(input);
        return -1;
    }

    while (count < 10) {

        record = NULL;

        result = marcviiew_parser_next(
            &parser,
            &record
        );

        if (result == 0) {
            printf(
                "FAIL: Database ended after %zu records.\n",
                count
            );

            fclose(input);
            return -1;
        }

        if (result < 0) {
            printf(
                "FAIL: Parser error on record %zu.\n",
                count + 1
            );

            fclose(input);
            return -1;
        }

        if (record == NULL) {
            printf(
                "FAIL: Parser returned NULL record.\n"
            );

            fclose(input);
            return -1;
        }

        if (count == 0) {
            if (check_record(
                    record,
                    "007E01"
                ) != 0) {

                marc_record_free(record);
                fclose(input);
                return -1;
            }
        }

        if (count == 1) {
            if (check_record(
                    record,
                    "091E00"
                ) != 0) {

                marc_record_free(record);
                fclose(input);
                return -1;
            }
        }

        printf(
            "PASS: Record %zu parsed.\n",
            count + 1
        );

        marc_record_free(record);

        count++;
    }

    fclose(input);

    printf(
        "PASS: First %zu records parsed successfully.\n",
        count
    );

    return 0;
}


/*
 * Encode the complete MarcViiew database.
 */
static int test_database_encoding(void)
{
    FILE *output;
    long size;
    int result;

    printf(
        "\nTesting complete database encoding...\n"
    );

    result = marcviiew_encode_database(
        DATABASE_PATH,
        DATABASE_OUTPUT
    );

    if (result != 0) {
        printf(
            "FAIL: Complete database encoding failed.\n"
        );

        return -1;
    }

    output = fopen(
        DATABASE_OUTPUT,
        "rb"
    );

    if (output == NULL) {
        printf(
            "FAIL: Encoder reported success, "
            "but output could not be opened.\n"
        );

        return -1;
    }

    if (fseek(
            output,
            0,
            SEEK_END
        ) != 0) {

        printf(
            "FAIL: Could not seek to encoded output.\n"
        );

        fclose(output);
        return -1;
    }

    size = ftell(output);

    fclose(output);

    if (size <= 0) {
        printf(
            "FAIL: Encoded database is empty.\n"
        );

        return -1;
    }

    printf(
        "PASS: Complete database encoded.\n"
    );

    printf(
        "PASS: Database output size = %ld bytes.\n",
        size
    );

    return 0;
}


/*
 * Encode one known game.
 */
static int test_game_encoding(void)
{
    FILE *output;
    long size;
    int result;

    printf(
        "\nTesting selected game encoding...\n"
    );

    result = marcviiew_encode_game(
        DATABASE_PATH,
        GAME_OUTPUT,
        "007E01"
    );

    if (result != 0) {
        printf(
            "FAIL: Selected-game encoding failed.\n"
        );

        return -1;
    }

    output = fopen(
        GAME_OUTPUT,
        "rb"
    );

    if (output == NULL) {
        printf(
            "FAIL: Encoder reported success, "
            "but game output could not be opened.\n"
        );

        return -1;
    }

    if (fseek(
            output,
            0,
            SEEK_END
        ) != 0) {

        printf(
            "FAIL: Could not seek to game output.\n"
        );

        fclose(output);
        return -1;
    }

    size = ftell(output);

    fclose(output);

    if (size <= 0) {
        printf(
            "FAIL: Selected-game output is empty.\n"
        );

        return -1;
    }

    printf(
        "PASS: Selected game 007E01 encoded.\n"
    );

    printf(
        "PASS: Game output size = %ld bytes.\n",
        size
    );

    return 0;
}


/*
 * Read the complete ISO 2709 database back through
 * ViiewLib and count the records.
 *
 * marc_record_read() appends one ISO 2709 record
 * to the supplied MARC_Record.
 */
static int test_database_round_trip(void)
{
    FILE *input;
    MARC_Record *record;
    int result;
    size_t count = 0;
    int found_target = 0;

    printf(
        "\nTesting complete ISO 2709 round-trip...\n"
    );

    input = fopen(
        DATABASE_OUTPUT,
        "rb"
    );

    if (input == NULL) {
        printf(
            "FAIL: Could not open encoded database.\n"
        );

        return -1;
    }

    for (;;) {

        record = marc_record_create();

        if (record == NULL) {
            printf(
                "FAIL: Could not create MARC record "
                "for ISO 2709 input.\n"
            );

            fclose(input);
            return -1;
        }

        result = marc_record_read(
            record,
            input
        );

        if (result != 0) {

            /*
             * EOF after a complete record is expected.
             *
             * Unfortunately marc_record_read() currently
             * reports both malformed input and EOF through
             * its return value, so check the stream state.
             */
            if (feof(input)) {

                marc_record_free(record);
                break;
            }

            printf(
                "FAIL: ViiewLib rejected ISO 2709 "
                "record %zu.\n",
                count + 1
            );

            marc_record_free(record);
            fclose(input);
            return -1;
        }

        count++;

        {
            const char *id =
                marc_record_get_control_field(
                    record,
                    "001"
                );

            if (id != NULL &&
                strcmp(id, "007E01") == 0) {

                found_target = 1;

                printf(
                    "PASS: ISO 2709 record %zu "
                    "contains 001=007E01.\n",
                    count
                );
            }
        }

        marc_record_free(record);
    }

    fclose(input);

    printf(
        "ISO 2709 records read: %zu\n",
        count
    );

    if (count != 10170) {
        printf(
            "FAIL: Expected 10170 ISO 2709 records, "
            "got %zu.\n",
            count
        );

        return -1;
    }

    if (!found_target) {
        printf(
            "FAIL: 001=007E01 was not found in "
            "the encoded database.\n"
        );

        return -1;
    }

    printf(
        "PASS: All 10170 ISO 2709 records read successfully.\n"
    );

    printf(
        "PASS: Record 007E01 survived the ISO 2709 round-trip.\n"
    );

    return 0;
}


/*
 * Read the single-record .mrc file back through ViiewLib
 * and verify the MARC record contents.
 */
static int test_game_round_trip(void)
{
    FILE *input;
    MARC_Record *record;
    MARC_Field *field;
    MARC_Subfield *subfield;
    const char *value;
    int result;
    int found_245 = 0;

    printf(
        "\nTesting selected-game ISO 2709 round-trip...\n"
    );

    input = fopen(
        GAME_OUTPUT,
        "rb"
    );

    if (input == NULL) {
        printf(
            "FAIL: Could not open selected-game .mrc.\n"
        );

        return -1;
    }

    record = marc_record_create();

    if (record == NULL) {
        printf(
            "FAIL: Could not create MARC record.\n"
        );

        fclose(input);
        return -1;
    }

    result = marc_record_read(
        record,
        input
    );

    if (result != 0) {
        printf(
            "FAIL: ViiewLib could not read "
            "selected-game ISO 2709 record.\n"
        );

        marc_record_free(record);
        fclose(input);
        return -1;
    }

    fclose(input);

    /*
     * Verify 001.
     */
    value = marc_record_get_control_field(
        record,
        "001"
    );

    if (value == NULL ||
        strcmp(value, "007E01") != 0) {

        printf(
            "FAIL: Selected-game 001 did not survive "
            "round-trip.\n"
        );

        marc_record_free(record);
        return -1;
    }

    printf(
        "PASS: Selected-game 001=007E01.\n"
    );

    /*
     * Find 245.
     */
    {
        size_t i;
        size_t field_count =
            marc_record_get_field_count(record);

        for (i = 0; i < field_count; i++) {

            field = marc_record_get_field(
                record,
                i
            );

            if (field == NULL) {
                continue;
            }

            if (strcmp(
                    marc_field_get_tag(field),
                    "245"
                ) == 0) {

                found_245 = 1;
                break;
            }
        }
    }

    if (!found_245) {
        printf(
            "FAIL: Selected-game 245 field "
            "was not found.\n"
        );

        marc_record_free(record);
        return -1;
    }

    printf(
        "PASS: Selected-game 245 field found.\n"
    );

    /*
     * Verify first 245 subfield.
     */
    subfield = marc_field_get_subfield(
        field,
        0
    );

    if (subfield == NULL) {
        printf(
            "FAIL: Selected-game 245 has no subfield.\n"
        );

        marc_record_free(record);
        return -1;
    }

    if (marc_subfield_get_code(subfield) != 'a') {
        printf(
            "FAIL: Expected 245 $a.\n"
        );

        marc_record_free(record);
        return -1;
    }

    value = marc_subfield_get_value(
        subfield
    );

    if (value == NULL ||
        strcmp(value, "Auto Erase Disc") != 0) {

        printf(
            "FAIL: Expected 245 $a=Auto Erase Disc.\n"
        );

        if (value != NULL) {
            printf(
                "      Got: %s\n",
                value
            );
        }

        marc_record_free(record);
        return -1;
    }

    printf(
        "PASS: Selected-game 245 $a=Auto Erase Disc.\n"
    );

    marc_record_free(record);

    return 0;
}


int main(void)
{
    printf(
        "MarcViiew encoder/parser test\n"
        "==============================\n\n"
    );

    printf(
        "Database:\n%s\n\n",
        DATABASE_PATH
    );

    /*
     * 1. Database access.
     */
    if (test_database_access() != 0) {
        return 1;
    }

    /*
     * 2. Text parser.
     */
    if (test_parser() != 0) {
        return 1;
    }

    /*
     * 3. Complete database encoding.
     */
    if (test_database_encoding() != 0) {
        return 1;
    }

    /*
     * 4. Selected game encoding.
     */
    if (test_game_encoding() != 0) {
        return 1;
    }

    /*
     * 5. Read the entire generated database back.
     */
    if (test_database_round_trip() != 0) {
        return 1;
    }

    /*
     * 6. Read the selected game back.
     */
    if (test_game_round_trip() != 0) {
        return 1;
    }

    printf(
        "\n============================================\n"
        "ALL PARSER/ENCODER ROUND-TRIP TESTS PASSED!\n"
        "============================================\n"
    );

    return 0;
}