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

static MARC_Field *find_field_by_tag(
    MARC_Record *record,
    const char *tag
)
{
    size_t count;
    size_t i;

    count = marc_record_get_field_count(record);

    for (i = 0; i < count; i++)
    {
        MARC_Field *field;
        const char *field_tag;

        field = marc_record_get_field(record, i);

        if (field == NULL)
            continue;

        field_tag = marc_field_get_tag(field);

        if (field_tag == NULL)
            continue;

        if (strcmp(field_tag, tag) == 0)
            return field;
    }

    return NULL;
}

static void print_field(
    MARC_Record *record,
    const char *tag
)
{
    MARC_Field *field;
    size_t subfield_count;
    size_t i;

    field = find_field_by_tag(record, tag);

    if (field == NULL)
    {
        printf("%s: NOT FOUND\n", tag);
        return;
    }

    printf("%s", tag);

    printf(
        "  Indicators: %c %c\n",
        marc_field_get_indicator1(field),
        marc_field_get_indicator2(field)
    );

    subfield_count =
        marc_field_get_subfield_count(field);

    for (i = 0; i < subfield_count; i++)
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
            "  $%c: %s\n",
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
    MARC_Record *record;
    FILE *file;
    int result;

    const char *control_001;

    (void)argc;
    (void)argv;

    VIDEO_Init();
    WPAD_Init();

    GXRModeObj *rmode =
        VIDEO_GetPreferredMode(NULL);

    void *xfb =
        MEM_K0_TO_K1(
            SYS_AllocateFramebuffer(rmode)
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
        "ViiewLib MARC Import Test\n"
    );

    printf(
        "=========================\n\n"
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

    printf(
        "[2] Opening MARC file...\n"
    );

    file =
        fopen(
            "sd:/viiewlib_import_test.mrc",
            "rb"
        );

    if (file == NULL)
    {
        printf(
            "FAIL: Could not open:\n"
            "sd:/viiewlib_import_test.mrc\n\n"
        );

        printf(
            "Copy the test .mrc file to the SD card\n"
            "root and try again.\n"
        );

        wait_for_button();

        return 1;
    }

    printf(
        "PASS: MARC file opened.\n\n"
    );

    printf(
        "[3] Creating MARC record...\n"
    );

    record =
        marc_record_create();

    if (record == NULL)
    {
        printf(
            "FAIL: Could not create MARC record.\n"
        );

        fclose(file);

        wait_for_button();

        return 1;
    }

    printf(
        "PASS: MARC record created.\n\n"
    );

    printf(
        "[4] Importing ISO 2709 record...\n"
    );

    result =
        marc_record_read(
            record,
            file
        );

    fclose(file);

    printf(
        "marc_record_read() returned: %d\n",
        result
    );

    if (result != 0)
    {
        printf(
            "FAIL: MARC import failed.\n"
        );

        marc_record_free(record);

        wait_for_button();

        return 1;
    }

    printf(
        "PASS: MARC record imported.\n\n"
    );

    printf(
        "[5] Checking 001...\n"
    );

    control_001 =
        marc_record_get_control_field(
            record,
            "001"
        );

    if (control_001 == NULL)
    {
        printf(
            "FAIL: 001 not found.\n"
        );

        marc_record_free(record);

        wait_for_button();

        return 1;
    }

    printf(
        "001: [%s]\n",
        control_001
    );

    printf(
        "PASS: 001 found.\n\n"
    );

    printf(
        "[6] Reading 245...\n"
    );

    print_field(
        record,
        "245"
    );

    printf(
        "\n"
    );

    printf(
        "[7] Reading 264...\n"
    );

    print_field(
        record,
        "264"
    );

    printf(
        "\n"
    );

    printf(
        "[8] Reading 650...\n"
    );

    print_field(
        record,
        "650"
    );

    printf(
        "\n"
    );

    printf(
        "=========================\n"
    );

    printf(
        "MARC IMPORT PASSED!\n"
    );

    printf(
        "=========================\n\n"
    );

    printf(
        "ViiewLib successfully imported\n"
        "the ISO 2709 MARC record on Wii.\n\n"
    );

    marc_record_free(record);

    printf(
        "Press A to exit.\n"
    );

    wait_for_button();

    return 0;
}