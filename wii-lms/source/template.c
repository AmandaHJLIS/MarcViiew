#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <ctype.h>
#include <sys/stat.h>
#include <malloc.h>

#include <gccore.h>
#include <wiiuse/wpad.h>
#include <fat.h>
#include <ogc/es.h>
#include <ogc/isfs.h>
#include "nand_ios.h"

#include <viiewlib/marc.h>

#include "marc_encoder.h"
#include "marc_text_parser.h"

#define MAX_GAMES 100
#define MAX_SEARCH_RESULTS 100
#define MAX_IMPORTED_RECORDS 100
#define IMPORTED_RECORD_FILENAME_LENGTH 64
#define IMPORTED_RECORD_TITLE_LENGTH 100
#define IMPORTED_RECORD_ID_LENGTH 7
#define IMPORTED_RECORD_001_LENGTH 100

#define DATABASE_LINE_LENGTH 4096
#define MAX_INFO_LINES 100
#define INFO_LINE_LENGTH 100

#define MAX_MARC_LINES 300
#define MARC_LINE_LENGTH 100

#define SEARCH_LENGTH 100

#define CONSOLE_COLUMNS 80
#define UI_WIDTH 50
#define GAME_LIST_VISIBLE_ITEMS 15


static void *xfb = NULL;
static GXRModeObj *rmode = NULL;

static int menu_selection = 0;
static int screen = 0;

static int catalogue_selection = 0;
static int catalogue_scroll = 0;
static int marc_menu_scroll = 0;
static int game_count = 0;
static int nand_title_count = 0;
static int nand_game_count = 0;

static int info_scroll = 0;

static int marc_selection = 0;
static int marc_scroll = 0;
static int marc_line_count = 0;

static int search_selection = 0;
static int search_result_count = 0;

static char search_query[SEARCH_LENGTH];
static int search_length = 0;


#define SEARCH_MODE_NORMAL 0
#define SEARCH_MODE_MARC21 1

static int search_mode = SEARCH_MODE_NORMAL;
static int search_mode_selection = 0;


/*
    Tracks how the current MARC record screen was opened.

    0 = opened from the main MARC Records menu
    1 = opened from MARC21 search results
*/
static int marc_record_from_search = 0;


/*
    Settings screens.

    0 = Reload SD / USB
    1 = Reload NAND
    2 = Reload Databases
    3 = Credits
*/
static int settings_selection = 0;


/*
    Stores the result of the most recent settings
    operation so the settings screen can display
    a useful status message.
*/
static char settings_status[160] = "";

void show_encode_marc(void);

/*
    Encode MARC screens.

    Screen 11 = Encode MARC menu
    Screen 12 = Select game for MARC export
*/
static int encode_selection = 0;
static int encode_game_selection = 0;

static char encode_status[160] = "";


#define INPUT_UP       (1 << 0)
#define INPUT_DOWN     (1 << 1)
#define INPUT_LEFT     (1 << 2)
#define INPUT_RIGHT    (1 << 3)
#define INPUT_SELECT   (1 << 4)
#define INPUT_BACK     (1 << 5)
#define INPUT_PLUS     (1 << 6)
#define INPUT_HOME     (1 << 7)


void print_spaces(
    int count
) {

    for (
        int i = 0;
        i < count;
        i++
    )
        putchar(' ');
}


void print_centered(
    const char *text
) {

    int length =
        strlen(text);

    int padding =
        (CONSOLE_COLUMNS - length) / 2;

    if (
        padding < 0
    )
        padding = 0;

    print_spaces(
        padding
    );

    printf(
        "%s\n",
        text
    );
}


void print_ui_line(
    char character
) {

    int padding =
        (CONSOLE_COLUMNS - UI_WIDTH) / 2;

    print_spaces(
        padding
    );

    for (
        int i = 0;
        i < UI_WIDTH;
        i++
    )
        putchar(character);

    putchar('\n');
}


void print_menu_item(
    const char *text,
    int selected
) {

    char line[160];

    if (
        selected
    ) {

        snprintf(
            line,
            sizeof(line),
            "> %s",
            text
        );

    } else {

        snprintf(
            line,
            sizeof(line),
            "  %s",
            text
        );
    }

    print_centered(
        line
    );
}


void print_label_value(
    const char *label,
    const char *value
) {

    char line[160];

    snprintf(
        line,
        sizeof(line),
        "%s%s",
        label,
        value
    );

    print_centered(
        line
    );
}


u32 get_input() {

    u32 pressed =
        WPAD_ButtonsDown(0);

    u32 input = 0;


    if (
        pressed & WPAD_BUTTON_UP
    )
        input |= INPUT_UP;

    if (
        pressed & WPAD_CLASSIC_BUTTON_UP
    )
        input |= INPUT_UP;


    if (
        pressed & WPAD_BUTTON_DOWN
    )
        input |= INPUT_DOWN;

    if (
        pressed & WPAD_CLASSIC_BUTTON_DOWN
    )
        input |= INPUT_DOWN;


    if (
        pressed & WPAD_BUTTON_LEFT
    )
        input |= INPUT_LEFT;

    if (
        pressed & WPAD_CLASSIC_BUTTON_LEFT
    )
        input |= INPUT_LEFT;


    if (
        pressed & WPAD_BUTTON_RIGHT
    )
        input |= INPUT_RIGHT;

    if (
        pressed & WPAD_CLASSIC_BUTTON_RIGHT
    )
        input |= INPUT_RIGHT;


    if (
        pressed & WPAD_BUTTON_A
    )
        input |= INPUT_SELECT;

    if (
        pressed & WPAD_CLASSIC_BUTTON_A
    )
        input |= INPUT_SELECT;


    if (
        pressed & WPAD_BUTTON_B
    )
        input |= INPUT_BACK;

    if (
        pressed & WPAD_CLASSIC_BUTTON_B
    )
        input |= INPUT_BACK;


    if (
        pressed & WPAD_BUTTON_PLUS
    )
        input |= INPUT_PLUS;

    if (
        pressed & WPAD_CLASSIC_BUTTON_PLUS
    )
        input |= INPUT_PLUS;


    if (
        pressed & WPAD_BUTTON_HOME
    )
        input |= INPUT_HOME;


    if (
        pressed & WPAD_CLASSIC_BUTTON_HOME
    )
        input |= INPUT_HOME;


    return input;
}


#define KEYBOARD_ROWS 4
#define KEYBOARD_COLUMNS 10

static const char keyboard[KEYBOARD_ROWS][KEYBOARD_COLUMNS + 1] = {
    "ABCDEFGHIJ",
    "KLMNOPQRST",
    "UVWXYZ0123",
    "456789"
};

static int keyboard_row = 0;
static int keyboard_column = 0;
static int keyboard_special_selection = 0;


typedef struct {

    char title[100];
    char id[7];
    char platform[30];
    char region[20];
    char release_date[30];
    char publisher[50];
    char developer[50];
    char genre[50];
    char series[50];
    char source[20];
    char distribution[20];
    char synopsis[3000];

} Game;


static Game games[MAX_GAMES];


static int search_results[MAX_SEARCH_RESULTS];


static char marc_lines[
    MAX_MARC_LINES
][
    MARC_LINE_LENGTH
];



typedef struct {
    char filename[IMPORTED_RECORD_FILENAME_LENGTH];
    char title[IMPORTED_RECORD_TITLE_LENGTH];
    char game_id[IMPORTED_RECORD_ID_LENGTH];
    char marc_001[IMPORTED_RECORD_001_LENGTH];
} ImportedRecord;

static ImportedRecord imported_records[MAX_IMPORTED_RECORDS];
static int imported_record_count = 0;
static int imported_selection = 0;
static int imported_scroll = 0;

#define NAND_ENTRY_NAME_LENGTH 13
#define NAND_MAX_ENTRIES 512

#define NAND_ENTRY_DIRECTORY 1
#define NAND_ENTRY_FILE 2
#define NAND_ENTRY_UNKNOWN 3
#define NAND_ENTRY_NOACCESS 4

typedef struct
{
    char name[NAND_ENTRY_NAME_LENGTH];
    char path[ISFS_MAXPATH];
    u32 size;
    u8 type;
} NandEntry;

#define NAND_MAX_TMD_CONTENTS 512

typedef struct
{
    u32 content_id;
    u16 index;
    u16 type;
    u64 size;
    u8 hash[20];
} NandContentInfo;

static NandEntry nand_entries[NAND_MAX_ENTRIES];
static NandContentInfo nand_tmd_contents[NAND_MAX_TMD_CONTENTS];
static u32 nand_tmd_content_count = 0;
static int nand_tmd_loaded = 0;
static u32 nand_entry_count = 0;
static u32 nand_total_entry_count = 0;
static int nand_selection = 0;
static int nand_scroll = 0;
static int nand_initialized = 0;
static char nand_current_path[ISFS_MAXPATH] = "/";
static char nand_status[160] = "";
static u64 nand_original_titleid = 0;
static int nand_original_titleid_valid = 0;
static u64 nand_active_titleid = 0;
static int nand_active_titleid_valid = 0;

static fstats nand_file_stats ATTRIBUTE_ALIGN(32);

static int nand_is_known_directory(const char *path, const char *name)
{
    static const char *const root_directories[] = {
        "import",
        "meta",
        "shared1",
        "shared2",
        "sys",
        "ticket",
        "title",
        "tmp",
        "wfs"
    };
    size_t i;

    if (path == NULL || name == NULL || strcmp(path, "/") != 0)
        return 0;

    for (i = 0; i < sizeof(root_directories) / sizeof(root_directories[0]); ++i)
    {
        if (strcmp(name, root_directories[i]) == 0)
            return 1;
    }

    return 0;
}

static MARC_Record *loaded_marc_record = NULL;

static void render_marc_record(MARC_Record *record);
static MARC_Record *read_mrc_record(FILE *file);
static void imported_update_metadata_from_record(
    ImportedRecord *imported,
    MARC_Record *record
);
static int imported_load_metadata(
    ImportedRecord *imported
);

static int imported_get_id(const char *filename, char *id)
{
    const char *dot;
    size_t id_length;

    if (filename == NULL || id == NULL)
        return 0;

    if (strncmp(filename, "marcviiew_", 10) != 0)
        return 0;

    dot = strrchr(filename, '.');

    if (dot == NULL || strcmp(dot, ".mrc") != 0)
        return 0;

    id_length = (size_t)(dot - (filename + 10));

    /*
     * Accept both unbracketed and bracketed filenames. Wii software
     * identifiers are not all the same length: disc-style records
     * may use six-character IDs, while channel/WiiWare records can
     * use four-character IDs such as HCMP.
     *
     * Examples:
     *   marcviiew_HCMP.mrc
     *   marcviiew_[HCMP].mrc
     *   marcviiew_RMCP01.mrc
     *   marcviiew_[RMCP01].mrc
     */
    if (id_length >= 4 && id_length <= 6)
    {
        memcpy(id, filename + 10, id_length);
        id[id_length] = '\0';
        return 1;
    }

    if (id_length >= 6 && id_length <= 8 &&
        filename[10] == '[' &&
        filename[id_length + 9] == ']')
    {
        size_t bracketed_id_length = id_length - 2;

        if (bracketed_id_length >= 4 &&
            bracketed_id_length <= 6)
        {
            memcpy(id, filename + 11, bracketed_id_length);
            id[bracketed_id_length] = '\0';
            return 1;
        }
    }

    return 0;
}

static int imported_subfield(
    MARC_Field *field,
    char code,
    char *output,
    size_t output_size
)
{
    size_t i;
    size_t count;

    if (field == NULL || output == NULL || output_size == 0)
        return 0;

    count = marc_field_get_subfield_count(field);

    for (i = 0; i < count; ++i)
    {
        MARC_Subfield *subfield =
            marc_field_get_subfield(field, i);
        const char *value;

        if (subfield == NULL ||
            marc_subfield_get_code(subfield) != code)
            continue;

        value = marc_subfield_get_value(subfield);

        if (value == NULL)
            return 0;

        strncpy(output, value, output_size - 1);
        output[output_size - 1] = '\0';
        return 1;
    }

    return 0;
}

static int imported_exists(const ImportedRecord *record)
{
    int i;

    if (record == NULL)
        return 0;

    for (i = 0; i < imported_record_count; ++i)
    {
        if (strcmp(imported_records[i].filename, record->filename) == 0 ||
            strcmp(imported_records[i].game_id, record->game_id) == 0)
            return 1;
    }

    return 0;
}
static void imported_add_filename(const char *filename)
{
    ImportedRecord record;

    if (filename == NULL ||
        imported_record_count >= MAX_IMPORTED_RECORDS)
        return;

    memset(&record, 0, sizeof(record));

    if (!imported_get_id(filename, record.game_id))
        return;

    strncpy(
        record.filename,
        filename,
        sizeof(record.filename) - 1
    );

    snprintf(
        record.title,
        sizeof(record.title),
        "Imported MARC record (%s)",
        record.game_id
    );

    strcpy(
        record.marc_001,
        "(not loaded)"
    );

    if (imported_exists(&record))
        return;

    imported_records[
        imported_record_count
    ] = record;

    imported_load_metadata(
        &imported_records[
            imported_record_count
        ]
    );

    imported_record_count++;
}

static int imported_id_exists(const char *game_id)
{
    int i;

    if (game_id == NULL)
        return 0;

    for (i = 0; i < imported_record_count; ++i)
    {
        if (strcmp(imported_records[i].game_id, game_id) == 0)
            return 1;
    }

    return 0;
}

static int imported_persistent_id_exists(const char *game_id)
{
    DIR *dir;
    struct dirent *entry;

    if (game_id == NULL || game_id[0] == '\0')
        return 0;

    dir = opendir("sd:/marcviiew/imported");

    if (dir == NULL)
        return 0;

    while ((entry = readdir(dir)) != NULL)
    {
        char existing_id[IMPORTED_RECORD_ID_LENGTH];
        const char *extension = strrchr(entry->d_name, '.');

        if (entry->d_name[0] == '.' ||
            extension == NULL ||
            strcmp(extension, ".mrc") != 0)
            continue;

        if (imported_get_id(entry->d_name, existing_id) &&
            strcmp(existing_id, game_id) == 0)
        {
            closedir(dir);
            return 1;
        }
    }

    closedir(dir);
    return 0;
}

static void imported_scan_directory(const char *directory)
{
    DIR *dir;
    struct dirent *entry;
    int incoming = (strcmp(directory, "sd:/marcviiew_import") == 0);

    dir = opendir(directory);

    if (dir == NULL)
        return;

    while ((entry = readdir(dir)) != NULL)
    {
        const char *extension = strrchr(entry->d_name, '.');

        if (entry->d_name[0] == '.' ||
            extension == NULL ||
            strcmp(extension, ".mrc") != 0)
            continue;

        if (incoming)
        {
            char game_id[IMPORTED_RECORD_ID_LENGTH];
            char incoming_path[512];
            char imported_path[512];

            /*
             * Imported records use the marcviiew_GAMEID.mrc naming
             * convention. New incoming records are moved into the
             * persistent collection as soon as they are discovered.
             */
            if (!imported_get_id(entry->d_name, game_id))
                continue;

            snprintf(
                incoming_path,
                sizeof(incoming_path),
                "sd:/marcviiew_import/%s",
                entry->d_name
            );

            snprintf(
                imported_path,
                sizeof(imported_path),
                "sd:/marcviiew/imported/%s",
                entry->d_name
            );

            /*
             * A record already present in the persistent collection
             * is a duplicate. Remove only the incoming duplicate.
             */
            if (imported_id_exists(game_id) ||
                imported_persistent_id_exists(game_id) ||
                access(imported_path, F_OK) == 0)
            {
                /*
                 * Incoming records are temporary. Once a matching
                 * persistent record exists, remove the incoming copy.
                 *
                 * Use remove() here rather than relying on the POSIX
                 * unlink() wrapper, since MarcViiew runs through libfat
                 * on Wii SD/USB filesystems.
                 */
                remove(incoming_path);
                continue;
            }

            /*
             * Move the new record into persistent storage. If the
             * move fails, still index the incoming file so it remains
             * visible and can be opened.
             */
            if (rename(incoming_path, imported_path) != 0)
            {
                /*
                 * If the move fails because a persistent copy appeared
                 * between the checks above, remove the incoming copy.
                 * Otherwise keep it visible for another import attempt.
                 */
                if (access(imported_path, F_OK) == 0)
                    remove(incoming_path);
                else
                    imported_add_filename(entry->d_name);

                continue;
            }

            if (imported_record_count < MAX_IMPORTED_RECORDS)
                imported_add_filename(entry->d_name);

            continue;
        }

        if (imported_record_count >= MAX_IMPORTED_RECORDS)
            continue;

        imported_add_filename(entry->d_name);
    }

    closedir(dir);
}

static void scan_imported_records(void)
{
    imported_record_count = 0;
    imported_selection = 0;
    imported_scroll = 0;

    mkdir("sd:/marcviiew_import", 0777);
    mkdir("sd:/marcviiew", 0777);
    mkdir("sd:/marcviiew/imported", 0777);

    /*
     * Do not parse ISO 2709 during startup. The incoming directory
     * is intentionally just indexed here; parsing happens when a
     * record is opened.
     */
    imported_scan_directory("sd:/marcviiew/imported");
    imported_scan_directory("sd:/marcviiew_import");
}



void extract_game_id(
    const char *name,
    char *id
) {

    const char *start =
        strchr(name, '[');

    if (
        start != NULL &&
        strlen(start) >= 8
    ) {

        memcpy(id, start + 1, 6);
        id[6] = '\0';

    } else {

        strcpy(
            id,
            "??????"
        );
    }
}


void extract_game_title(
    const char *name,
    char *title
) {

    const char *start =
        strchr(name, '[');

    if (
        start != NULL
    ) {

        int length =
            start - name;

        if (
            length >= 100
        )
            length = 99;

        strncpy(
            title,
            name,
            length
        );

        title[length] = '\0';

        while (
            length > 0 &&
            title[length - 1] == ' '
        ) {

            title[length - 1] =
                '\0';

            length--;
        }

    } else {

        strncpy(
            title,
            name,
            99
        );

        title[99] = '\0';
    }
}


void setup_game_metadata(
    Game *game
) {

    strcpy(
        game->platform,
        "Nintendo Wii"
    );

    strcpy(
        game->region,
        "Unknown"
    );

    strcpy(
        game->release_date,
        "Not yet catalogued"
    );

    strcpy(
        game->publisher,
        "Not yet catalogued"
    );

    strcpy(
        game->developer,
        "Not yet catalogued"
    );

    strcpy(
        game->genre,
        "Not yet catalogued"
    );

    strcpy(
        game->series,
        "Not yet catalogued"
    );

    strcpy(
        game->source,
        "Unknown"
    );

    strcpy(
        game->distribution,
        "Unknown"
    );

    strcpy(
        game->synopsis,
        "Not yet catalogued"
    );
}


void copy_database_value(
    const char *line,
    const char *field,
    char *destination,
    int destination_size
) {

    size_t field_length;

    if (line == NULL ||
        field == NULL ||
        destination == NULL ||
        destination_size <= 0)
        return;

    field_length = strlen(field);

    if (
        strncmp(
            line,
            field,
            field_length
        ) == 0
    ) {

        int value_length =
            strlen(line + field_length);

        if (
            value_length >=
            destination_size
        )
            value_length =
                destination_size - 1;

        memcpy(
            destination,
            line + field_length,
            value_length
        );

        destination[value_length] =
            '\0';
    }
}


int find_game_by_id(
    const char *id
) {

    for (
        int i = 0;
        i < game_count;
        i++
    ) {

        if (
            strcmp(
                games[i].id,
                id
            ) == 0
        )
            return i;
    }

    return -1;
}


int load_game_database() {

    FILE *database;

    char line[
        DATABASE_LINE_LENGTH
    ];

    int current_game_index = -1;

    database =
        fopen(
            "sd:/marcviiew_games.txt",
            "r"
        );

    if (
        database == NULL
    )
        return 0;

    while (
        fgets(
            line,
            sizeof(line),
            database
        ) != NULL
    ) {

        line[
            strcspn(
                line,
                "\r\n"
            )
        ] = '\0';


        if (
            strncmp(
                line,
                "ID=",
                3
            ) == 0
        ) {

            current_game_index =
                find_game_by_id(
                    line + 3
                );

            continue;
        }


        if (
            current_game_index < 0
        )
            continue;


        if (
            strlen(line) == 0
        ) {

            current_game_index = -1;

            continue;
        }


        copy_database_value(
            line,
            "TITLE=",
            games[
                current_game_index
            ].title,
            sizeof(
                games[
                    current_game_index
                ].title
            )
        );

        copy_database_value(
            line,
            "REGION=",
            games[
                current_game_index
            ].region,
            sizeof(
                games[
                    current_game_index
                ].region
            )
        );

        copy_database_value(
            line,
            "DEVELOPER=",
            games[
                current_game_index
            ].developer,
            sizeof(
                games[
                    current_game_index
                ].developer
            )
        );

        copy_database_value(
            line,
            "PUBLISHER=",
            games[
                current_game_index
            ].publisher,
            sizeof(
                games[
                    current_game_index
                ].publisher
            )
        );

        copy_database_value(
            line,
            "RELEASE_DATE=",
            games[
                current_game_index
            ].release_date,
            sizeof(
                games[
                    current_game_index
                ].release_date
            )
        );

        copy_database_value(
            line,
            "GENRE=",
            games[
                current_game_index
            ].genre,
            sizeof(
                games[
                    current_game_index
                ].genre
            )
        );

        copy_database_value(
            line,
            "DISTRIBUTION=",
            games[
                current_game_index
            ].distribution,
            sizeof(
                games[
                    current_game_index
                ].distribution
            )
        );

        copy_database_value(
            line,
            "SYNOPSIS=",
            games[
                current_game_index
            ].synopsis,
            sizeof(
                games[
                    current_game_index
                ].synopsis
            )
        );
    }

    fclose(
        database
    );

    return 1;
}


static void add_game_source(Game *game, const char *source)
{
    size_t current_length;
    size_t source_length;

    if (game == NULL || source == NULL || source[0] == '\0')
        return;

    if (strcmp(game->source, source) == 0)
        return;

    if (strcmp(game->source, "Unknown") == 0)
    {
        snprintf(
            game->source,
            sizeof(game->source),
            "%s",
            source
        );
        return;
    }

    current_length = strlen(game->source);
    source_length = strlen(source);

    /*
     * Sources are stored as a small human-readable list. Check each
     * complete source token rather than using an unbounded substring
     * test, so one source name cannot accidentally match part of
     * another.
     */
    {
        const char *cursor = game->source;

        while (*cursor != '\0')
        {
            const char *end = strstr(cursor, " + ");
            size_t length = end != NULL
                ? (size_t)(end - cursor)
                : strlen(cursor);

            if (length == source_length &&
                strncmp(cursor, source, length) == 0)
                return;

            if (end == NULL)
                break;

            cursor = end + 3;
        }
    }

    if (current_length + source_length + 3 < sizeof(game->source))
    {
        snprintf(
            game->source + current_length,
            sizeof(game->source) - current_length,
            " + %s",
            source
        );
    }
    else
    {
        snprintf(
            game->source,
            sizeof(game->source),
            "Multiple"
        );
    }
}


void scan_storage(
    const char *storage_path
) {

    DIR *dir;

    struct dirent *entry;

    dir =
        opendir(
            storage_path
        );

    if (
        dir == NULL
    )
        return;


    while (
        (entry = readdir(dir))
        != NULL
    ) {

        if (
            entry->d_name[0] == '.'
        )
            continue;

        if (
            game_count >=
            MAX_GAMES
        )
            break;


        char discovered_id[7];
        const char *source = NULL;

        extract_game_id(
            entry->d_name,
            discovered_id
        );

        if (discovered_id[0] == '\0')
            continue;

        if (strcmp(storage_path, "sd:/wbfs") == 0)
            source = "SD";
        else if (strcmp(storage_path, "usb:/wbfs") == 0)
            source = "USB";

        {
            int existing = find_game_by_id(discovered_id);

            if (existing >= 0)
            {
                add_game_source(&games[existing], source);
                continue;
            }
        }

        extract_game_title(
            entry->d_name,
            games[game_count].title
        );

        memcpy(games[game_count].id, discovered_id, 6);
        games[game_count].id[6] = '\0';

        setup_game_metadata(&games[game_count]);
        add_game_source(&games[game_count], source);

        game_count++;
    }

    closedir(
        dir
    );
}


static void scan_digital_titles(const char *root_path, const char *source)
{
    DIR *dir;
    struct dirent *entry;

    dir = opendir(root_path);

    if (dir == NULL)
        return;

    while ((entry = readdir(dir)) != NULL)
    {
        char title_directory[512];
        char content_path[512];
        char code[7];
        size_t length;
        struct stat title_status;
        FILE *content;

        if (entry->d_name[0] == '.')
            continue;

        length = strlen(entry->d_name);

        if (length != 4 || game_count >= MAX_GAMES)
            continue;

        snprintf(
            title_directory,
            sizeof(title_directory),
            "%s/%s",
            root_path,
            entry->d_name
        );

        if (stat(title_directory, &title_status) != 0 ||
            !S_ISDIR(title_status.st_mode))
            continue;

        snprintf(
            content_path,
            sizeof(content_path),
            "%s/content.bin",
            title_directory
        );

        content = fopen(content_path, "rb");

        if (content == NULL)
            continue;

        fclose(content);

        /*
            Nintendo's SD Card Menu stores transferred
            WiiWare, Virtual Console and channel titles
            below private/wii/title/<four-letter ID>/.
            The directory name is the ASCII title code.
        */
        code[0] = entry->d_name[0];
        code[1] = entry->d_name[1];
        code[2] = entry->d_name[2];
        code[3] = entry->d_name[3];
        code[4] = '\0';
        code[5] = '\0';
        code[6] = '\0';

        {
            int i;

            for (i = 0; i < 4; ++i)
                code[i] = (char)toupper((unsigned char)code[i]);
        }

        {
            int existing = find_game_by_id(code);

            if (existing >= 0)
            {
                add_game_source(&games[existing], source);
                continue;
            }
        }

        memset(&games[game_count], 0, sizeof(Game));

        memcpy(games[game_count].id, code, 6);
        games[game_count].id[6] = '\0';

        strncpy(
            games[game_count].title,
            code,
            sizeof(games[game_count].title) - 1
        );

        setup_game_metadata(&games[game_count]);

        add_game_source(
            &games[game_count],
            source
        );

        game_count++;
    }

    closedir(dir);
}


static void scan_nand_catalogue(void)
{

    u32 title_count = 0;
    u64 *titles = NULL;
    s32 result;
    u32 i;

    nand_title_count = 0;
    nand_game_count = 0;

    result = __ES_Init();

    if (result < 0)
        return;

    result = ES_GetNumTitles(&title_count);

    if (result < 0 || title_count == 0)
    {
        __ES_Close();
        return;
    }

    /*
     * ES_GetTitles() returns exactly title_count entries; no sentinel
     * element is required.
     */
    titles = malloc(
        (size_t)title_count * sizeof(*titles)
    );

    if (titles == NULL)
    {
        __ES_Close();
        return;
    }

    result = ES_GetTitles(
        titles,
        title_count
    );

    if (result < 0)
    {
        free(titles);
        __ES_Close();
        return;
    }

    for (
        i = 0;
        i < title_count;
        i++
    ) {

        u64 title_id = titles[i];
        u32 title_type =
            (u32)(title_id >> 32);
        u32 title_code =
            (u32)title_id;
        char code[7];

        /*
            00010001 contains downloadable Wii
            titles, including WiiWare and Virtual
            Console software.
        */
        if (title_type != 0x00010001)
            continue;

        code[0] = (char)((title_code >> 24) & 0xFF);
        code[1] = (char)((title_code >> 16) & 0xFF);
        code[2] = (char)((title_code >> 8) & 0xFF);
        code[3] = (char)(title_code & 0xFF);
        code[4] = '\0';
        code[5] = '\0';
        code[6] = '\0';

        nand_title_count++;

        /*
            Only add titles which are represented in
            the existing MarcViiew game database.
            The database is loaded after this scan.
        */
        {
            int existing = find_game_by_id(code);

            if (existing >= 0)
            {
                add_game_source(&games[existing], "NAND");
                continue;
            }
        }

        if (game_count >= MAX_GAMES)
            continue;

        memset(
            &games[game_count],
            0,
            sizeof(Game)
        );

        memcpy(games[game_count].id, code, 6);
        games[game_count].id[6] = '\0';

        strncpy(
            games[game_count].title,
            code,
            sizeof(games[game_count].title) - 1
        );

        setup_game_metadata(
            &games[game_count]
        );

        strcpy(
            games[game_count].source,
            "NAND"
        );

        game_count++;
        nand_game_count++;
    }

    free(titles);
    __ES_Close();
}


/*
    NAND scanning discovers installed title IDs. The normal game
    database is loaded afterwards so newly discovered NAND records
    can be filled with their derived MarcViiew metadata.
*/

void scan_catalogue() {

    game_count = 0;

    scan_storage(
        "sd:/wbfs"
    );

    scan_digital_titles(
        "sd:/private/wii/title",
        "SD"
    );

    scan_digital_titles(
        "usb:/private/wii/title",
        "USB"
    );

    if (
        game_count <
        MAX_GAMES
    ) {

        scan_storage(
            "usb:/wbfs"
        );
    }

    scan_nand_catalogue();

    load_game_database();
}


int reload_storage() {

    int result;

    fatUnmount(
        "sd"
    );

    fatUnmount(
        "usb"
    );

    result =
        fatInitDefault();

    return result;
}


int reload_databases() {

    FILE *game_database;
    FILE *marc_database;

    int game_database_loaded = 0;
    int marc_database_loaded = 0;

    search_result_count = 0;
    search_selection = 0;

    search_length = 0;
    search_query[0] = '\0';

    marc_record_from_search = 0;

    catalogue_selection = 0;
    catalogue_scroll = 0;
    marc_selection = 0;
    marc_menu_scroll = 0;

    info_scroll = 0;
    marc_scroll = 0;

    encode_selection = 0;
    encode_game_selection = 0;
    encode_status[0] = '\0';

    scan_catalogue();
    scan_imported_records();

    /*
        The normal MarcViiew LMS uses the text databases.
        The .mrc files are exports only.
    */
    game_database =
        fopen(
            "sd:/marcviiew_games.txt",
            "r"
        );

    if (
        game_database != NULL
    ) {

        game_database_loaded = 1;

        fclose(
            game_database
        );
    }


    marc_database =
        fopen(
            "sd:/marcviiew_marc.txt",
            "r"
        );

    if (
        marc_database != NULL
    ) {

        marc_database_loaded = 1;

        fclose(
            marc_database
        );
    }


    if (
        game_database_loaded &&
        marc_database_loaded
    ) {

        snprintf(
            settings_status,
            sizeof(settings_status),
            "Both databases reloaded. %d game(s) found.",
            game_count
        );

    }

    else if (
        game_database_loaded
    ) {

        snprintf(
            settings_status,
            sizeof(settings_status),
            "Game database OK; MARC database not found."
        );

    }

    else if (
        marc_database_loaded
    ) {

        snprintf(
            settings_status,
            sizeof(settings_status),
            "MARC database OK; game database not found."
        );

    }

    else {

        snprintf(
            settings_status,
            sizeof(settings_status),
            "Neither database was found."
        );
    }

    return (
        game_database_loaded &&
        marc_database_loaded
    );
}


void add_info_line(
    char info_lines[][INFO_LINE_LENGTH],
    int *line_count,
    const char *text
) {

    if (
        *line_count >=
        MAX_INFO_LINES
    )
        return;

    snprintf(
        info_lines[*line_count],
        INFO_LINE_LENGTH,
        "%s",
        text
    );

    (*line_count)++;
}


void add_info_field(
    char info_lines[][INFO_LINE_LENGTH],
    int *line_count,
    const char *label,
    const char *value
) {

    char line[
        INFO_LINE_LENGTH
    ];

    snprintf(
        line,
        sizeof(line),
        "%s:",
        label
    );

    add_info_line(
        info_lines,
        line_count,
        line
    );

    add_info_line(
        info_lines,
        line_count,
        value
    );

    add_info_line(
        info_lines,
        line_count,
        ""
    );
}


void add_wrapped_text(
    char info_lines[][INFO_LINE_LENGTH],
    int *line_count,
    const char *text
) {

    const int wrap_width = 70;

    int length =
        strlen(text);

    int position = 0;

    while (
        position < length &&
        *line_count <
        MAX_INFO_LINES
    ) {

        int remaining =
            length - position;

        int line_length =
            remaining;

        if (
            line_length >
            wrap_width
        )
            line_length =
                wrap_width;

        if (
            position + line_length <
            length
        ) {

            int break_position =
                line_length;

            while (
                break_position > 0 &&
                text[
                    position +
                    break_position
                ] != ' '
            )
                break_position--;

            if (
                break_position > 0
            )
                line_length =
                    break_position;
        }

        strncpy(
            info_lines[
                *line_count
            ],
            text + position,
            line_length
        );

        info_lines[
            *line_count
        ][
            line_length
        ] = '\0';

        (*line_count)++;

        position += line_length;

        while (
            position < length &&
            text[position] == ' '
        )
            position++;
    }
}


void show_game_information() {

    char info_lines[
        MAX_INFO_LINES
    ][
        INFO_LINE_LENGTH
    ];

    int line_count = 0;

    Game *game =
        &games[
            catalogue_selection
        ];


    add_info_field(
        info_lines,
        &line_count,
        "Title",
        game->title
    );

    add_info_field(
        info_lines,
        &line_count,
        "Game ID",
        game->id
    );

    add_info_field(
        info_lines,
        &line_count,
        "Platform",
        game->platform
    );

    add_info_field(
        info_lines,
        &line_count,
        "Region",
        game->region
    );

    add_info_field(
        info_lines,
        &line_count,
        "Release Date",
        game->release_date
    );

    add_info_field(
        info_lines,
        &line_count,
        "Publisher",
        game->publisher
    );

    add_info_field(
        info_lines,
        &line_count,
        "Developer",
        game->developer
    );

    add_info_field(
        info_lines,
        &line_count,
        "Genre",
        game->genre
    );

    add_info_field(
        info_lines,
        &line_count,
        "Series",
        game->series
    );

    add_info_field(
        info_lines,
        &line_count,
        "Source",
        game->source
    );

    add_info_field(
        info_lines,
        &line_count,
        "Distribution",
        game->distribution
    );

    add_info_line(
        info_lines,
        &line_count,
        "Synopsis:"
    );

    add_wrapped_text(
        info_lines,
        &line_count,
        game->synopsis
    );

    add_info_line(
        info_lines,
        &line_count,
        ""
    );


    printf(
        "\x1b[2J\x1b[H"
    );

    print_ui_line('=');

    print_centered(
        "Game Information"
    );

    print_ui_line('=');

    printf("\n");


    int visible_lines = 17;

    int max_scroll =
        line_count -
        visible_lines;

    if (
        max_scroll < 0
    )
        max_scroll = 0;

    if (
        info_scroll >
        max_scroll
    )
        info_scroll =
            max_scroll;

    if (
        info_scroll < 0
    )
        info_scroll = 0;


    for (
        int i = info_scroll;
        i <
            info_scroll +
            visible_lines &&
        i < line_count;
        i++
    ) {

        print_centered(
            info_lines[i]
        );
    }


    printf("\n");


    if (
        max_scroll > 0
    ) {

        char scroll_line[80];

        snprintf(
            scroll_line,
            sizeof(scroll_line),
            "UP / DOWN = Scroll  (%d/%d)",
            info_scroll + 1,
            max_scroll + 1
        );

        print_centered(
            scroll_line
        );

    } else {

        print_centered(
            "UP / DOWN = Scroll"
        );
    }


    print_centered(
        "B = Back"
    );
}


void add_marc_line(
    const char *text
) {

    if (
        marc_line_count >=
        MAX_MARC_LINES
    )
        return;

    snprintf(
        marc_lines[marc_line_count],
        MARC_LINE_LENGTH,
        "%s",
        text
    );

    marc_line_count++;
}


void add_marc_wrapped_subfield(
    const char *prefix,
    const char *text
) {

    const int wrap_width = 70;

    int text_length =
        strlen(text);

    int position = 0;

    int first_line = 1;


    while (
        position < text_length &&
        marc_line_count < MAX_MARC_LINES
    ) {

        int remaining =
            text_length - position;

        int available_width =
            wrap_width;

        if (
            first_line
        )
            available_width -= strlen(prefix);


        if (
            available_width < 1
        )
            available_width = 1;


        int line_length =
            remaining;

        if (
            line_length >
            available_width
        )
            line_length =
                available_width;


        if (
            position + line_length <
            text_length
        ) {

            int break_position =
                line_length;

            while (
                break_position > 0 &&
                text[
                    position +
                    break_position
                ] != ' '
            )
                break_position--;

            if (
                break_position > 0
            )
                line_length =
                    break_position;
        }


        char output_line[
            MARC_LINE_LENGTH
        ];


        if (
            first_line
        ) {

            snprintf(
                output_line,
                sizeof(output_line),
                "%s%.*s",
                prefix,
                line_length,
                text + position
            );

            first_line = 0;

        } else {

            snprintf(
                output_line,
                sizeof(output_line),
                "    %.*s",
                line_length,
                text + position
            );
        }


        add_marc_line(
            output_line
        );


        position += line_length;


        while (
            position < text_length &&
            text[position] == ' '
        )
            position++;
    }


    if (
        text_length == 0 &&
        marc_line_count < MAX_MARC_LINES
    ) {

        add_marc_line(
            prefix
        );
    }
}
static void render_marc_record(MARC_Record *record)
{
    size_t i;

    marc_line_count = 0;

    if (record == NULL)
        return;

    {
        const char *leader = marc_record_get_leader(record);

        if (leader != NULL)
        {
            char line[MARC_LINE_LENGTH];

            snprintf(line, sizeof(line), "LDR %s", leader);
            add_marc_line(line);
        }
    }

    for (i = 0; i < marc_record_get_field_count(record); ++i)
    {
        MARC_Field *field = marc_record_get_field(record, i);
        const char *tag;

        if (field == NULL)
            continue;

        tag = marc_field_get_tag(field);

        if (tag == NULL)
            continue;

        if (marc_field_is_control_field(field))
        {
            const char *value =
                marc_field_get_control_value(field);
            char line[MARC_LINE_LENGTH];

            if (value == NULL)
                value = "";

            snprintf(
                line,
                sizeof(line),
                "%s %s",
                tag,
                value
            );

            add_marc_line(line);
        }
        else
        {
            char header[MARC_LINE_LENGTH];
            size_t j;
            size_t subfield_count =
                marc_field_get_subfield_count(field);

            snprintf(
                header,
                sizeof(header),
                "%s %c%c",
                tag,
                marc_field_get_indicator1(field),
                marc_field_get_indicator2(field)
            );

            add_marc_line(header);

            for (j = 0; j < subfield_count; ++j)
            {
                MARC_Subfield *subfield =
                    marc_field_get_subfield(field, j);
                const char *value;
                char prefix[16];

                if (subfield == NULL)
                    continue;

                value = marc_subfield_get_value(subfield);

                if (value == NULL)
                    value = "";

                snprintf(
                    prefix,
                    sizeof(prefix),
                    "  $%c ",
                    marc_subfield_get_code(subfield)
                );

                add_marc_wrapped_subfield(
                    prefix,
                    value
                );
            }
        }
    }
}


static MARC_Record *read_mrc_record(FILE *file)
{
    MARC_Record *record;

    if (file == NULL)
        return NULL;

    record = marc_record_create();

    if (record == NULL)
        return NULL;

    if (marc_record_read(record, file) != MARC_SUCCESS)
    {
        marc_record_free(record);
        return NULL;
    }

    return record;
}


static void imported_update_metadata_from_record(
    ImportedRecord *imported,
    MARC_Record *record
)
{
    MARC_Field *field;

    if (imported == NULL || record == NULL)
        return;

    field = marc_record_get_field_by_tag(record, "245");

    if (field != NULL)
    {
        imported_subfield(
            field,
            'a',
            imported->title,
            sizeof(imported->title)
        );
    }

    field = marc_record_get_field_by_tag(record, "001");

    if (field != NULL)
    {
        const char *value =
            marc_field_get_control_value(field);

        if (value != NULL && value[0] != '\0')
        {
            snprintf(
                imported->marc_001,
                sizeof(imported->marc_001),
                "%s",
                value
            );
        }
    }
}


static int imported_load_metadata(
    ImportedRecord *imported
)
{
    char path[256];
    FILE *file;
    MARC_Record *record;

    if (imported == NULL)
        return -1;

    snprintf(
        path,
        sizeof(path),
        "sd:/marcviiew/imported/%s",
        imported->filename
    );

    file = fopen(path, "rb");

    if (file == NULL)
    {
        snprintf(
            path,
            sizeof(path),
            "sd:/marcviiew_import/%s",
            imported->filename
        );

        file = fopen(path, "rb");
    }

    if (file == NULL)
        return -2;

    record = read_mrc_record(file);

    fclose(file);

    if (record == NULL)
    {
        strcpy(
            imported->marc_001,
            "(read error)"
        );

        return -1;
    }

    imported_update_metadata_from_record(
        imported,
        record
    );

    marc_record_free(record);

    return 0;
}



/*
    Load a MARC record from the normal MarcViiew text
    database.

    IMPORTANT:

    This is intentionally NOT the ISO 2709 encoder.

    The LMS continues to use:

        sd:/marcviiew_marc.txt

    The .mrc files generated by ViiewLib are exports
    only.

    The selected Wii game's ID is matched against:

        [RECORD]
        GAME_ID=xxxxxx

    and the complete corresponding MARC record is
    rendered on the MARC viewer screen.
*/int load_marc_record()
{
    FILE *marc_file;
    MarcViiewParser parser;
    MARC_Record *record = NULL;
    int result;

    if (loaded_marc_record != NULL)
    {
        marc_record_free(
            loaded_marc_record
        );

        loaded_marc_record = NULL;
    }

    if (game_count == 0)
        return 0;

    if (marc_selection < 0 ||
        marc_selection >= game_count)
        return 0;

    marc_file = fopen(
        "sd:/marcviiew_marc.txt",
        "rb"
    );

    if (marc_file == NULL)
        return -1;

    if (marcviiew_parser_init(
            &parser,
            marc_file
        ) != 0)
    {
        fclose(marc_file);
        return -1;
    }

    while (1)
    {
        const char *record_id;

        record = NULL;

        result =
            marcviiew_parser_next(
                &parser,
                &record
            );

        if (result == 0)
            break;

        if (result < 0)
        {
            fclose(marc_file);
            return -1;
        }

        if (record == NULL)
        {
            fclose(marc_file);
            return -1;
        }

        record_id =
            marc_record_get_control_field(
                record,
                "001"
            );

        if (record_id != NULL &&
            strcmp(
                record_id,
                games[marc_selection].id
            ) == 0)
        {
            loaded_marc_record = record;
            fclose(marc_file);

            render_marc_record(
                loaded_marc_record
            );

            return 1;
        }

        marc_record_free(record);
    }

    fclose(marc_file);

    return 0;
}


int contains_ignore_case(
    const char *haystack,
    const char *needle
) {

    int haystack_length =
        strlen(haystack);

    int needle_length =
        strlen(needle);

    if (
        needle_length == 0
    )
        return 1;

    if (
        needle_length >
        haystack_length
    )
        return 0;


    for (
        int i = 0;
        i <=
            haystack_length -
            needle_length;
        i++
    ) {

        int match = 1;


        for (
            int j = 0;
            j < needle_length;
            j++
        ) {

            char a =
                tolower(
                    (unsigned char)
                    haystack[i + j]
                );

            char b =
                tolower(
                    (unsigned char)
                    needle[j]
                );


            if (
                a != b
            ) {

                match = 0;

                break;
            }
        }


        if (
            match
        )
            return 1;
    }


    return 0;
}


void perform_marc_search() {

    FILE *marc_file;

    char line[
        DATABASE_LINE_LENGTH
    ];

    char current_game_id[7] = "";

    int in_record = 0;

    int record_matches = 0;


    search_result_count = 0;


    marc_file =
        fopen(
            "sd:/marcviiew_marc.txt",
            "r"
        );


    if (
        marc_file == NULL
    ) {

        search_selection = 0;

        return;
    }


    while (
        fgets(
            line,
            sizeof(line),
            marc_file
        ) != NULL
    ) {

        line[
            strcspn(
                line,
                "\r\n"
            )
        ] = '\0';


        if (
            strncmp(
                line,
                "GAME_ID=",
                8
            ) == 0
        ) {

            if (
                in_record &&
                record_matches
            ) {

                int game_index =
                    find_game_by_id(
                        current_game_id
                    );


                if (
                    game_index >= 0 &&
                    search_result_count <
                    MAX_SEARCH_RESULTS
                ) {

                    search_results[
                        search_result_count
                    ] = game_index;

                    search_result_count++;
                }
            }


            snprintf(
                current_game_id,
                sizeof(current_game_id),
                "%s",
                line + 8
            );


            in_record = 1;


            record_matches =
                (
                    search_length == 0 ||
                    search_query[0] == '\0'
                );


            if (
                !record_matches &&
                contains_ignore_case(
                    current_game_id,
                    search_query
                )
            ) {

                record_matches = 1;
            }


            continue;
        }


        if (
            !in_record
        )
            continue;


        if (
            !record_matches &&
            contains_ignore_case(
                line,
                search_query
            )
        ) {

            record_matches = 1;
        }
    }


    if (
        in_record &&
        record_matches
    ) {

        int game_index =
            find_game_by_id(
                current_game_id
            );


        if (
            game_index >= 0 &&
            search_result_count <
            MAX_SEARCH_RESULTS
        ) {

            search_results[
                search_result_count
            ] = game_index;

            search_result_count++;
        }
    }


    fclose(
        marc_file
    );


    search_selection = 0;
}


void perform_search() {

    search_result_count = 0;


    if (
        search_mode ==
        SEARCH_MODE_MARC21
    ) {

        perform_marc_search();

        return;
    }


    for (
        int i = 0;
        i < game_count;
        i++
    ) {

        int match =
            contains_ignore_case(
                games[i].title,
                search_query
            );


        if (
            match &&
            search_result_count <
            MAX_SEARCH_RESULTS
        ) {

            search_results[
                search_result_count
            ] = i;

            search_result_count++;
        }
    }


    search_selection = 0;
}


void show_marc_menu() {

    printf(
        "\x1b[2J\x1b[H"
    );

    print_ui_line('=');

    print_centered(
        "MARC Records"
    );

    print_ui_line('=');

    printf("\n");


    if (
        game_count == 0
    ) {

        print_centered(
            "No games found."
        );

        printf("\n");

        print_centered(
            "B = Back"
        );

        return;
    }


    if (
        marc_selection < 0
    )
        marc_selection = 0;

    if (
        marc_selection >= game_count
    )
        marc_selection = game_count - 1;

    if (
        marc_menu_scroll < 0
    )
        marc_menu_scroll = 0;

    if (
        marc_selection < marc_menu_scroll
    )
        marc_menu_scroll = marc_selection;

    if (
        marc_selection >=
        marc_menu_scroll +
        GAME_LIST_VISIBLE_ITEMS
    )
        marc_menu_scroll =
            marc_selection -
            GAME_LIST_VISIBLE_ITEMS +
            1;

    if (
        marc_menu_scroll >
        game_count -
        GAME_LIST_VISIBLE_ITEMS
    ) {

        marc_menu_scroll =
            game_count -
            GAME_LIST_VISIBLE_ITEMS;

        if (
            marc_menu_scroll < 0
        )
            marc_menu_scroll = 0;
    }


    int visible_end =
        marc_menu_scroll +
        GAME_LIST_VISIBLE_ITEMS;

    if (
        visible_end > game_count
    )
        visible_end = game_count;


    for (
        int i = marc_menu_scroll;
        i < visible_end;
        i++
    ) {

        print_menu_item(
            games[i].title,
            i == marc_selection
        );
    }


    printf("\n");


    {
        char line[80];

        snprintf(
            line,
            sizeof(line),
            "%d game(s) available.",
            game_count
        );

        print_centered(
            line
        );
    }


    printf("\n");

    print_centered(
        "UP / DOWN = Move"
    );

    print_centered(
        "A = View MARC Record"
    );

    print_centered(
        "B = Back"
    );
}

void show_marc_record()
{
    int result =
        load_marc_record();

    printf(
        "\x1b[2J\x1b[H"
    );

    print_ui_line('=');
    print_centered("MARC Record");
    print_ui_line('=');
    printf("\n");

    if (result == -1)
    {
        print_centered(
            "MARC database not found"
        );

        print_centered(
            "or could not be parsed."
        );

        printf("\n");
        print_centered("Expected:");
        print_centered("sd:/marcviiew_marc.txt");
        printf("\n");
        print_centered("B = Back");
        return;
    }

    if (result == 0)
    {
        print_centered(
            "MARC record not found."
        );

        printf("\n");

        {
            char line[120];

            snprintf(
                line,
                sizeof(line),
                "Game: %s",
                games[marc_selection].title
            );

            print_centered(line);

            snprintf(
                line,
                sizeof(line),
                "ID: %s",
                games[marc_selection].id
            );

            print_centered(line);
        }

        printf("\n");
        print_centered("B = Back");
        return;
    }

    {
        char line[160];

        snprintf(
            line,
            sizeof(line),
            "Game: %s",
            games[marc_selection].title
        );

        print_centered(line);

        snprintf(
            line,
            sizeof(line),
            "ID: %s",
            games[marc_selection].id
        );

        print_centered(line);
    }

    printf("\n");

    {
        int visible_lines = 15;
        int max_scroll =
            marc_line_count - visible_lines;
        int i;

        if (max_scroll < 0)
            max_scroll = 0;

        if (marc_scroll > max_scroll)
            marc_scroll = max_scroll;

        if (marc_scroll < 0)
            marc_scroll = 0;

        for (
            i = marc_scroll;
            i < marc_scroll + visible_lines &&
            i < marc_line_count;
            ++i
        )
        {
            print_centered(
                marc_lines[i]
            );
        }

        printf("\n");

        if (max_scroll > 0)
        {
            char scroll_line[80];

            snprintf(
                scroll_line,
                sizeof(scroll_line),
                "UP / DOWN = Scroll  (%d/%d)",
                marc_scroll + 1,
                max_scroll + 1
            );

            print_centered(scroll_line);
        }
        else
        {
            print_centered(
                "UP / DOWN = Scroll"
            );
        }

        print_centered("B = Back");
    }
}


/*
    Encode the entire MarcViiew MARC text database.

    This is an EXPORT operation.

    The normal LMS continues using marcviiew_marc.txt.
*/

void show_imported_menu(void)
{
    int i;
    int visible_end;
    char line[80];

    printf("\x1b[2J\x1b[H");
    print_ui_line('=');
    print_centered("Imported Records");
    print_ui_line('=');
    printf("\n");

    if (imported_record_count == 0)
    {
        print_centered("No imported records found.");
        printf("\n");
        print_centered("Place .mrc files in:");
        print_centered("sd:/marcviiew_import");
        printf("\n");
        print_centered("B = Back");
        return;
    }

    /* Keep the selection within the imported-record list. */
    if (imported_selection < 0)
        imported_selection = 0;

    if (imported_selection >= imported_record_count)
        imported_selection = imported_record_count - 1;

    /* Keep the selected record inside the visible 15-item window. */
    if (imported_scroll < 0)
        imported_scroll = 0;

    if (imported_selection < imported_scroll)
        imported_scroll = imported_selection;

    if (imported_selection >=
        imported_scroll + GAME_LIST_VISIBLE_ITEMS)
        imported_scroll =
            imported_selection -
            GAME_LIST_VISIBLE_ITEMS + 1;

    if (imported_scroll >
        imported_record_count - GAME_LIST_VISIBLE_ITEMS)
    {
        imported_scroll =
            imported_record_count - GAME_LIST_VISIBLE_ITEMS;

        if (imported_scroll < 0)
            imported_scroll = 0;
    }

    visible_end =
        imported_scroll + GAME_LIST_VISIBLE_ITEMS;

    if (visible_end > imported_record_count)
        visible_end = imported_record_count;

    for (i = imported_scroll; i < visible_end; ++i)
        print_menu_item(imported_records[i].title, i == imported_selection);

    printf("\n");

    snprintf(line, sizeof(line), "%d imported record(s).", imported_record_count);
    print_centered(line);

    printf("\n");
    print_centered("UP / DOWN = Move");
    print_centered("A = View Record");
    print_centered("B = Back");
}
void show_imported_record(void)
{
    char path[256];
    char incoming_path[256];
    int installed;
    int incoming = 0;
    FILE *file;
    MARC_Record *record;

    if (imported_record_count == 0 ||
        imported_selection < 0 ||
        imported_selection >= imported_record_count)
        return;

    printf("\x1b[2J\x1b[H");
    print_ui_line('=');
    print_centered("Imported MARC Record");
    print_ui_line('=');
    printf("\n");

    installed =
        find_game_by_id(
            imported_records[imported_selection].game_id
        );

    /*
     * The MARC record is loaded once when the viewer is entered.
     *
     * Keeping it in loaded_marc_record means UP/DOWN scrolling
     * only redraws the already parsed record instead of opening
     * and parsing the .mrc file again on every frame.
     */
    if (loaded_marc_record == NULL)
    {
        snprintf(
            path,
            sizeof(path),
            "sd:/marcviiew/imported/%s",
            imported_records[imported_selection].filename
        );

        file = fopen(path, "rb");

        if (file == NULL)
        {
            snprintf(
                incoming_path,
                sizeof(incoming_path),
                "sd:/marcviiew_import/%s",
                imported_records[imported_selection].filename
            );

            file = fopen(
                incoming_path,
                "rb"
            );

            if (file != NULL)
                incoming = 1;
        }

        if (file == NULL)
        {
            print_centered(
                "Imported .mrc file could not be opened."
            );

            printf("\n");
            print_centered("B = Back");
            return;
        }

        record = read_mrc_record(file);

        fclose(file);

        if (record == NULL)
        {
            print_centered(
                "Could not read imported MARC record."
            );

            printf("\n");
            print_centered(
                "The file is not a readable ISO 2709"
            );
            print_centered(
                "MARC record for ViiewLib."
            );

            printf("\n");
            print_centered("B = Back");
            return;
        }

        loaded_marc_record = record;

        imported_update_metadata_from_record(
            &imported_records[imported_selection],
            loaded_marc_record
        );

        if (incoming)
        {
            snprintf(
                incoming_path,
                sizeof(incoming_path),
                "sd:/marcviiew_import/%s",
                imported_records[imported_selection].filename
            );

            snprintf(
                path,
                sizeof(path),
                "sd:/marcviiew/imported/%s",
                imported_records[imported_selection].filename
            );

            rename(
                incoming_path,
                path
            );
        }
    }

    record = loaded_marc_record;

    print_label_value(
        "Title: ",
        imported_records[imported_selection].title
    );

    print_label_value(
        "Game ID: ",
        imported_records[imported_selection].game_id
    );

    print_label_value(
        "MARC 001: ",
        imported_records[imported_selection].marc_001
    );

    print_centered(
        installed >= 0
            ? "Status: Imported record / Game installed"
            : "Status: Imported record / Game not installed"
    );

    printf("\n");

    render_marc_record(
        record
    );

    {
        int visible_lines = 12;
        int max_scroll =
            marc_line_count - visible_lines;
        int i;

        if (max_scroll < 0)
            max_scroll = 0;

        if (imported_scroll > max_scroll)
            imported_scroll = max_scroll;

        if (imported_scroll < 0)
            imported_scroll = 0;

        for (
            i = imported_scroll;
            i < imported_scroll + visible_lines &&
            i < marc_line_count;
            ++i
        )
        {
            print_centered(
                marc_lines[i]
            );
        }

        printf("\n");

        if (max_scroll > 0)
        {
            char scroll_line[80];

            snprintf(
                scroll_line,
                sizeof(scroll_line),
                "UP / DOWN = Scroll (%d/%d)",
                imported_scroll + 1,
                max_scroll + 1
            );

            print_centered(scroll_line);
        }
        else
        {
            print_centered(
                "UP / DOWN = Scroll"
            );
        }

        print_centered("B = Back");
        print_centered("+ = Main Menu");
    }
}

void encode_entire_database() {

    int result;


    strcpy(
        encode_status,
        "Encoding entire MARC database..."
    );

    show_encode_marc();

    fflush(
        stdout
    );


    result =
        marcviiew_encode_database(
            "sd:/marcviiew_marc.txt",
            "sd:/marcviiew.mrc"
        );


    if (
        result == 0
    ) {

        snprintf(
            encode_status,
            sizeof(encode_status),
            "Complete database exported to sd:/marcviiew.mrc"
        );

    } else {

        strcpy(
            encode_status,
            "Database encoding failed."
        );
    }


    show_encode_marc();
}


/*
    Encode one selected Wii game.

    The selected game's ID comes from games[].

    The encoder searches marcviiew_marc.txt for the
    matching 001 / GAME_ID record and writes only
    that MARC record to its own ISO 2709 file.
*/
void encode_selected_game() {

    int result;

    char output_path[64];

    if (
        game_count == 0
    ) {

        strcpy(
            encode_status,
            "No games are available."
        );

        show_encode_marc();

        return;
    }


    if (
        encode_game_selection < 0 ||
        encode_game_selection >= game_count
    )
        return;


    snprintf(
        output_path,
        sizeof(output_path),
        "sd:/marcviiew_%s.mrc",
        games[
            encode_game_selection
        ].id
    );


    snprintf(
        encode_status,
        sizeof(encode_status),
        "Encoding %s...",
        games[
            encode_game_selection
        ].title
    );


    show_encode_marc();

    fflush(
        stdout
    );


    result =
        marcviiew_encode_game(
            "sd:/marcviiew_marc.txt",
            output_path,
            games[
                encode_game_selection
            ].id
        );


    if (
        result == 0
    ) {

        snprintf(
            encode_status,
            sizeof(encode_status),
            "Exported %s to %s",
            games[
                encode_game_selection
            ].id,
            output_path
        );

    } else {

        snprintf(
            encode_status,
            sizeof(encode_status),
            "Could not export %s.",
            games[
                encode_game_selection
            ].title
        );
    }


    show_encode_marc();
}


/*
    Encode MARC menu.

    0 = Encode Entire Database
    1 = Encode Selected Game
    2 = Back
*/
void show_encode_marc() {

    printf(
        "\x1b[2J\x1b[H"
    );

    print_ui_line('=');

    print_centered(
        "Encode MARC"
    );

    print_ui_line('=');

    printf("\n");


    print_menu_item(
        "Encode Entire Database",
        encode_selection == 0
    );

    print_menu_item(
        "Encode Selected Game",
        encode_selection == 1
    );

    print_menu_item(
        "Back",
        encode_selection == 2
    );


    printf("\n");


    if (
        strlen(
            encode_status
        ) > 0
    ) {

        print_centered(
            encode_status
        );

        printf("\n");
    }


    print_centered(
        "Export only - LMS uses marcviiew_marc.txt"
    );

    printf("\n");

    print_centered(
        "UP / DOWN = Move"
    );

    print_centered(
        "A = Select"
    );

    print_centered(
        "B = Back"
    );

    print_centered(
        "+ = Main Menu"
    );
}


/*
    Game picker for individual MARC export.
*/
void show_encode_game_menu() {

    printf(
        "\x1b[2J\x1b[H"
    );

    print_ui_line('=');

    print_centered(
        "Encode Selected Game"
    );

    print_ui_line('=');

    printf("\n");


    if (
        game_count == 0
    ) {

        print_centered(
            "No games found."
        );

        printf("\n");

        print_centered(
            "B = Back"
        );

        return;
    }


    /*
        Keep the game picker readable on the Wii.
        The selection can still move through all games.
    */
    int first =
        encode_game_selection - 5;

    if (
        first < 0
    )
        first = 0;


    int last =
        first + 10;

    if (
        last > game_count
    )
        last = game_count;


    if (
        last - first < 11
    ) {

        first =
            last - 11;

        if (
            first < 0
        )
            first = 0;
    }


    for (
        int i = first;
        i < last;
        i++
    ) {

        char line[160];

        snprintf(
            line,
            sizeof(line),
            "%s [%s]",
            games[i].title,
            games[i].id
        );

        print_menu_item(
            line,
            i == encode_game_selection
        );
    }


    printf("\n");


    {
        char line[100];

        snprintf(
            line,
            sizeof(line),
            "Game %d of %d",
            encode_game_selection + 1,
            game_count
        );

        print_centered(
            line
        );
    }


    printf("\n");

    print_centered(
        "UP / DOWN = Select Game"
    );

    print_centered(
        "A = Export MARC"
    );

    print_centered(
        "B = Back"
    );

    print_centered(
        "+ = Main Menu"
    );
}


static void show_nand_browser(void);
static void show_nand_file_info(void);

static int nand_get_title_context(
    const char *path,
    char *title_type,
    size_t title_type_size,
    char *title_id,
    size_t title_id_size
);

static int nand_identify_title(const char *path);
static int nand_load_tmd_for_path(const char *path);

static int nand_set_title_uid(const char *path)
{
    char title_type[16];
    char title_id[16];
    u64 titleid;
    s32 result;

    if (!nand_get_title_context(path, title_type, sizeof(title_type), title_id, sizeof(title_id)))
        return 0;

    if (strlen(title_type) != 8 || strlen(title_id) != 8)
        return 0;

    titleid = ((u64)strtoul(title_type, NULL, 16) << 32) |
              (u64)strtoul(title_id, NULL, 16);

    if (!nand_original_titleid_valid)
    {
        if (ES_GetTitleID(&nand_original_titleid) < 0)
            return 0;
        nand_original_titleid_valid = 1;
    }

    if (nand_active_titleid_valid && nand_active_titleid == titleid)
        return 1;

    result = ES_SetUID(titleid);
    if (result < 0)
    {
        snprintf(nand_status, sizeof(nand_status), "ES_SetUID failed (%d).", (int)result);
        return 0;
    }

    nand_active_titleid = titleid;
    nand_active_titleid_valid = 1;
    return 1;
}

static void nand_restore_title_uid(void)
{
    if (!nand_original_titleid_valid || !nand_active_titleid_valid)
        return;

    ES_SetUID(nand_original_titleid);
    nand_active_titleid_valid = 0;
}

static int nand_load_directory(const char *path)
{
    u32 entry_count = 0;
    u32 read_count;
    char *name_buffer;
    s32 result;
    u32 i;

    if (path == NULL || path[0] == '\0')
        return 0;

    /*
     * ISFS permissions are tied to the current IOS title identity. For a
     * title directory, establish that identity before probing content/data.
     * ES_Identify changes permissions without launching the title.
     */
    if (strncmp(path, "/title/", 7) == 0)
    {
        /*
         * ES_Identify is the normal IOS mechanism for temporarily
         * adopting a title's NAND permissions. Once the IOS ES
         * permission checks are patched, this is sufficient for
         * ISFS access; do not overwrite the identity it establishes
         * with ES_SetUID().
         */
        /*
         * Identification is an aid to ISFS permissions, not a prerequisite
         * for browsing. Some system/title paths can legitimately reject
         * ES_Identify even though their parent directory is readable.
         * Let ISFS report the actual access result below.
         */
        nand_identify_title(path);
    }
    else
    {
        nand_restore_title_uid();
    }

    result = ISFS_ReadDir(path, NULL, &entry_count);

    if (result != ISFS_OK)
        return 0;

    nand_total_entry_count = entry_count;
    nand_entry_count = 0;
    nand_selection = 0;
    nand_scroll = 0;

    if (entry_count == 0)
        return 1;

    read_count = entry_count;

    if (read_count > NAND_MAX_ENTRIES)
        read_count = NAND_MAX_ENTRIES;

    /*
     * ISFS_ReadDir() returns a sequence of NUL-terminated names,
     * not a fixed-width array of NAND_ENTRY_NAME_LENGTH-byte slots.
     */
    name_buffer =
        (char *)memalign(
            32,
            (size_t)read_count * ISFS_MAXPATH
        );

    if (name_buffer == NULL)
        return 0;

    result =
        ISFS_ReadDir(
            path,
            name_buffer,
            &read_count
        );

    if (result != ISFS_OK)
    {
        free(name_buffer);
        return 0;
    }

    {
        const char *name = name_buffer;

        for (i = 0; i < read_count && nand_entry_count < NAND_MAX_ENTRIES; ++i)
        {
            NandEntry *entry = &nand_entries[nand_entry_count];
            u32 child_count = 0;
            s32 fd;

            memset(entry, 0, sizeof(*entry));

            snprintf(
                entry->name,
                sizeof(entry->name),
                "%s",
                name
            );

            if (strcmp(path, "/") == 0)
            {
                snprintf(
                    entry->path,
                    sizeof(entry->path),
                    "/%s",
                    entry->name
                );
            }
            else
            {
                snprintf(
                    entry->path,
                    sizeof(entry->path),
                    "%s/%s",
                    path,
                    entry->name
                );
            }

            /*
             * A successful ReadDir() probe identifies a directory.
             * If the probe is denied, try opening it as a file. This
             * keeps the browser honest: inaccessible entries are shown
             * as "no access" instead of pretending they are empty.
             */
            if (ISFS_ReadDir(entry->path, NULL, &child_count) == ISFS_OK)
            {
                entry->type = NAND_ENTRY_DIRECTORY;
                entry->size = child_count;
            }
            else
            {
                fd = ISFS_Open(entry->path, ISFS_OPEN_READ);

                if (fd >= 0)
                {
                    if (ISFS_GetFileStats(fd, &nand_file_stats) == ISFS_OK)
                    {
                        entry->type = NAND_ENTRY_FILE;
                        entry->size = nand_file_stats.file_length;
                    }

                    ISFS_Close(fd);
                }

                if (entry->type == 0)
                {
                    if (nand_is_known_directory(path, entry->name))
                        entry->type = NAND_ENTRY_NOACCESS;
                    else
                        entry->type = NAND_ENTRY_UNKNOWN;

                    entry->size = 0;
                }
            }

            nand_entry_count++;
            name += strlen(name) + 1;
        }
    }

    free(name_buffer);

    return 1;
}

static int nand_get_title_context(
    const char *path,
    char *title_type,
    size_t title_type_size,
    char *title_id,
    size_t title_id_size
)
{
    const char *prefix = "/title/";
    const char *type_start;
    const char *type_end;
    const char *id_start;
    const char *id_end;
    size_t type_length;
    size_t id_length;

    if (path == NULL ||
        title_type == NULL ||
        title_id == NULL ||
        title_type_size == 0 ||
        title_id_size == 0 ||
        strncmp(path, prefix, strlen(prefix)) != 0)
        return 0;

    type_start = path + strlen(prefix);
    type_end = strchr(type_start, '/');

    if (type_end == NULL)
        return 0;

    type_length = (size_t)(type_end - type_start);

    if (type_length == 0 || type_length >= title_type_size)
        return 0;

    memcpy(title_type, type_start, type_length);
    title_type[type_length] = '\0';

    id_start = type_end + 1;
    id_end = strchr(id_start, '/');

    if (id_end == NULL)
        id_end = id_start + strlen(id_start);

    id_length = (size_t)(id_end - id_start);

    if (id_length == 0 || id_length >= title_id_size)
        return 0;

    memcpy(title_id, id_start, id_length);
    title_id[id_length] = '\0';

    return 1;
}

static int nand_identify_title(const char *path)
{
    char title_type[16];
    char title_id[16];
    char cert_path[] = "/sys/cert.sys";
    u64 titleid;
    u32 tmd_size = 0;
    u32 cert_size = 0;
    u32 tik_size = STD_SIGNED_TIK_SIZE;
    signed_blob *tmd_buffer = NULL;
    signed_blob *cert_buffer = NULL;
    signed_blob *tik_buffer = NULL;
    s32 fd;
    s32 result;

    if (!nand_get_title_context(
            path,
            title_type,
            sizeof(title_type),
            title_id,
            sizeof(title_id)))
        return 0;

    if (strlen(title_type) != 8 || strlen(title_id) != 8)
        return 0;

    titleid = ((u64)strtoul(title_type, NULL, 16) << 32) |
              (u64)strtoul(title_id, NULL, 16);

    if (ES_GetStoredTMDSize(titleid, &tmd_size) < 0 ||
        tmd_size == 0 ||
        tmd_size > MAX_SIGNED_TMD_SIZE)
        return 0;

    tmd_buffer = (signed_blob *)memalign(32, tmd_size);
    if (tmd_buffer == NULL)
        return 0;

    if (ES_GetStoredTMD(titleid, tmd_buffer, tmd_size) < 0)
    {
        free(tmd_buffer);
        return 0;
    }

    fd = ISFS_Open(cert_path, ISFS_OPEN_READ);
    if (fd < 0)
    {
        free(tmd_buffer);
        return 0;
    }

    if (ISFS_GetFileStats(fd, &nand_file_stats) != ISFS_OK ||
        nand_file_stats.file_length == 0 ||
        nand_file_stats.file_length > 0x10000)
    {
        ISFS_Close(fd);
        free(tmd_buffer);
        return 0;
    }

    cert_size = nand_file_stats.file_length;
    cert_buffer = (signed_blob *)memalign(32, cert_size);
    if (cert_buffer == NULL)
    {
        ISFS_Close(fd);
        free(tmd_buffer);
        return 0;
    }

    result = ISFS_Read(fd, cert_buffer, cert_size);
    ISFS_Close(fd);

    if (result != (s32)cert_size)
    {
        free(cert_buffer);
        free(tmd_buffer);
        return 0;
    }

    tik_buffer = (signed_blob *)memalign(32, tik_size);
    if (tik_buffer == NULL)
    {
        free(cert_buffer);
        free(tmd_buffer);
        return 0;
    }

    memset(tik_buffer, 0, tik_size);

    {
        sig_rsa2048 *signature = (sig_rsa2048 *)tik_buffer;
        tik *ticket = (tik *)SIGNATURE_PAYLOAD(tik_buffer);

        signature->type = ES_SIG_RSA2048;
        strcpy(ticket->issuer, "Root-CA00000001-XS00000003");
        memset(ticket->cidx_mask, 0xFF, 32);
    }

    result = ES_Identify(
        cert_buffer,
        cert_size,
        tmd_buffer,
        tmd_size,
        tik_buffer,
        tik_size,
        NULL
    );

    free(tik_buffer);
    free(cert_buffer);
    free(tmd_buffer);

    if (result < 0)
    {
        snprintf(
            nand_status,
            sizeof(nand_status),
            "ES_Identify failed (%d).",
            (int)result
        );
        return 0;
    }

    snprintf(
        nand_status,
        sizeof(nand_status),
        "ES_Identify title permissions active."
    );

    return 1;
}

static const char *nand_title_type_name(const char *title_type)
{
    if (title_type == NULL)
        return "Unknown";

    if (strcmp(title_type, "00000001") == 0)
        return "Essential system title";

    if (strcmp(title_type, "00010000") == 0)
        return "Disc-based title";

    if (strcmp(title_type, "00010001") == 0)
        return "Downloadable channel";

    if (strcmp(title_type, "00010002") == 0)
        return "System channel";

    if (strcmp(title_type, "00010004") == 0)
        return "Game channel";

    if (strcmp(title_type, "00010005") == 0)
        return "Downloadable content";

    if (strcmp(title_type, "00010008") == 0)
        return "Hidden channel";

    return "Unknown title type";
}

static u16 nand_read_be16(const u8 *data)
{
    return (u16)(((u16)data[0] << 8) | data[1]);
}

static u32 nand_read_be32(const u8 *data)
{
    return ((u32)data[0] << 24) |
           ((u32)data[1] << 16) |
           ((u32)data[2] << 8) |
           (u32)data[3];
}

static int nand_load_tmd_contents(const char *title_path)
{
    char tmd_path[ISFS_MAXPATH];
    u8 header[0x1E4 + 2] ATTRIBUTE_ALIGN(32);
    u8 *records = NULL;
    u16 content_count;
    u32 i;
    s32 fd;
    s32 bytes_read;
    size_t records_size;

    nand_tmd_content_count = 0;
    nand_tmd_loaded = 0;

    if (title_path == NULL || title_path[0] == '\0')
        return 0;

    snprintf(
        tmd_path,
        sizeof(tmd_path),
        "%s/content/title.tmd",
        title_path
    );

    fd = ISFS_Open(tmd_path, ISFS_OPEN_READ);

    if (fd < 0)
        return 0;

    bytes_read = ISFS_Read(fd, header, sizeof(header));

    if (bytes_read != (s32)sizeof(header))
    {
        ISFS_Close(fd);
        return 0;
    }

    content_count = nand_read_be16(&header[0x1DE]);

    if (content_count == 0 || content_count > NAND_MAX_TMD_CONTENTS)
    {
        ISFS_Close(fd);
        return 0;
    }

    records_size = (size_t)content_count * 0x24;

    /*
     * ISFS_Read() requires a 32-byte aligned destination. Read the
     * content records directly from the start of the TMD instead of
     * seeking the ISFS file descriptor. This avoids relying on IOS seek
     * state while the NAND browser is inspecting a protected title.
     */
    records = (u8 *)memalign(32, 0x1E4 + records_size);

    if (records == NULL)
    {
        ISFS_Close(fd);
        return 0;
    }

    if (ISFS_Seek(fd, 0, SEEK_SET) < 0)
    {
        free(records);
        ISFS_Close(fd);
        return 0;
    }

    bytes_read = ISFS_Read(fd, records, 0x1E4 + records_size);
    ISFS_Close(fd);

    if (bytes_read != (s32)(0x1E4 + records_size))
    {
        free(records);
        return 0;
    }

    for (i = 0; i < content_count; ++i)
    {
        const u8 *record = &records[0x1E4 + (i * 0x24)];

        nand_tmd_contents[i].content_id =
            nand_read_be32(&record[0]);
        nand_tmd_contents[i].index =
            nand_read_be16(&record[4]);
        nand_tmd_contents[i].type =
            nand_read_be16(&record[6]);

        nand_tmd_contents[i].size =
            ((u64)nand_read_be32(&record[8]) << 32) |
            (u64)nand_read_be32(&record[12]);

        memcpy(
            nand_tmd_contents[i].hash,
            &record[16],
            sizeof(nand_tmd_contents[i].hash)
        );
    }

    nand_tmd_content_count = content_count;
    nand_tmd_loaded = 1;

    free(records);
    return 1;
}

static const NandContentInfo *nand_find_content_info(u32 content_id)
{
    u32 i;

    if (!nand_tmd_loaded)
        return NULL;

    for (i = 0; i < nand_tmd_content_count; ++i)
    {
        if (nand_tmd_contents[i].content_id == content_id)
            return &nand_tmd_contents[i];
    }

    return NULL;
}

static int nand_get_current_title_path(
    const char *path,
    char *title_path,
    size_t title_path_size
)
{
    char title_type[16];
    char title_id[16];

    if (!nand_get_title_context(
            path,
            title_type,
            sizeof(title_type),
            title_id,
            sizeof(title_id)))
        return 0;

    snprintf(
        title_path,
        title_path_size,
        "/title/%s/%s",
        title_type,
        title_id
    );

    return 1;
}

static int nand_parse_content_id(
    const char *name,
    u32 *content_id
)
{
    char *end;
    unsigned long value;

    if (name == NULL ||
        content_id == NULL ||
        strlen(name) != 12 ||
        name[8] != '.' ||
        tolower((unsigned char)name[9]) != 'a' ||
        tolower((unsigned char)name[10]) != 'p' ||
        tolower((unsigned char)name[11]) != 'p')
        return 0;

    value = strtoul(name, &end, 16);

    if (end != name + 8)
        return 0;

    *content_id = (u32)value;
    return 1;
}

static int nand_load_tmd_for_path(const char *path)
{
    char title_path[ISFS_MAXPATH];

    if (!nand_get_current_title_path(
            path,
            title_path,
            sizeof(title_path)))
    {
        nand_tmd_loaded = 0;
        nand_tmd_content_count = 0;
        return 0;
    }

    return nand_load_tmd_contents(title_path);
}

static int nand_read_title_name(
    const char *title_path,
    char *title_name,
    size_t title_name_size
)
{
    char tmd_path[ISFS_MAXPATH];
    char app_path[ISFS_MAXPATH];
    int fd;
    s32 bytes_read;
    u8 *tmd_buffer = NULL;
    u8 banner_buffer[0x80 + 0x640] ATTRIBUTE_ALIGN(32);
    u32 content_id = 0;
    u16 content_count;
    u16 content_index;
    u32 i;
    int found_content = 0;
    const u8 *imet;
    const u8 *name_table;
    const u8 *name_utf16;
    size_t j;

    if (title_path == NULL ||
        title_name == NULL ||
        title_name_size < 2)
        return 0;

    title_name[0] = '\0';

    snprintf(tmd_path, sizeof(tmd_path), "%s/content/title.tmd", title_path);

    fd = ISFS_Open(tmd_path, ISFS_OPEN_READ);
    if (fd < 0)
        return 0;

    {
        u8 tmd_header[0x1E4 + 2] ATTRIBUTE_ALIGN(32);

        bytes_read = ISFS_Read(fd, tmd_header, sizeof(tmd_header));

        if (bytes_read != (s32)sizeof(tmd_header))
        {
            ISFS_Close(fd);
            return 0;
        }

        content_count = nand_read_be16(&tmd_header[0x1DE]);
    }

    if (content_count == 0)
    {
        ISFS_Close(fd);
        return 0;
    }

    tmd_buffer = (u8 *)memalign(
        32,
        0x1E4 + ((size_t)content_count * 0x24)
    );

    if (tmd_buffer == NULL)
    {
        ISFS_Close(fd);
        return 0;
    }

    if (ISFS_Seek(fd, 0, SEEK_SET) < 0)
    {
        free(tmd_buffer);
        ISFS_Close(fd);
        return 0;
    }

    bytes_read = ISFS_Read(
        fd,
        tmd_buffer,
        0x1E4 + ((size_t)content_count * 0x24)
    );

    ISFS_Close(fd);

    if (bytes_read != (s32)(0x1E4 + ((size_t)content_count * 0x24)))
    {
        free(tmd_buffer);
        return 0;
    }

    for (i = 0; i < content_count; ++i)
    {
        const u8 *content =
            &tmd_buffer[0x1E4 + ((size_t)i * 0x24)];

        content_index = nand_read_be16(&content[4]);

        if (content_index == 0)
        {
            content_id = nand_read_be32(&content[0]);
            found_content = 1;
            break;
        }
    }

    free(tmd_buffer);

    if (!found_content)
        return 0;

    snprintf(
        app_path,
        sizeof(app_path),
        "%s/content/%08x.app",
        title_path,
        (unsigned int)content_id
    );

    fd = ISFS_Open(app_path, ISFS_OPEN_READ);
    if (fd < 0)
        return 0;

    bytes_read = ISFS_Read(
        fd,
        banner_buffer,
        sizeof(banner_buffer)
    );

    ISFS_Close(fd);

    if (bytes_read < (s32)(0x80 + 0x640))
        return 0;

    imet = &banner_buffer[0x80];

    if (imet[0] != 'I' ||
        imet[1] != 'M' ||
        imet[2] != 'E' ||
        imet[3] != 'T')
        return 0;

    /* The IMET name table begins after the 0x1C-byte IMET header. */
    name_table = &imet[0x1C];

    {
        const u32 language_order[10] = {
            1, 0, 2, 3, 4, 5, 6, 7, 8, 9
        };

        for (i = 0; i < 10; ++i)
        {
            u32 language = language_order[i];

            name_utf16 = name_table + ((size_t)language * 84);

        if (name_utf16[0] == 0 && name_utf16[1] == 0)
            continue;

        {
            size_t out = 0;

            for (j = 0; j < 42; ++j)
            {
                u16 codepoint =
                    (u16)(((u16)name_utf16[j * 2] << 8) |
                          name_utf16[j * 2 + 1]);

                if (codepoint == 0)
                    break;

                if (codepoint >= 0xD800 && codepoint <= 0xDFFF)
                    break;

                if (codepoint < 0x80)
                {
                    if (out + 1 >= title_name_size)
                        break;

                    title_name[out++] = (char)codepoint;
                }
                else if (codepoint < 0x800)
                {
                    if (out + 2 >= title_name_size)
                        break;

                    title_name[out++] =
                        (char)(0xC0 | (codepoint >> 6));
                    title_name[out++] =
                        (char)(0x80 | (codepoint & 0x3F));
                }
                else
                {
                    if (out + 3 >= title_name_size)
                        break;

                    title_name[out++] =
                        (char)(0xE0 | (codepoint >> 12));
                    title_name[out++] =
                        (char)(0x80 | ((codepoint >> 6) & 0x3F));
                    title_name[out++] =
                        (char)(0x80 | (codepoint & 0x3F));
                }
            }

            title_name[out] = '\0';

            if (out != 0)
                return 1;
        }
    }
    }

    return 0;
}

static int nand_decode_title_id(
    const char *title_id,
    char *decoded,
    size_t decoded_size
);

static const char *nand_database_title(const char *title_id)
{
    int existing;
    char decoded_id[8];

    if (title_id == NULL || title_id[0] == '\0')
        return NULL;

    /*
     * The NAND title directory is the authoritative GAMEID we display.
     * MarcViiew's derived database may store a four-character GAMEID
     * directly (for example NALE), while an ES title ID can appear as
     * its hexadecimal byte representation (for example 4E414C45).
     *
     * Try the directory value first, then the decoded four-character
     * form when the directory is an 8-digit hexadecimal title ID.
     */
    existing = find_game_by_id(title_id);

    if (existing >= 0 &&
        games[existing].title[0] != '\0' &&
        strcmp(games[existing].title, title_id) != 0)
        return games[existing].title;

    if (!nand_decode_title_id(
            title_id,
            decoded_id,
            sizeof(decoded_id)))
        return NULL;

    existing = find_game_by_id(decoded_id);

    if (existing < 0 ||
        games[existing].title[0] == '\0' ||
        strcmp(games[existing].title, decoded_id) == 0)
        return NULL;

    return games[existing].title;
}

static int nand_decode_title_id(
    const char *title_id,
    char *decoded,
    size_t decoded_size
)
{
    size_t i;

    if (title_id == NULL ||
        decoded == NULL ||
        decoded_size < 5 ||
        strlen(title_id) != 8)
        return 0;

    for (i = 0; i < 8; ++i)
    {
        if (!isxdigit((unsigned char)title_id[i]))
            return 0;
    }

    for (i = 0; i < 4; ++i)
    {
        char hex_pair[3];
        unsigned int value;

        hex_pair[0] = title_id[i * 2];
        hex_pair[1] = title_id[i * 2 + 1];
        hex_pair[2] = '\0';

        value = (unsigned int)strtoul(hex_pair, NULL, 16);

        if (value < 0x20 || value > 0x7E)
            return 0;

        decoded[i] = (char)value;
    }

    decoded[4] = '\0';
    return 1;
}

static void nand_format_size(u32 size, char *output, size_t output_size)
{
    if (size >= 1024 * 1024)
    {
        snprintf(
            output,
            output_size,
            "%.2f MiB",
            (double)size / (1024.0 * 1024.0)
        );
    }
    else if (size >= 1024)
    {
        snprintf(
            output,
            output_size,
            "%.2f KiB",
            (double)size / 1024.0
        );
    }
    else
    {
        snprintf(
            output,
            output_size,
            "%u B",
            (unsigned int)size
        );
    }
}

static int nand_go_parent(void)
{
    char *last_slash;

    if (strcmp(nand_current_path, "/") == 0)
        return 0;

    last_slash = strrchr(nand_current_path, '/');

    if (last_slash == NULL || last_slash == nand_current_path)
        strcpy(nand_current_path, "/");
    else
        *last_slash = '\0';

    if (!nand_load_directory(nand_current_path))
    {
        strcpy(nand_current_path, "/");
        nand_load_directory(nand_current_path);
    }

    return 1;
}

static int nand_enter_selected(void)
{
    NandEntry *entry;
    char previous_path[ISFS_MAXPATH];

    if (nand_entry_count == 0 ||
        nand_selection < 0 ||
        (u32)nand_selection >= nand_entry_count)
        return 0;

    entry = &nand_entries[nand_selection];

    if (entry->type != NAND_ENTRY_DIRECTORY)
        return 0;

    snprintf(
        previous_path,
        sizeof(previous_path),
        "%s",
        nand_current_path
    );

    snprintf(
        nand_current_path,
        sizeof(nand_current_path),
        "%s",
        entry->path
    );

    if (nand_load_directory(nand_current_path))
        return 1;

    snprintf(
        nand_current_path,
        sizeof(nand_current_path),
        "%s",
        previous_path
    );

    nand_load_directory(nand_current_path);
    return 0;
}

static void show_nand_browser(void)
{
    int i;
    int visible_end;
    char line[160];

    printf("\x1b[2J\x1b[H");
    print_ui_line('=');
    print_centered("NAND Root Navigation");
    print_ui_line('=');
    printf("\n");

    if (!nand_initialized)
    {
        print_centered("NAND filesystem unavailable.");
        if (nand_status[0] != '\0')
            print_centered(nand_status);

        printf("\n");
        print_centered("B = Back    PLUS = Main Menu");
        return;
    }

    if (nand_status[0] != '\0')
        print_centered(nand_status);

    snprintf(
        line,
        sizeof(line),
        "Path: %s",
        nand_current_path
    );
    print_centered(line);

    {
        char title_type[32];
        char title_id[32];

        if (nand_get_title_context(
                nand_current_path,
                title_type,
                sizeof(title_type),
                title_id,
                sizeof(title_id)))
        {
            /*
             * Keep the NAND identifiers literal here. The first
             * directory under /title/ is the raw title type and the
             * second directory is the NAND GAMEID.
             */
            snprintf(
                line,
                sizeof(line),
                "Title type: %s",
                title_type
            );
            print_centered(line);

            snprintf(
                line,
                sizeof(line),
                "Title ID: %s",
                title_id
            );
            print_centered(line);

            {
                const char *database_title =
                    nand_database_title(title_id);

                snprintf(
                    line,
                    sizeof(line),
                    "Title name: %s",
                    database_title != NULL
                        ? database_title
                        : "Unknown"
                );
                print_centered(line);
            }
        }
    }

    if (nand_total_entry_count > NAND_MAX_ENTRIES)
    {
        snprintf(
            line,
            sizeof(line),
            "Showing first %u of %u entries",
            (unsigned int)nand_entry_count,
            (unsigned int)nand_total_entry_count
        );
        print_centered(line);
    }

    printf("\n");

    if (nand_entry_count == 0)
    {
        print_centered("Directory is empty.");
    }
    else
    {
        if (nand_selection < 0)
            nand_selection = 0;

        if ((u32)nand_selection >= nand_entry_count)
            nand_selection = (int)nand_entry_count - 1;

        if (nand_scroll < 0)
            nand_scroll = 0;

        if (nand_selection < nand_scroll)
            nand_scroll = nand_selection;

        if (nand_selection >=
            nand_scroll + GAME_LIST_VISIBLE_ITEMS)
        {
            nand_scroll =
                nand_selection -
                GAME_LIST_VISIBLE_ITEMS + 1;
        }

        visible_end =
            nand_scroll + GAME_LIST_VISIBLE_ITEMS;

        if ((u32)visible_end > nand_entry_count)
            visible_end = (int)nand_entry_count;

        for (i = nand_scroll; i < visible_end; ++i)
        {
            NandEntry *entry = &nand_entries[i];
            char type;
            char size_text[32];

            if (entry->type == NAND_ENTRY_DIRECTORY)
                type = 'D';
            else if (entry->type == NAND_ENTRY_FILE)
                type = 'F';
            else if (entry->type == NAND_ENTRY_NOACCESS)
                type = 'D';
            else
                type = '?';

            if (entry->type == NAND_ENTRY_FILE)
            {
                nand_format_size(
                    entry->size,
                    size_text,
                    sizeof(size_text)
                );

                {
                    u32 content_id;
                    const NandContentInfo *content_info =
                        nand_parse_content_id(entry->name, &content_id)
                            ? nand_find_content_info(content_id)
                            : NULL;

                    if (content_info != NULL)
                    {
                        snprintf(
                            size_text,
                            sizeof(size_text),
                            "idx %u, %u B",
                            (unsigned int)content_info->index,
                            (unsigned int)entry->size
                        );
                    }
                }
            }
            else if (entry->type == NAND_ENTRY_DIRECTORY)
                snprintf(
                    size_text,
                    sizeof(size_text),
                    "%u entries",
                    (unsigned int)entry->size
                );
            else if (entry->type == NAND_ENTRY_NOACCESS)
                snprintf(
                    size_text,
                    sizeof(size_text),
                    "no access"
                );
            else
                snprintf(
                    size_text,
                    sizeof(size_text),
                    "unknown"
                );

            snprintf(
                line,
                sizeof(line),
                "%c [%c] %-12s %s",
                i == nand_selection ? '>' : ' ',
                type,
                entry->name,
                size_text
            );

            print_centered(line);
        }
    }

    printf("\n");
    print_centered("UP / DOWN = Move    A = Open");
    print_centered("B = Back    PLUS = Main Menu");
}

static void show_nand_file_info(void)
{
    NandEntry *entry;
    char line[160];
    char size_text[32];

    if (nand_entry_count == 0 ||
        nand_selection < 0 ||
        (u32)nand_selection >= nand_entry_count)
        return;

    entry = &nand_entries[nand_selection];

    printf("\x1b[2J\x1b[H");
    print_ui_line('=');
    print_centered("NAND File Information");
    print_ui_line('=');
    printf("\n");

    snprintf(line, sizeof(line), "Name: %s", entry->name);
    print_centered(line);

    snprintf(line, sizeof(line), "Path: %s", entry->path);
    print_centered(line);

    {
        char title_type[32];
        char title_id[32];

        if (nand_get_title_context(
                entry->path,
                title_type,
                sizeof(title_type),
                title_id,
                sizeof(title_id)))
        {
            char title_code[8];

            snprintf(
                line,
                sizeof(line),
                "Title type: %s (%s)",
                title_type,
                nand_title_type_name(title_type)
            );
            print_centered(line);

            if (nand_decode_title_id(
                    title_id,
                    title_code,
                    sizeof(title_code)))
            {
                snprintf(
                    line,
                    sizeof(line),
                    "Title ID: %s (%s)",
                    title_id,
                    title_code
                );
            }
            else
            {
                snprintf(
                    line,
                    sizeof(line),
                    "Title ID: %s",
                    title_id
                );
            }
            print_centered(line);
        }
    }

    nand_format_size(entry->size, size_text, sizeof(size_text));

    snprintf(line, sizeof(line), "Size: %s", size_text);
    print_centered(line);

    {
        u32 content_id;
        const NandContentInfo *content_info;

        /*
         * Load the TMD lazily here rather than while entering a title
         * directory. Some titles expose their content directory through
         * IOS in ways that make an eager TMD read unsafe on real NAND.
         */
        nand_load_tmd_for_path(entry->path);

        content_info =
            nand_parse_content_id(entry->name, &content_id)
                ? nand_find_content_info(content_id)
                : NULL;

        if (content_info != NULL)
        {
            char hash_text[64];

            snprintf(
                hash_text,
                sizeof(hash_text),
                "%02x%02x%02x%02x...%02x%02x",
                content_info->hash[0],
                content_info->hash[1],
                content_info->hash[2],
                content_info->hash[3],
                content_info->hash[18],
                content_info->hash[19]
            );

            snprintf(
                line,
                sizeof(line),
                "TMD content ID: %08x",
                (unsigned int)content_info->content_id
            );
            print_centered(line);

            snprintf(
                line,
                sizeof(line),
                "TMD index: %u",
                (unsigned int)content_info->index
            );
            print_centered(line);

            snprintf(
                line,
                sizeof(line),
                "TMD type: %04x",
                (unsigned int)content_info->type
            );
            print_centered(line);

            /*
             * Keep the 64-bit TMD size out of the console printf path.
             * The raw file size is already displayed above, while the
             * TMD size is retained internally for later comparison.
             */
            snprintf(
                line,
                sizeof(line),
                "TMD record size: %u:%u",
                (unsigned int)(content_info->size >> 32),
                (unsigned int)(content_info->size & 0xFFFFFFFF)
            );
            print_centered(line);

            snprintf(
                line,
                sizeof(line),
                "TMD SHA-1: %s",
                hash_text
            );
            print_centered(line);
        }
    }

    printf("\n");
    print_centered("Read-only inspection");
    printf("\n");
    print_centered("B = Back    PLUS = Main Menu");
}

void show_main_menu() {

    printf(
        "\x1b[2J\x1b[H"
    );

    printf("\n");

    print_ui_line('=');

    print_centered(
        "MarcViiew"
    );

    print_centered(
        "Wii Library Management System"
    );

    print_ui_line('=');

    printf("\n");


    print_menu_item(
        "Catalogue",
        menu_selection == 0
    );

    print_menu_item(
        "Search",
        menu_selection == 1
    );

    print_menu_item(
        "MARC Records",
        menu_selection == 2
    );

    print_menu_item(
        "Encode MARC",
        menu_selection == 3
    );

    print_menu_item(
        "Imported Records",
        menu_selection == 4
    );

    print_menu_item(
        "NAND Root Navigation",
        menu_selection == 5
    );

    print_menu_item(
        "Settings",
        menu_selection == 6
    );


    printf("\n");

    print_centered(
        "UP / DOWN = Move    A = Select"
    );

    print_centered(
        "HOME = Exit"
    );
}


void show_catalogue() {

    printf(
        "\x1b[2J\x1b[H"
    );

    print_ui_line('=');

    print_centered(
        "Catalogue"
    );

    print_ui_line('=');

    printf("\n");


    if (
        game_count == 0
    ) {

        print_centered(
            "No games found."
        );

        printf("\n");

        print_centered(
            "B = Back"
        );

        return;
    }


    if (
        catalogue_selection < 0
    )
        catalogue_selection = 0;

    if (
        catalogue_selection >= game_count
    )
        catalogue_selection = game_count - 1;

    if (
        catalogue_scroll < 0
    )
        catalogue_scroll = 0;

    if (
        catalogue_selection < catalogue_scroll
    )
        catalogue_scroll = catalogue_selection;

    if (
        catalogue_selection >=
        catalogue_scroll +
        GAME_LIST_VISIBLE_ITEMS
    )
        catalogue_scroll =
            catalogue_selection -
            GAME_LIST_VISIBLE_ITEMS +
            1;

    if (
        catalogue_scroll >
        game_count -
        GAME_LIST_VISIBLE_ITEMS
    ) {

        catalogue_scroll =
            game_count -
            GAME_LIST_VISIBLE_ITEMS;

        if (
            catalogue_scroll < 0
        )
            catalogue_scroll = 0;
    }


    int visible_end =
        catalogue_scroll +
        GAME_LIST_VISIBLE_ITEMS;

    if (
        visible_end > game_count
    )
        visible_end = game_count;


    for (
        int i = catalogue_scroll;
        i < visible_end;
        i++
    ) {

        print_menu_item(
            games[i].title,
            i == catalogue_selection
        );
    }


    printf("\n");


    {
        char line[80];

        snprintf(
            line,
            sizeof(line),
            "%d game(s) found.",
            game_count
        );

        print_centered(
            line
        );
    }


    printf("\n");

    print_centered(
        "UP / DOWN = Move"
    );

    print_centered(
        "A = View Game"
    );

    print_centered(
        "B = Back"
    );
}


void show_search_results() {

    printf(
        "\x1b[2J\x1b[H"
    );

    print_ui_line('=');


    if (
        search_mode ==
        SEARCH_MODE_MARC21
    )
        print_centered(
            "MARC21 Search Results"
        );
    else
        print_centered(
            "Search Results"
        );


    print_ui_line('=');

    printf("\n");


    {
        char line[160];

        snprintf(
            line,
            sizeof(line),
            "Search: %s",
            search_query
        );

        print_centered(
            line
        );
    }


    printf("\n");


    if (
        search_result_count == 0
    ) {

        print_centered(
            "No games found."
        );

        printf("\n");

        print_centered(
            "B = Back"
        );

        print_centered(
            "+ = Main Menu"
        );

        return;
    }


    for (
        int i = 0;
        i < search_result_count;
        i++
    ) {

        int game_index =
            search_results[i];


        print_menu_item(
            games[game_index].title,
            i == search_selection
        );
    }


    printf("\n");


    {
        char line[80];

        snprintf(
            line,
            sizeof(line),
            "%d result(s) found.",
            search_result_count
        );

        print_centered(
            line
        );
    }


    printf("\n");

    print_centered(
        "UP / DOWN = Move"
    );


    if (
        search_mode ==
        SEARCH_MODE_MARC21
    )
        print_centered(
            "A = View MARC Record"
        );
    else
        print_centered(
            "A = View Game"
        );


    print_centered(
        "B = Back"
    );

    print_centered(
        "+ = Main Menu"
    );
}


void show_settings_menu() {

    printf(
        "\x1b[2J\x1b[H"
    );

    print_ui_line('=');

    print_centered(
        "Settings"
    );

    print_ui_line('=');

    printf("\n");


    print_menu_item(
        "Reload SD / USB",
        settings_selection == 0
    );

    print_menu_item(
        "Reload NAND",
        settings_selection == 1
    );

    print_menu_item(
        "Reload Databases",
        settings_selection == 2
    );

    print_menu_item(
        "Credits",
        settings_selection == 3
    );


    printf("\n");


    if (
        strlen(settings_status) > 0
    ) {

        print_centered(
            settings_status
        );

        printf("\n");
    }


    print_centered(
        "UP / DOWN = Move"
    );

    print_centered(
        "A = Select"
    );

    print_centered(
        "B = Back"
    );

    print_centered(
        "+ = Main Menu"
    );
}


void settings_reload_storage() {

    int result;


    strcpy(
        settings_status,
        "Reloading SD / USB..."
    );

    show_settings_menu();

    fflush(
        stdout
    );


    usleep(
        200000
    );


    result =
        reload_storage();


    if (
        result
    ) {

        scan_catalogue();


        catalogue_selection = 0;
        marc_selection = 0;

        info_scroll = 0;
        marc_scroll = 0;

        search_selection = 0;
        search_result_count = 0;

        marc_record_from_search = 0;

        encode_selection = 0;
        encode_game_selection = 0;


        snprintf(
            settings_status,
            sizeof(settings_status),
            "SD / USB reloaded. %d game(s) found.",
            game_count
        );

    } else {

        strcpy(
            settings_status,
            "SD / USB reload failed."
        );
    }


    show_settings_menu();
}


void settings_reload_nand() {

    strcpy(
        settings_status,
        "Reloading NAND..."
    );

    show_settings_menu();

    fflush(
        stdout
    );

    usleep(
        200000
    );

    /*
        Rebuild the complete catalogue so titles removed from
        NAND do not remain as stale entries. The NAND scanner
        itself is refreshed as part of scan_catalogue().
    */
    scan_catalogue();

    catalogue_selection = 0;
    marc_selection = 0;

    info_scroll = 0;
    marc_scroll = 0;

    search_selection = 0;
    search_result_count = 0;

    marc_record_from_search = 0;

    encode_selection = 0;
    encode_game_selection = 0;

    snprintf(
        settings_status,
        sizeof(settings_status),
        "NAND reloaded. %d game(s) found.",
        game_count
    );

    show_settings_menu();
}


void settings_reload_databases() {

    strcpy(
        settings_status,
        "Reloading databases..."
    );

    show_settings_menu();

    fflush(
        stdout
    );


    usleep(
        200000
    );


    reload_databases();


    show_settings_menu();
}


void show_credits() {

    printf(
        "\x1b[2J\x1b[H"
    );

    print_ui_line('=');

    print_centered(
        "Credits"
    );

    print_ui_line('=');

    printf("\n");


    print_centered(
        "MarcViiew"
    );

    print_centered(
        "Wii Library Management System"
    );


    printf("\n");

    print_centered(
        "Created by Amanda/Riruru. A library and Information Sciences student. Tested extensively by LilyFlower on VWii, provider of external .mrc files."
    );


    printf("\n");

    print_ui_line('-');

    printf("\n");


    print_centered(
        "Built with"
    );

    print_centered(
        "devkitPPC"
    );

    print_centered(
        "libogc"
    );

    print_centered(
        "ViiewLib"
    );

    printf("\n");

    print_ui_line('-');

    printf("\n");


    print_centered(
        "A Wii homebrew library"
    );

    print_centered(
        "management system."
    );


    printf("\n");

    print_centered(
        "B = Back"
    );

    print_centered(
        "+ = Main Menu"
    );
}


void show_search_mode_menu() {

    printf(
        "\x1b[2J\x1b[H"
    );

    printf("\n");

    print_ui_line('=');

    print_centered(
        "Search"
    );

    print_ui_line('=');

    printf("\n");

    print_centered(
        "Choose record type"
    );

    printf("\n");


    print_menu_item(
        "Normal Records",
        search_mode_selection == 0
    );

    print_menu_item(
        "MARC21 Records",
        search_mode_selection == 1
    );


    printf("\n");

    print_centered(
        "UP / DOWN = Move"
    );

    print_centered(
        "A = Select"
    );

    print_centered(
        "B = Back"
    );

    print_centered(
        "+ = Main Menu"
    );
}


void reset_keyboard() {

    keyboard_row = 0;

    keyboard_column = 0;

    keyboard_special_selection = 0;

    search_length = 0;

    search_query[0] = '\0';
}


void keyboard_add_character(
    char character
) {

    if (
        search_length <
        SEARCH_LENGTH - 1
    ) {

        search_query[
            search_length
        ] = character;

        search_length++;

        search_query[
            search_length
        ] = '\0';
    }
}


void keyboard_backspace() {

    if (
        search_length > 0
    ) {

        search_length--;

        search_query[
            search_length
        ] = '\0';
    }
}


void show_search_keyboard() {

    printf(
        "\x1b[2J\x1b[H"
    );

    printf("\n");

    print_ui_line('=');

    print_centered(
        "Search"
    );

    print_ui_line('=');

    printf("\n");


    {
        char query_line[160];

        snprintf(
            query_line,
            sizeof(query_line),
            "Query: %s_",
            search_query
        );

        print_centered(
            query_line
        );
    }


    printf("\n");


    if (
        search_mode ==
        SEARCH_MODE_MARC21
    )
        print_centered(
            "MARC21 Records"
        );
    else
        print_centered(
            "Normal Records"
        );


    printf("\n");


    for (
        int row = 0;
        row < KEYBOARD_ROWS;
        row++
    ) {

        char keyboard_line[64];

        int position = 0;


        for (
            int column = 0;
            column < KEYBOARD_COLUMNS;
            column++
        ) {

            if (
                row == 3 &&
                column >= 6
            ) {

                keyboard_line[
                    position++
                ] = ' ';

                keyboard_line[
                    position++
                ] = ' ';

                keyboard_line[
                    position++
                ] = ' ';

                keyboard_line[
                    position++
                ] = ' ';

                keyboard_line[
                    position++
                ] = ' ';

                continue;
            }


            keyboard_line[
                position++
            ] = '[';


            if (
                row == keyboard_row &&
                column == keyboard_column
            ) {

                keyboard_line[
                    position++
                ] = '>';

            } else {

                keyboard_line[
                    position++
                ] = ' ';
            }


            keyboard_line[
                position++
            ] = keyboard[
                row
            ][
                column
            ];

            keyboard_line[
                position++
            ] = ']';

            keyboard_line[
                position++
            ] = ' ';
        }


        keyboard_line[position] =
            '\0';


        print_centered(
            keyboard_line
        );
    }


    printf("\n");


    {
        char special_line[64];

        int position = 0;


        if (
            keyboard_row == 4 &&
            keyboard_special_selection == 0
        ) {

            const char *text =
                "[> SPACE ] ";

            strcpy(
                special_line + position,
                text
            );

            position += strlen(text);

        } else {

            const char *text =
                "[  SPACE ] ";

            strcpy(
                special_line + position,
                text
            );

            position += strlen(text);
        }


        if (
            keyboard_row == 4 &&
            keyboard_special_selection == 1
        ) {

            const char *text =
                "[>BACKSP] ";

            strcpy(
                special_line + position,
                text
            );

            position += strlen(text);

        } else {

            const char *text =
                "[ BACKSP ] ";

            strcpy(
                special_line + position,
                text
            );

            position += strlen(text);
        }


        if (
            keyboard_row == 4 &&
            keyboard_special_selection == 2
        ) {

            const char *text =
                "[>SEARCH ]";

            strcpy(
                special_line + position,
                text
            );

            position += strlen(text);

        } else {

            const char *text =
                "[ SEARCH ]";

            strcpy(
                special_line + position,
                text
            );

            position += strlen(text);
        }


        special_line[position] =
            '\0';


        print_centered(
            special_line
        );
    }


    printf("\n");

    print_centered(
        "D-PAD = Move"
    );

    print_centered(
        "A = Select"
    );

    print_centered(
        "B = Backspace"
    );

    print_centered(
        "+ = Main Menu"
    );
}


void keyboard_move_up() {

    if (
        keyboard_row == 4
    ) {

        keyboard_row = 3;

        if (
            keyboard_column > 5
        )
            keyboard_column = 5;

        return;
    }


    if (
        keyboard_row > 0
    ) {

        keyboard_row--;

        if (
            keyboard_row == 3 &&
            keyboard_column > 5
        )
            keyboard_column = 5;
    }
}


void keyboard_move_down() {

    if (
        keyboard_row < 4
    ) {

        keyboard_row++;

        if (
            keyboard_row == 4
        )
            keyboard_special_selection =
                keyboard_column > 2
                ? 2
                : keyboard_column;

        return;
    }
}


void keyboard_move_left() {

    if (
        keyboard_row == 4
    ) {

        if (
            keyboard_special_selection > 0
        )
            keyboard_special_selection--;

        return;
    }


    if (
        keyboard_column > 0
    )
        keyboard_column--;
}


void keyboard_move_right() {

    if (
        keyboard_row == 4
    ) {

        if (
            keyboard_special_selection < 2
        )
            keyboard_special_selection++;

        return;
    }


    int max_column = 9;

    if (
        keyboard_row == 3
    )
        max_column = 5;


    if (
        keyboard_column < max_column
    )
        keyboard_column++;
}


void keyboard_select() {

    if (
        keyboard_row < 4
    ) {

        char character =
            keyboard[
                keyboard_row
            ][
                keyboard_column
            ];


        keyboard_add_character(
            character
        );

        return;
    }


    if (
        keyboard_special_selection == 0
    ) {

        keyboard_add_character(
            ' '
        );

    }

    else if (
        keyboard_special_selection == 1
    ) {

        keyboard_backspace();

    }

    else if (
        keyboard_special_selection == 2
    ) {

        perform_search();

        screen = 6;

        show_search_results();
    }
}


void handle_search_keyboard(
    u32 input
) {

    if (
        input & INPUT_HOME
    ) {

        exit(0);
    }


    if (
        input & INPUT_PLUS
    ) {

        screen = 1;

        search_result_count = 0;

        marc_record_from_search = 0;

        show_main_menu();

        return;
    }


    if (
        input & INPUT_UP
    ) {

        keyboard_move_up();

        show_search_keyboard();

    }

    else if (
        input & INPUT_DOWN
    ) {

        keyboard_move_down();

        show_search_keyboard();

    }

    else if (
        input & INPUT_LEFT
    ) {

        keyboard_move_left();

        show_search_keyboard();

    }

    else if (
        input & INPUT_RIGHT
    ) {

        keyboard_move_right();

        show_search_keyboard();
    }


    if (
        input & INPUT_SELECT
    ) {

        keyboard_select();

        if (
            screen == 5
        )
            show_search_keyboard();
    }


    if (
        input & INPUT_BACK
    ) {

        keyboard_backspace();

        show_search_keyboard();
    }
}


void start_search() {

    search_mode_selection =
        search_mode;

    screen = 8;

    show_search_mode_menu();
}


void show_loading_screen(
    const char *message,
    int progress
) {

    const int bar_width = 30;

    int filled;
    int empty;


    if (
        progress < 0
    )
        progress = 0;

    if (
        progress > 100
    )
        progress = 100;


    filled =
        (progress * bar_width) / 100;

    empty =
        bar_width - filled;


    printf(
        "\x1b[2J\x1b[H"
    );


    printf("\n");


    print_centered(
        "=================================================="
    );

    printf("\n");


    print_centered(
        "MARCViiew"
    );

    print_centered(
        "Wii Library System"
    );

    printf("\n");


    print_centered(
        "--------------------------------------------------"
    );

    printf("\n");


    {
        char bar_line[40];

        int position = 0;


        bar_line[position++] = '+';


        for (
            int i = 0;
            i < filled;
            i++
        )
            bar_line[position++] = '#';


        for (
            int i = 0;
            i < empty;
            i++
        )
            bar_line[position++] = '-';


        bar_line[position++] = '+';

        bar_line[position] = '\0';


        print_centered(
            bar_line
        );
    }


    printf("\n");


    print_centered(
        message
    );

    printf("\n");


    {
        char percentage[20];

        snprintf(
            percentage,
            sizeof(percentage),
            "%3d%%",
            progress
        );

        print_centered(
            percentage
        );
    }


    printf("\n");


    print_centered(
        "--------------------------------------------------"
    );


    if (
        game_count > 0
    ) {

        printf("\n");

        {
            char game_count_line[50];

            snprintf(
                game_count_line,
                sizeof(game_count_line),
                "Games found: %d",
                game_count
            );

            print_centered(
                game_count_line
            );
        }
    }


    printf("\n");


    print_centered(
        "=================================================="
    );


    fflush(
        stdout
    );
}


/*
    Application entry point.
*/
int main(void)
{
    VIDEO_Init();

    WPAD_Init();

    fatInitDefault();


    rmode =
        VIDEO_GetPreferredMode(
            NULL
        );


    xfb =
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


    VIDEO_Configure(
        rmode
    );


    VIDEO_SetNextFramebuffer(
        xfb
    );

    VIDEO_SetBlack(
        false
    );

    VIDEO_Flush();

    VIDEO_WaitVSync();


    if (
        rmode->viTVMode &
        VI_NON_INTERLACE
    )
        VIDEO_WaitVSync();


    printf(
        "\x1b[2J\x1b[H"
    );

    printf("\n");


    print_centered(
        "=================================================="
    );

    printf("\n");


    print_centered(
        "MARCViiew"
    );

    print_centered(
        "Wii Library System"
    );

    printf("\n");


    print_centered(
        "--------------------------------------------------"
    );

    printf("\n");


    print_centered(
        "Press A to continue"
    );

    printf("\n");


    print_centered(
        "--------------------------------------------------"
    );

    printf("\n");


    print_centered(
        "HOME = Exit"
    );

    printf("\n");


    print_centered(
        "=================================================="
    );


    while (
        SYS_MainLoop()
    ) {

        WPAD_ScanPads();

        u32 splash_input =
            get_input();


        if (
            splash_input & INPUT_SELECT
        )
            break;


        if (
            splash_input & INPUT_HOME
        )
            exit(0);


        VIDEO_WaitVSync();
    }


    show_loading_screen(
        "Initialising system...",
        10
    );

    usleep(
        300000
    );


    show_loading_screen(
        "Scanning Wii games...",
        40
    );

    scan_catalogue();


    show_loading_screen(
        "Loading imported MARC records...",
        65
    );

    scan_imported_records();


    show_loading_screen(
        "Loading game metadata...",
        80
    );

    usleep(
        200000
    );


    show_loading_screen(
        "Preparing catalogue...",
        90
    );

    usleep(
        200000
    );


    show_loading_screen(
        "Loading complete!",
        100
    );

    usleep(
        500000
    );


    screen = 1;

    show_main_menu();


    while (
        SYS_MainLoop()
    ) {

        WPAD_ScanPads();

        u32 input =
            get_input();


        if (
            input & INPUT_HOME
        )
            exit(0);


        /*
            Search keyboard screen.
        */
        if (
            screen == 5
        ) {

            handle_search_keyboard(
                input
            );

            VIDEO_WaitVSync();

            continue;
        }


        /*
            Search mode selection screen.
        */
        if (
            screen == 8
        ) {

            if (
                input & INPUT_UP
            ) {

                if (
                    search_mode_selection > 0
                ) {

                    search_mode_selection--;

                    show_search_mode_menu();
                }
            }


            if (
                input & INPUT_DOWN
            ) {

                if (
                    search_mode_selection < 1
                ) {

                    search_mode_selection++;

                    show_search_mode_menu();
                }
            }


            if (
                input & INPUT_SELECT
            ) {

                search_mode =
                    search_mode_selection;

                marc_record_from_search = 0;

                reset_keyboard();

                screen = 5;

                show_search_keyboard();
            }


            if (
                input & INPUT_BACK
            ) {

                screen = 1;

                show_main_menu();
            }


            if (
                input & INPUT_PLUS
            ) {

                screen = 1;

                show_main_menu();
            }


            VIDEO_WaitVSync();

            continue;
        }


        /*
            NAND Root Navigation.
        */
        if (screen == 15)
        {
            if (input & INPUT_UP)
            {
                if (nand_selection > 0)
                {
                    nand_selection--;

                    if (nand_selection < nand_scroll)
                        nand_scroll--;

                    show_nand_browser();
                }
            }

            if (input & INPUT_DOWN)
            {
                if (nand_selection <
                    (int)nand_entry_count - 1)
                {
                    nand_selection++;

                    if (nand_selection >=
                        nand_scroll + GAME_LIST_VISIBLE_ITEMS)
                        nand_scroll++;

                    show_nand_browser();
                }
            }

            if (input & INPUT_SELECT)
            {
                if (nand_entry_count > 0)
                {
                    if (nand_entries[nand_selection].type ==
                        NAND_ENTRY_DIRECTORY)
                    {
                        nand_enter_selected();
                        show_nand_browser();
                    }
                    else if (nand_entries[nand_selection].type ==
                             NAND_ENTRY_FILE)
                    {
                        screen = 16;
                        show_nand_file_info();
                    }
                }
            }

            if (input & INPUT_BACK)
            {
                if (!nand_go_parent())
                {
                    screen = 1;
                    show_main_menu();
                }
                else
                {
                    show_nand_browser();
                }
            }

            if (input & INPUT_PLUS)
            {
                screen = 1;
                show_main_menu();
            }

            VIDEO_WaitVSync();
            continue;
        }

        /*
            NAND File Information.
        */
        if (screen == 16)
        {
            if (input & INPUT_BACK)
            {
                screen = 15;
                show_nand_browser();
            }

            if (input & INPUT_PLUS)
            {
                screen = 1;
                show_main_menu();
            }

            VIDEO_WaitVSync();
            continue;
        }

        /*
            Imported Records list.
        */
        if (screen == 13)
        {
            if (input & INPUT_UP)
            {
                if (imported_selection > 0)
                {
                    imported_selection--;
                    show_imported_menu();
                }
            }

            if (input & INPUT_DOWN)
            {
                if (imported_selection < imported_record_count - 1)
                {
                    imported_selection++;
                    show_imported_menu();
                }
            }

            if (input & INPUT_SELECT)
            {
                if (imported_record_count > 0)
                {
                    /*
                     * Release any record from another viewer before
                     * loading the selected imported .mrc.
                     */
                    if (loaded_marc_record != NULL)
                    {
                        marc_record_free(loaded_marc_record);
                        loaded_marc_record = NULL;
                    }

                    imported_scroll = 0;

                    show_loading_screen(
                        "Loading imported MARC record...",
                        50
                    );

                    screen = 14;
                    show_imported_record();
                }
            }

            if (input & INPUT_BACK)
            {
                screen = 1;
                show_main_menu();
            }

            if (input & INPUT_PLUS)
            {
                screen = 1;
                show_main_menu();
            }

            VIDEO_WaitVSync();
            continue;
        }

        /*
            Imported MARC record viewer.
        */
        if (screen == 14)
        {
            if (input & INPUT_UP)
            {
                imported_scroll--;
                show_imported_record();
            }

            if (input & INPUT_DOWN)
            {
                imported_scroll++;
                show_imported_record();
            }

            if (input & INPUT_BACK)
            {
                screen = 13;

                /*
                 * Refresh the imported-record index when returning
                 * from a record viewer so newly added .mrc files are
                 * visible immediately.
                 */
                scan_imported_records();

                show_imported_menu();
            }

            if (input & INPUT_PLUS)
            {
                screen = 1;
                show_main_menu();
            }

            VIDEO_WaitVSync();
            continue;
        }

        /*
            Settings screen.
        */
        if (
            screen == 9
        ) {

            if (
                input & INPUT_UP
            ) {

                if (
                    settings_selection > 0
                ) {

                    settings_selection--;

                    show_settings_menu();
                }
            }


            if (
                input & INPUT_DOWN
            ) {

                if (
                    settings_selection < 3
                ) {

                    settings_selection++;

                    show_settings_menu();
                }
            }


            if (
                input & INPUT_SELECT
            ) {

                if (
                    settings_selection == 0
                ) {

                    settings_reload_storage();

                }

                else if (
                    settings_selection == 1
                ) {

                    settings_reload_nand();

                }

                else if (
                    settings_selection == 2
                ) {

                    settings_reload_databases();

                }

                else if (
                    settings_selection == 3
                ) {

                    screen = 10;

                    show_credits();
                }
            }


            if (
                input & INPUT_BACK
            ) {

                screen = 1;

                show_main_menu();
            }


            if (
                input & INPUT_PLUS
            ) {

                screen = 1;

                show_main_menu();
            }


            VIDEO_WaitVSync();

            continue;
        }


        /*
            Credits screen.
        */
        if (
            screen == 10
        ) {

            if (
                input & INPUT_BACK
            ) {

                screen = 9;

                show_settings_menu();
            }


            if (
                input & INPUT_PLUS
            ) {

                screen = 1;

                show_main_menu();
            }


            VIDEO_WaitVSync();

            continue;
        }


        /*
            Encode MARC menu.
        */
        if (
            screen == 11
        ) {

            if (
                input & INPUT_UP
            ) {

                if (
                    encode_selection > 0
                ) {

                    encode_selection--;

                    show_encode_marc();
                }
            }


            if (
                input & INPUT_DOWN
            ) {

                if (
                    encode_selection < 2
                ) {

                    encode_selection++;

                    show_encode_marc();
                }
            }


            if (
                input & INPUT_SELECT
            ) {

                if (
                    encode_selection == 0
                ) {

                    encode_entire_database();

                }

                else if (
                    encode_selection == 1
                ) {

                    encode_game_selection = 0;

                    screen = 12;

                    show_encode_game_menu();

                }

                else if (
                    encode_selection == 2
                ) {

                    screen = 1;

                    show_main_menu();
                }
            }


            if (
                input & INPUT_BACK
            ) {

                screen = 1;

                show_main_menu();
            }


            if (
                input & INPUT_PLUS
            ) {

                screen = 1;

                show_main_menu();
            }


            VIDEO_WaitVSync();

            continue;
        }


        /*
            Select Game for MARC export.
        */
        if (
            screen == 12
        ) {

            if (
                input & INPUT_UP
            ) {

                if (
                    encode_game_selection > 0
                ) {

                    encode_game_selection--;

                    show_encode_game_menu();
                }
            }


            if (
                input & INPUT_DOWN
            ) {

                if (
                    encode_game_selection <
                    game_count - 1
                ) {

                    encode_game_selection++;

                    show_encode_game_menu();
                }
            }


            if (
                input & INPUT_SELECT
            ) {

                encode_selected_game();
            }


            if (
                input & INPUT_BACK
            ) {

                screen = 11;

                show_encode_marc();
            }


            if (
                input & INPUT_PLUS
            ) {

                screen = 1;

                show_main_menu();
            }


            VIDEO_WaitVSync();

            continue;
        }


        /*
            A button handling.
        */
        if (
            input & INPUT_SELECT
        ) {

            if (
                screen == 1
            ) {

                if (
                    menu_selection == 0
                ) {

                    screen = 2;

                    catalogue_selection = 0;
                    catalogue_scroll = 0;

                    show_catalogue();

                }

                else if (
                    menu_selection == 1
                ) {

                    start_search();

                }

                else if (
                    menu_selection == 2
                ) {

                    screen = 4;

                    marc_selection = 0;
                    marc_menu_scroll = 0;

                    marc_record_from_search = 0;

                    show_marc_menu();

                }

                else if (
                    menu_selection == 3
                ) {

                    screen = 11;

                    encode_selection = 0;

                    encode_game_selection = 0;

                    encode_status[0] = '\0';

                    show_encode_marc();

                }

                else if (
                    menu_selection == 4
                ) {

                    screen = 13;
                    imported_selection = 0;
                    imported_scroll = 0;

                    /*
                     * Refresh the imported-record index whenever the
                     * Imported Records screen is entered. This allows
                     * newly copied .mrc files in sd:/marcviiew_import
                     * to appear without restarting MarcViiew.
                     */
                    scan_imported_records();

                    show_imported_menu();

                }

                else if (
                    menu_selection == 5
                ) {

                    nand_selection = 0;
                    nand_scroll = 0;
                    nand_status[0] = '\0';

                    if (!nand_initialized)
                    {
                        int patch_result = nand_ios_enable_access();

                        if (patch_result > 0)
                        {
                            char patch_status[96];

                            nand_ios_get_patch_status(
                                patch_status,
                                sizeof(patch_status)
                            );

                            snprintf(
                                nand_status,
                                sizeof(nand_status),
                                "IOS patches: %s",
                                patch_status
                            );
                        }
                        else if (patch_result == 0)
                        {
                            char patch_status[96];

                            nand_ios_get_patch_status(
                                patch_status,
                                sizeof(patch_status)
                            );

                            snprintf(
                                nand_status,
                                sizeof(nand_status),
                                "IOS patch signatures not found: %s",
                                patch_status
                            );
                        }
                        else
                        {
                            snprintf(
                                nand_status,
                                sizeof(nand_status),
                                "AHBPROT unavailable; protected entries may be inaccessible."
                            );
                        }

                        s32 result = ISFS_Initialize();

                        if (result != ISFS_OK)
                        {
                            snprintf(
                                nand_status,
                                sizeof(nand_status),
                                "ISFS initialization failed (%d).",
                                (int)result
                            );
                        }
                        else
                        {
                            nand_initialized = 1;
                        }
                    }

                    if (nand_initialized)
                    {
                        strcpy(
                            nand_current_path,
                            "/"
                        );

                        if (!nand_load_directory(nand_current_path))
                        {
                            snprintf(
                                nand_status,
                                sizeof(nand_status),
                                "Could not read NAND root directory."
                            );
                        }
                    }

                    screen = 15;

                    show_nand_browser();
                }

                else if (
                    menu_selection == 6
                ) {

                    screen = 9;

                    settings_selection = 0;

                    settings_status[0] = '\0';

                    show_settings_menu();
                }
            }


            else if (
                screen == 2
            ) {

                if (
                    game_count > 0
                ) {

                    screen = 3;

                    info_scroll = 0;

                    show_game_information();
                }
            }


            else if (
                screen == 4
            ) {

                if (
                    game_count > 0
                ) {

                    screen = 7;

                    marc_scroll = 0;

                    marc_record_from_search = 0;

                    show_marc_record();
                }
            }


            else if (
                screen == 6
            ) {

                if (
                    search_result_count > 0
                ) {

                    int selected_game =
                        search_results[
                            search_selection
                        ];


                    if (
                        search_mode ==
                        SEARCH_MODE_MARC21
                    ) {

                        marc_selection =
                            selected_game;

                        marc_scroll = 0;

                        marc_record_from_search = 1;

                        screen = 7;

                        show_marc_record();

                    } else {

                        catalogue_selection =
                            selected_game;

                        info_scroll = 0;

                        screen = 3;

                        show_game_information();
                    }
                }
            }
        }


        /*
            B button handling.
        */
        if (
            input & INPUT_BACK
        ) {

            if (
                screen == 3
            ) {

                if (
                    search_result_count > 0
                ) {

                    screen = 6;

                    show_search_results();

                } else {

                    screen = 2;

                    show_catalogue();
                }

            }

            else if (
                screen == 4
            ) {

                screen = 1;

                show_main_menu();

            }

            else if (
                screen == 7
            ) {

                if (
                    marc_record_from_search
                ) {

                    screen = 6;

                    show_search_results();

                } else {

                    screen = 4;

                    show_marc_menu();
                }

            }

            else if (
                screen == 2
            ) {

                screen = 1;

                show_main_menu();

            }

            else if (
                screen == 13
            ) {

                screen = 1;

                show_main_menu();

            }

            else if (
                screen == 14
            ) {

                screen = 13;

                show_imported_menu();

            }

            else if (
                screen == 6
            ) {

                screen = 5;

                show_search_keyboard();
            }
        }


        /*
            PLUS = Main Menu.
        */
        if (
            input & INPUT_PLUS
        ) {

            if (
                screen != 0 &&
                screen != 1
            ) {

                screen = 1;

                search_result_count = 0;

                marc_record_from_search = 0;

                show_main_menu();
            }
        }


        /*
            DOWN navigation.
        */
        if (
            input & INPUT_DOWN
        ) {

            if (
                screen == 1
            ) {

                if (
                    menu_selection < 6
                ) {

                    menu_selection++;

                    show_main_menu();
                }

            }


            else if (
                screen == 2
            ) {

                if (
                    catalogue_selection <
                    game_count - 1
                ) {

                    catalogue_selection++;

                    if (
                        catalogue_selection >=
                        catalogue_scroll +
                        GAME_LIST_VISIBLE_ITEMS
                    )
                        catalogue_scroll++;

                    show_catalogue();
                }

            }


            else if (
                screen == 3
            ) {

                info_scroll++;

                show_game_information();

            }


            else if (
                screen == 4
            ) {

                if (
                    marc_selection <
                    game_count - 1
                ) {

                    marc_selection++;

                    if (
                        marc_selection >=
                        marc_menu_scroll +
                        GAME_LIST_VISIBLE_ITEMS
                    )
                        marc_menu_scroll++;

                    show_marc_menu();
                }

            }


            else if (
                screen == 7
            ) {

                marc_scroll++;

                show_marc_record();

            }


            else if (
                screen == 6
            ) {

                if (
                    search_selection <
                    search_result_count - 1
                ) {

                    search_selection++;

                    show_search_results();
                }
            }
        }


        /*
            UP navigation.
        */
        if (
            input & INPUT_UP
        ) {

            if (
                screen == 1
            ) {

                if (
                    menu_selection > 0
                ) {

                    menu_selection--;

                    show_main_menu();
                }

            }


            else if (
                screen == 2
            ) {

                if (
                    catalogue_selection > 0
                ) {

                    catalogue_selection--;

                    if (
                        catalogue_selection <
                        catalogue_scroll
                    )
                        catalogue_scroll--;

                    show_catalogue();
                }

            }


            else if (
                screen == 3
            ) {

                info_scroll--;

                show_game_information();

            }


            else if (
                screen == 4
            ) {

                if (
                    marc_selection > 0
                ) {

                    marc_selection--;

                    if (
                        marc_selection <
                        marc_menu_scroll
                    )
                        marc_menu_scroll--;

                    show_marc_menu();
                }

            }


            else if (
                screen == 7
            ) {

                marc_scroll--;

                show_marc_record();

            }


            else if (
                screen == 6
            ) {

                if (
                    search_selection > 0
                ) {

                    search_selection--;

                    show_search_results();
                }
            }
        }


        VIDEO_WaitVSync();
    }


    if (nand_initialized)
    {
        nand_restore_title_uid();
        ISFS_Deinitialize();
    }

    return 0;
}