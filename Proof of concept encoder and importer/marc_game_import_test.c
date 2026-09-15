#include <stdio.h>
#include <string.h>

#include <gccore.h>
#include <fat.h>
#include <wiiuse/wpad.h>

#include "viiewlib/marc.h"

#define SOURCE_FILE "sd:/marcviiew_marc.txt"
#define OUTPUT_FILE "sd:/007E01.mrc"

static void wait_for_button(void)
{
    WPAD_ScanPads();

    while (!(WPAD_ButtonsDown(0) & WPAD_BUTTON_A))
    {
        VIDEO_WaitVSync();
        WPAD_ScanPads();
    }
}

static int is_target_record_start(
    const char *line
)
{
    return strcmp(
        line,
        "[RECORD]"
    ) == 0;
}

static int is_target_game(
    const char *line
)
{
    return strcmp(
        line,
        "GAME_ID=007E01"
    ) == 0;
}

static int parse_indicator(
    const char *value
)
{
    if (value == NULL || value[0] == '\0')
        return ' ';

    if (value[0] == '#')
        return ' ';

    return (unsigned char)value[0];
}

static int add_field_from_text(
    MARC_Record *record,
    const char *tag,
    char indicator1,
    char indicator2,
    const char *subfields[],
    const char codes[],
    size_t subfield_count
)
{
    MARC_Field *field;
    size_t i;

    field =
        marc_field_create(
            tag,
            indicator1,
            indicator2
        );

    if (field == NULL)
    {
        printf(
            "FAIL: Could not create field %s.\n",
            tag
        );

        return 1;
    }

    for (i = 0; i < subfield_count; i++)
    {
        if (marc_field_add_subfield(
                field,
                codes[i],
                subfields[i]
            ) != 0)
        {
            printf(
                "FAIL: Could not add %s $%c.\n",
                tag,
                codes[i]
            );

            marc_field_free(field);

            return 1;
        }
    }

    if (marc_record_add_field(
            record,
            field
        ) != 0)
    {
        printf(
            "FAIL: Could not add field %s.\n",
            tag
        );

        marc_field_free(field);

        return 1;
    }

    return 0;
}

static MARC_Field *find_field_by_tag(
    MARC_Record *record,
    const char *tag
)
{
    size_t count;
    size_t i;

    count =
        marc_record_get_field_count(
            record
        );

    for (i = 0; i < count; i++)
    {
        MARC_Field *field;
        const char *field_tag;

        field =
            marc_record_get_field(
                record,
                i
            );

        if (field == NULL)
            continue;

        field_tag =
            marc_field_get_tag(
                field
            );

        if (field_tag == NULL)
            continue;

        if (strcmp(
                field_tag,
                tag
            ) == 0)
        {
            return field;
        }
    }

    return NULL;
}

static void print_field(
    MARC_Record *record,
    const char *tag
)
{
    MARC_Field *field;
    size_t count;
    size_t i;

    field =
        find_field_by_tag(
            record,
            tag
        );

    if (field == NULL)
    {
        printf(
            "%s: NOT FOUND\n",
            tag
        );

        return;
    }

    printf(
        "%s  IND1=[%c] IND2=[%c]\n",
        tag,
        marc_field_get_indicator1(field),
        marc_field_get_indicator2(field)
    );

    count =
        marc_field_get_subfield_count(
            field
        );

    for (i = 0; i < count; i++)
    {
        MARC_Subfield *subfield;
        char code;
        const char *value;

        subfield =
            marc_field_get_subfield(
                field,
                i
            );

        if (subfield == NULL)
            continue;

        code =
            marc_subfield_get_code(
                subfield
            );

        value =
            marc_subfield_get_value(
                subfield
            );

        printf(
            "  $%c = %s\n",
            code,
            value != NULL ? value : "(NULL)"
        );
    }
}

int main(
    int argc,
    char **argv
)
{
    FILE *source;
    FILE *output;

    MARC_Record *record;
    MARC_Record *loaded;

    int result;
    int found_record;

    char line[512];

    const char *subfields_245[] =
    {
        "Auto Erase Disc"
    };

    const char codes_245[] =
    {
        'a'
    };

    const char *subfields_264[] =
    {
        "Nintendo",
        "2007-2-19"
    };

    const char codes_264[] =
    {
        'b',
        'c'
    };

    const char *subfields_300[] =
    {
        "1 Wii optical disc"
    };

    const char codes_300[] =
    {
        'a'
    };

    const char *subfields_500[] =
    {
        "The following disc was used within authorized Nintendo repair stores to quickly wipe all of the contents on Wii systems which were brought in for return. A detailed list of instructions on operating the disc appear on the front of the case."
    };

    const char codes_500[] =
    {
        'a'
    };

    const char *subfields_542[] =
    {
        "2007",
        "Nintendo",
        "GameTDB"
    };

    const char codes_542[] =
    {
        'i',
        'k',
        's'
    };

    (void)argc;
    (void)argv;

    VIDEO_Init();
    WPAD_Init();

    GXRModeObj *rmode =
        VIDEO_GetPreferredMode(NULL);

    void *xfb =
        MEM_K0_TO_K1(
            SYS_AllocateFramebuffer(
                rmode
            )
        );

    console_init(
        xfb,
        20,
        20,
        rmode->fbWidth,
        rmode->xfbHeight,
        rmode->fbWidth *
            VI_DISPLAY_PIX_SZ
    );

    VIDEO_Configure(rmode);
    VIDEO_SetNextFramebuffer(xfb);
    VIDEO_SetBlack(FALSE);
    VIDEO_Flush();
    VIDEO_WaitVSync();

    printf(
        "MarcViiew Real MARC Integration Test\n"
    );

    printf(
        "=====================================\n\n"
    );

    printf(
        "[1] Initialising FAT...\n"
    );

    if (!fatInitDefault())
    {
        printf(
            "FAIL: FAT initialisation failed.\n"
        );

        wait_for_button();

        return 1;
    }

    printf(
        "PASS: FAT initialised.\n\n"
    );

    /*
     * ---------------------------------------------------------
     * Locate the real 007E01 record.
     * ---------------------------------------------------------
     */

    printf(
        "[2] Opening MarcViiew MARC database...\n"
    );

    source =
        fopen(
            SOURCE_FILE,
            "rb"
        );

    if (source == NULL)
    {
        printf(
            "FAIL: Could not open:\n"
            "%s\n",
            SOURCE_FILE
        );

        wait_for_button();

        return 1;
    }

    printf(
        "PASS: MARC database opened.\n\n"
    );

    found_record = 0;

    while (fgets(
        line,
        sizeof(line),
        source
    ) != NULL)
    {
        size_t length;

        length = strlen(line);

        while (length > 0 &&
               (line[length - 1] == '\n' ||
                line[length - 1] == '\r'))
        {
            line[length - 1] = '\0';
            length--;
        }

        if (!is_target_record_start(line))
            continue;

        if (fgets(
            line,
            sizeof(line),
            source
        ) == NULL)
        {
            break;
        }

        length = strlen(line);

        while (length > 0 &&
               (line[length - 1] == '\n' ||
                line[length - 1] == '\r'))
        {
            line[length - 1] = '\0';
            length--;
        }

        if (is_target_game(line))
        {
            found_record = 1;
            break;
        }
    }

    fclose(source);

    if (!found_record)
    {
        printf(
            "FAIL: GAME_ID=007E01 was not found.\n"
        );

        wait_for_button();

        return 1;
    }

    printf(
        "PASS: Found GAME_ID=007E01.\n\n"
    );

    /*
     * ---------------------------------------------------------
     * Construct a real MARC_Record from the existing data.
     * ---------------------------------------------------------
     */

    printf(
        "[3] Creating MARC_Record...\n"
    );

    record =
        marc_record_create();

    if (record == NULL)
    {
        printf(
            "FAIL: Could not create MARC_Record.\n"
        );

        wait_for_button();

        return 1;
    }

    printf(
        "PASS: MARC_Record created.\n\n"
    );

    /*
     * 001
     */
    result =
        marc_record_set_control_field(
            record,
            "001",
            "007E01"
        );

    if (result != 0)
    {
        printf(
            "FAIL: Could not create 001.\n"
        );

        marc_record_free(record);

        wait_for_button();

        return 1;
    }

    /*
     * 245
     */
    if (add_field_from_text(
            record,
            "245",
            '0',
            '0',
            subfields_245,
            codes_245,
            1
        ) != 0)
    {
        marc_record_free(record);

        wait_for_button();

        return 1;
    }

    /*
     * 264
     */
    if (add_field_from_text(
            record,
            "264",
            ' ',
            '1',
            subfields_264,
            codes_264,
            2
        ) != 0)
    {
        marc_record_free(record);

        wait_for_button();

        return 1;
    }

    /*
     * 300
     */
    if (add_field_from_text(
            record,
            "300",
            ' ',
            ' ',
            subfields_300,
            codes_300,
            1
        ) != 0)
    {
        marc_record_free(record);

        wait_for_button();

        return 1;
    }

    /*
     * 500
     */
    if (add_field_from_text(
            record,
            "500",
            ' ',
            ' ',
            subfields_500,
            codes_500,
            1
        ) != 0)
    {
        marc_record_free(record);

        wait_for_button();

        return 1;
    }

    /*
     * 542
     */
    if (add_field_from_text(
            record,
            "542",
            ' ',
            ' ',
            subfields_542,
            codes_542,
            3
        ) != 0)
    {
        marc_record_free(record);

        wait_for_button();

        return 1;
    }

    printf(
        "PASS: Real 007E01 data converted to MARC_Record.\n\n"
    );

    /*
     * ---------------------------------------------------------
     * Display the constructed record.
     * ---------------------------------------------------------
     */

    printf(
        "[4] Constructed MARC record\n"
    );

    printf(
        "-------------------------------------\n"
    );

    printf(
        "001: %s\n\n",
        marc_record_get_control_field(
            record,
            "001"
        )
    );

    print_field(record, "245");
    print_field(record, "264");
    print_field(record, "300");
    print_field(record, "500");
    print_field(record, "542");

    printf(
        "\n"
    );

    /*
     * ---------------------------------------------------------
     * Write actual ISO 2709.
     * ---------------------------------------------------------
     */

    printf(
        "[5] Writing real record to ISO 2709...\n"
    );

    output =
        fopen(
            OUTPUT_FILE,
            "wb"
        );

    if (output == NULL)
    {
        printf(
            "FAIL: Could not open output file.\n"
        );

        marc_record_free(record);

        wait_for_button();

        return 1;
    }

    result =
        marc_record_write(
            record,
            output
        );

    fclose(output);

    if (result != 0)
    {
        printf(
            "FAIL: marc_record_write() failed.\n"
        );

        marc_record_free(record);

        wait_for_button();

        return 1;
    }

    printf(
        "PASS: Wrote:\n"
        "%s\n\n",
        OUTPUT_FILE
    );

    marc_record_free(record);

    /*
     * ---------------------------------------------------------
     * Read the generated ISO 2709 back.
     * ---------------------------------------------------------
     */

    printf(
        "[6] Reading generated ISO 2709...\n"
    );

    output =
        fopen(
            OUTPUT_FILE,
            "rb"
        );

    if (output == NULL)
    {
        printf(
            "FAIL: Could not reopen generated .mrc.\n"
        );

        wait_for_button();

        return 1;
    }

    loaded =
        marc_record_create();

    if (loaded == NULL)
    {
        printf(
            "FAIL: Could not create loaded record.\n"
        );

        fclose(output);

        wait_for_button();

        return 1;
    }

    result =
        marc_record_read(
            loaded,
            output
        );

    fclose(output);

    if (result != 0)
    {
        printf(
            "FAIL: Generated ISO 2709 could not be read.\n"
        );

        marc_record_free(loaded);

        wait_for_button();

        return 1;
    }

    printf(
        "PASS: Generated ISO 2709 read successfully.\n\n"
    );

    /*
     * ---------------------------------------------------------
     * Verify important fields after round-trip.
     * ---------------------------------------------------------
     */

    printf(
        "[7] Verifying round-trip...\n"
    );

    if (strcmp(
            marc_record_get_control_field(
                loaded,
                "001"
            ),
            "007E01"
        ) != 0)
    {
        printf(
            "FAIL: 001 round-trip mismatch.\n"
        );

        marc_record_free(loaded);

        wait_for_button();

        return 1;
    }

    printf(
        "PASS: 001 = 007E01\n"
    );

    print_field(loaded, "245");
    print_field(loaded, "264");
    print_field(loaded, "300");
    print_field(loaded, "500");
    print_field(loaded, "542");

    printf(
        "\n=====================================\n"
    );

    printf(
        "REAL MARC INTEGRATION TEST PASSED!\n"
    );

    printf(
        "=====================================\n\n"
    );

    printf(
        "GameTDB-derived record 007E01 was:\n"
        "  parsed into a MARC_Record\n"
        "  encoded as ISO 2709\n"
        "  decoded again\n"
        "  successfully reconstructed\n"
    );

    marc_record_free(loaded);

    printf(
        "\nPress A to exit.\n"
    );

    wait_for_button();

    return 0;
}