#include <stdio.h>
#include <string.h>

#include <gccore.h>
#include <fat.h>
#include <wiiuse/wpad.h>

#include "viiewlib/marc.h"


static void wait_for_button(void)
{
    WPAD_ScanPads();

    while (!(WPAD_ButtonsDown(0) & WPAD_BUTTON_A))
    {
        VIDEO_WaitVSync();
        WPAD_ScanPads();
    }
}


static void pause_message(
    const char *message
)
{
    printf("%s\n", message);
    printf("Press A to continue.\n\n");

    wait_for_button();
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

    printf(
        "Field count: %lu\n",
        (unsigned long)count
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
        {
            printf(
                "Field %lu: NULL\n",
                (unsigned long)i
            );

            continue;
        }

        field_tag =
            marc_field_get_tag(
                field
            );

        if (field_tag == NULL)
        {
            printf(
                "Field %lu: tag is NULL\n",
                (unsigned long)i
            );

            continue;
        }

        printf(
            "Field %lu tag: [%s]\n",
            (unsigned long)i,
            field_tag
        );

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


int main(
    int argc,
    char **argv
)
{
    MARC_Record *record;
    MARC_Record *loaded;

    MARC_Field *title;
    MARC_Field *loaded_title;

    MARC_Subfield *subfield;

    FILE *file;

    int result;

    const char *control_001;
    const char *value;

    char code;


    (void)argc;
    (void)argv;


    /*
     * ----------------------------------------------------------------------
     * Wii video/input initialisation
     * ----------------------------------------------------------------------
     */

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
        "ViiewLib Wii ISO 2709 Test\n"
    );

    printf(
        "==========================\n\n"
    );


    /*
     * ----------------------------------------------------------------------
     * FAT
     * ----------------------------------------------------------------------
     */

    printf(
        "[1] Initialising FAT...\n"
    );

    if (!fatInitDefault())
    {
        printf(
            "FAIL: fatInitDefault()\n"
        );

        wait_for_button();

        return 1;
    }

    printf(
        "PASS: FAT initialised.\n\n"
    );


    /*
     * ----------------------------------------------------------------------
     * Create MARC record
     * ----------------------------------------------------------------------
     */

    printf(
        "[2] Creating MARC record...\n"
    );

    record =
        marc_record_create();

    if (record == NULL)
    {
        printf(
            "FAIL: marc_record_create()\n"
        );

        wait_for_button();

        return 1;
    }

    printf(
        "PASS: Record created.\n\n"
    );


    /*
     * ----------------------------------------------------------------------
     * Control field 001
     * ----------------------------------------------------------------------
     */

    printf(
        "[3] Setting 001...\n"
    );

    result =
        marc_record_set_control_field(
            record,
            "001",
            "WII-ISO-001"
        );

    if (result != 0)
    {
        printf(
            "FAIL: marc_record_set_control_field()\n"
        );

        marc_record_free(record);

        wait_for_button();

        return 1;
    }

    printf(
        "PASS: 001 set.\n\n"
    );


    /*
     * ----------------------------------------------------------------------
     * 245
     * ----------------------------------------------------------------------
     */

    printf(
        "[4] Creating 245...\n"
    );

    title =
        marc_field_create(
            "245",
            1,
            0
        );

    if (title == NULL)
    {
        printf(
            "FAIL: marc_field_create()\n"
        );

        marc_record_free(record);

        wait_for_button();

        return 1;
    }

    printf(
        "PASS: 245 created.\n"
    );


    printf(
        "Adding 245 $a...\n"
    );

    result =
        marc_field_add_subfield(
            title,
            'a',
            "ViiewLib Wii ISO 2709 Test"
        );

    if (result != 0)
    {
        printf(
            "FAIL: marc_field_add_subfield()\n"
        );

        marc_field_free(title);
        marc_record_free(record);

        wait_for_button();

        return 1;
    }

    printf(
        "PASS: 245 $a added.\n"
    );


    printf(
        "Original 245 subfield count: %lu\n",
        (unsigned long)
            marc_field_get_subfield_count(
                title
            )
    );


    /*
     * Verify the original record using the index-based
     * accessor before writing it.
     */
    subfield =
        marc_field_get_subfield(
            title,
            0
        );

    if (subfield == NULL)
    {
        printf(
            "FAIL: Could not retrieve original 245 subfield index 0.\n"
        );

        marc_field_free(title);
        marc_record_free(record);

        wait_for_button();

        return 1;
    }

    code =
        marc_subfield_get_code(
            subfield
        );

    value =
        marc_subfield_get_value(
            subfield
        );

    printf(
        "Original subfield index 0:\n"
    );

    printf(
        "  Code:  $%c\n",
        code
    );

    printf(
        "  Value: [%s]\n\n",
        value != NULL ? value : "(NULL)"
    );

    if (code != 'a')
    {
        printf(
            "FAIL: Original subfield code is not $a.\n"
        );

        marc_field_free(title);
        marc_record_free(record);

        wait_for_button();

        return 1;
    }

    if (value == NULL ||
        strcmp(
            value,
            "ViiewLib Wii ISO 2709 Test"
        ) != 0)
    {
        printf(
            "FAIL: Original subfield value is incorrect.\n"
        );

        marc_field_free(title);
        marc_record_free(record);

        wait_for_button();

        return 1;
    }

    printf(
        "PASS: Original 245 $a verified.\n\n"
    );


    printf(
        "Adding 245 to record...\n"
    );

    result =
        marc_record_add_field(
            record,
            title
        );

    if (result != 0)
    {
        printf(
            "FAIL: marc_record_add_field()\n"
        );

        marc_field_free(title);
        marc_record_free(record);

        wait_for_button();

        return 1;
    }

    printf(
        "PASS: 245 added.\n\n"
    );


    /*
     * ----------------------------------------------------------------------
     * Write ISO 2709
     * ----------------------------------------------------------------------
     */

    printf(
        "[5] Opening SD output file...\n"
    );

    file =
        fopen(
            "sd:/viiewlib_test.mrc",
            "wb"
        );

    if (file == NULL)
    {
        printf(
            "FAIL: fopen() for writing.\n"
        );

        marc_record_free(record);

        wait_for_button();

        return 1;
    }

    printf(
        "PASS: Output file opened.\n\n"
    );


    printf(
        "[6] Calling marc_record_write()...\n"
    );

    result =
        marc_record_write(
            record,
            file
        );

    printf(
        "marc_record_write() returned: %d\n",
        result
    );

    if (result != 0)
    {
        printf(
            "FAIL: ISO 2709 write.\n"
        );

        fclose(file);
        marc_record_free(record);

        wait_for_button();

        return 1;
    }

    printf(
        "PASS: ISO 2709 write completed.\n"
    );

    fclose(file);

    printf(
        "PASS: Output file closed.\n\n"
    );


    pause_message(
        "[WRITE COMPLETE]\n"
        "The .mrc file should now exist on the SD card."
    );


    /*
     * ----------------------------------------------------------------------
     * Free original
     * ----------------------------------------------------------------------
     */

    printf(
        "[7] Freeing original record...\n"
    );

    marc_record_free(record);

    printf(
        "PASS: Original record freed.\n\n"
    );


    /*
     * ----------------------------------------------------------------------
     * Re-open ISO 2709
     * ----------------------------------------------------------------------
     */

    printf(
        "[8] Re-opening ISO 2709 file...\n"
    );

    file =
        fopen(
            "sd:/viiewlib_test.mrc",
            "rb"
        );

    if (file == NULL)
    {
        printf(
            "FAIL: Could not reopen .mrc file.\n"
        );

        wait_for_button();

        return 1;
    }

    printf(
        "PASS: Input file opened.\n\n"
    );


    /*
     * ----------------------------------------------------------------------
     * Create destination record
     * ----------------------------------------------------------------------
     */

    printf(
        "[9] Creating destination record...\n"
    );

    loaded =
        marc_record_create();

    if (loaded == NULL)
    {
        printf(
            "FAIL: Could not create destination record.\n"
        );

        fclose(file);

        wait_for_button();

        return 1;
    }

    printf(
        "PASS: Destination record created.\n\n"
    );


    /*
     * ----------------------------------------------------------------------
     * Read ISO 2709
     * ----------------------------------------------------------------------
     */

    printf(
        "[10] Calling marc_record_read()...\n"
    );

    result =
        marc_record_read(
            loaded,
            file
        );

    printf(
        "marc_record_read() returned: %d\n",
        result
    );

    fclose(file);

    if (result != 0)
    {
        printf(
            "FAIL: ISO 2709 read.\n"
        );

        marc_record_free(loaded);

        wait_for_button();

        return 1;
    }

    printf(
        "PASS: ISO 2709 read completed.\n\n"
    );


    /*
     * ----------------------------------------------------------------------
     * Verify 001
     * ----------------------------------------------------------------------
     */

    printf(
        "[11] Verifying 001...\n"
    );

    control_001 =
        marc_record_get_control_field(
            loaded,
            "001"
        );

    if (control_001 == NULL)
    {
        printf(
            "FAIL: 001 missing.\n"
        );

        marc_record_free(loaded);

        wait_for_button();

        return 1;
    }

    printf(
        "Read 001: [%s]\n",
        control_001
    );

    if (strcmp(
            control_001,
            "WII-ISO-001"
        ) != 0)
    {
        printf(
            "FAIL: 001 value incorrect.\n"
        );

        marc_record_free(loaded);

        wait_for_button();

        return 1;
    }

    printf(
        "PASS: 001 verified.\n\n"
    );


    /*
     * ----------------------------------------------------------------------
     * Verify 245
     * ----------------------------------------------------------------------
     */

    printf(
        "[12] Verifying 245\n"
    );

    printf(
        "--------------------------\n"
    );

    loaded_title =
        find_field_by_tag(
            loaded,
            "245"
        );

    if (loaded_title == NULL)
    {
        printf(
            "\nFAIL: 245 field was NOT found.\n"
        );

        marc_record_free(loaded);

        wait_for_button();

        return 1;
    }

    printf(
        "\nPASS: 245 field found.\n\n"
    );


    /*
     * IMPORTANT:
     *
     * marc_field_get_subfield() is an INDEX accessor.
     *
     * The first subfield is index 0.
     *
     * Do NOT pass 'a' here.
     */
    printf(
        "245 subfield count: %lu\n",
        (unsigned long)
            marc_field_get_subfield_count(
                loaded_title
            )
    );

    printf(
        "Retrieving subfield index 0...\n"
    );

    subfield =
        marc_field_get_subfield(
            loaded_title,
            0
        );

    if (subfield == NULL)
    {
        printf(
            "\nFAIL: 245 subfield index 0 was NOT found.\n"
        );

        printf(
            "The reader created the 245 field, but\n"
            "the subfield accessor returned NULL.\n"
        );

        marc_record_free(loaded);

        wait_for_button();

        return 1;
    }

    printf(
        "PASS: 245 subfield index 0 found.\n\n"
    );


    /*
     * Check reconstructed subfield code.
     */

    code =
        marc_subfield_get_code(
            subfield
        );

    printf(
        "Reconstructed subfield code:\n"
    );

    printf(
        "  $%c\n\n",
        code
    );

    if (code != 'a')
    {
        printf(
            "FAIL: Expected subfield code $a.\n"
        );

        marc_record_free(loaded);

        wait_for_button();

        return 1;
    }

    printf(
        "PASS: Subfield code is $a.\n\n"
    );


    /*
     * Check reconstructed value.
     */

    value =
        marc_subfield_get_value(
            subfield
        );

    printf(
        "Read 245 $a:\n"
    );

    printf(
        "[%s]\n\n",
        value != NULL
            ? value
            : "(NULL)"
    );

    printf(
        "Expected:\n"
    );

    printf(
        "[ViiewLib Wii ISO 2709 Test]\n\n"
    );

    if (value == NULL)
    {
        printf(
            "FAIL: 245 $a value is NULL.\n"
        );

        marc_record_free(loaded);

        wait_for_button();

        return 1;
    }

    if (strcmp(
            value,
            "ViiewLib Wii ISO 2709 Test"
        ) != 0)
    {
        printf(
            "FAIL: 245 $a value is different.\n"
        );

        marc_record_free(loaded);

        wait_for_button();

        return 1;
    }

    printf(
        "PASS: 245 $a verified.\n\n"
    );


    /*
     * ----------------------------------------------------------------------
     * Success
     * ----------------------------------------------------------------------
     */

    marc_record_free(loaded);

    printf(
        "==========================\n"
    );

    printf(
        "ISO 2709 Wii test PASSED!\n"
    );

    printf(
        "==========================\n\n"
    );

    printf(
        "Verified:\n"
    );

    printf(
        "  001 control field\n"
    );

    printf(
        "  245 field\n"
    );

    printf(
        "  245 subfield index 0\n"
    );

    printf(
        "  245 $a code\n"
    );

    printf(
        "  245 $a value\n\n"
    );

    printf(
        "File:\n"
    );

    printf(
        "sd:/viiewlib_test.mrc\n\n"
    );

    printf(
        "Press A to exit.\n"
    );

    wait_for_button();

    return 0;
}