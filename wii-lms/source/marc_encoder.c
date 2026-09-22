#include <stdio.h>
#include <string.h>

#include <viiewlib/marc.h>

#include "marc_encoder.h"
#include "marc_text_parser.h"


/*
 * Encode the complete MarcViiew database.
 */
int marcviiew_encode_database(
    const char *input_path,
    const char *output_path
)
{
    FILE *input;
    FILE *output;

    MarcViiewParser parser;

    MARC_Record *record;

    size_t record_count;

    int result;
    int status;


    if (input_path == NULL ||
        output_path == NULL)
    {
        return -1;
    }


    input =
        fopen(
            input_path,
            "rb"
        );

    if (input == NULL)
        return -1;


    output =
        fopen(
            output_path,
            "wb"
        );

    if (output == NULL)
    {
        fclose(input);
        return -1;
    }


    if (marcviiew_parser_init(
            &parser,
            input
        ) != 0)
    {
        fclose(output);
        fclose(input);

        return -1;
    }


    record_count = 0;

    while (1)
    {
        record = NULL;

        result =
            marcviiew_parser_next(
                &parser,
                &record
            );

        /*
         * Normal end of database.
         */
        if (result == 0)
            break;


        /*
         * Parser error.
         */
        if (result < 0)
        {
            fclose(output);
            fclose(input);

            return -1;
        }


        /*
         * Defensive check.
         */
        if (record == NULL)
        {
            fclose(output);
            fclose(input);

            return -1;
        }


        /*
         * ViiewLib performs the actual ISO 2709
         * encoding.
         */
        status =
            marc_record_write(
                record,
                output
            );

        marc_record_free(
            record
        );


        if (status != 0)
        {
            fclose(output);
            fclose(input);

            return -1;
        }


        record_count++;
    }


    /*
     * An empty input database is not considered
     * a successful encoding.
     */
    if (record_count == 0)
    {
        fclose(output);
        fclose(input);

        return -1;
    }


    /*
     * Make sure all buffered output reached the file.
     */
    if (fflush(output) != 0)
    {
        fclose(output);
        fclose(input);

        return -1;
    }


    if (fclose(output) != 0)
    {
        fclose(input);
        return -1;
    }


    if (fclose(input) != 0)
        return -1;


    return 0;
}


/*
 * Encode exactly one game.
 */
int marcviiew_encode_game(
    const char *input_path,
    const char *output_path,
    const char *game_id
)
{
    FILE *input;
    FILE *output;

    MarcViiewParser parser;

    MARC_Record *record;

    int result;
    int status;

    const char *record_id;


    if (input_path == NULL ||
        output_path == NULL ||
        game_id == NULL ||
        game_id[0] == '\0')
    {
        return -1;
    }


    input =
        fopen(
            input_path,
            "rb"
        );

    if (input == NULL)
        return -1;


    if (marcviiew_parser_init(
            &parser,
            input
        ) != 0)
    {
        fclose(input);
        return -1;
    }


    while (1)
    {
        record = NULL;

        result =
            marcviiew_parser_next(
                &parser,
                &record
            );


        /*
         * Game was not found.
         */
        if (result == 0)
        {
            fclose(input);
            return -1;
        }


        /*
         * Database parsing failed.
         */
        if (result < 0)
        {
            fclose(input);
            return -1;
        }


        if (record == NULL)
        {
            fclose(input);
            return -1;
        }


        /*
         * The MarcViiew GAME_ID is represented in the
         * resulting MARC record by field 001.
         */
        record_id =
            marc_record_get_control_field(
                record,
                "001"
            );


        if (record_id != NULL &&
            strcmp(
                record_id,
                game_id
            ) == 0)
        {
            /*
             * We only open the output after finding
             * the requested game. This avoids creating
             * an empty .mrc when the ID does not exist.
             */
            output =
                fopen(
                    output_path,
                    "wb"
                );

            if (output == NULL)
            {
                marc_record_free(record);
                fclose(input);

                return -1;
            }


            status =
                marc_record_write(
                    record,
                    output
                );


            marc_record_free(record);


            if (status != 0)
            {
                fclose(output);
                fclose(input);

                return -1;
            }


            if (fflush(output) != 0)
            {
                fclose(output);
                fclose(input);

                return -1;
            }


            if (fclose(output) != 0)
            {
                fclose(input);
                return -1;
            }


            fclose(input);

            return 0;
        }


        marc_record_free(record);
    }
}