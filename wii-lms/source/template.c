#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>

#include <gccore.h>
#include <wiiuse/wpad.h>
#include <fat.h>

#define MAX_GAMES 100
#define DATABASE_LINE_LENGTH 4096
#define MAX_INFO_LINES 100
#define INFO_LINE_LENGTH 100

static void *xfb = NULL;
static GXRModeObj *rmode = NULL;

static int menu_selection = 0;
static int screen = 0;

static int catalogue_selection = 0;
static int game_count = 0;

static int info_scroll = 0;


/*
 * Information about a Wii game.
 */
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
    char synopsis[3000];
} Game;

static Game games[MAX_GAMES];


/*
 * Extract the Game ID from a folder name.
 *
 * Example:
 *
 * Animal Crossing City Folk [RUUE01]
 *
 * becomes:
 *
 * RUUE01
 */
void extract_game_id(const char *name, char *id) {

    const char *start = strchr(name, '[');

    if (start != NULL && strlen(start) >= 8) {

        strncpy(id, start + 1, 6);

        id[6] = '\0';

    } else {

        strcpy(id, "??????");
    }
}


/*
 * Extract the title from a folder name.
 *
 * Example:
 *
 * Animal Crossing City Folk [RUUE01]
 *
 * becomes:
 *
 * Animal Crossing City Folk
 */
void extract_game_title(const char *name, char *title) {

    const char *start = strchr(name, '[');

    if (start != NULL) {

        int length = start - name;

        if (length >= 100)
            length = 99;

        strncpy(title, name, length);

        title[length] = '\0';

        /*
         * Remove trailing spaces.
         */
        while (length > 0 && title[length - 1] == ' ') {

            title[length - 1] = '\0';

            length--;
        }

    } else {

        strncpy(title, name, 99);

        title[99] = '\0';
    }
}


/*
 * Set the basic metadata for a game.
 *
 * These values are used until the GameTDB
 * database provides the real metadata.
 */
void setup_game_metadata(Game *game) {

    strcpy(game->platform, "Nintendo Wii");
    strcpy(game->region, "Unknown");
    strcpy(game->release_date, "Not yet catalogued");
    strcpy(game->publisher, "Not yet catalogued");
    strcpy(game->developer, "Not yet catalogued");
    strcpy(game->genre, "Not yet catalogued");
    strcpy(game->series, "Not yet catalogued");
    strcpy(game->synopsis, "Not yet catalogued");
}


/*
 * Copy the value after a database field.
 *
 * Example:
 *
 * TITLE=Animal Crossing
 *
 * becomes:
 *
 * Animal Crossing
 */
void copy_database_value(
    const char *line,
    const char *field,
    char *destination,
    int destination_size
) {

    int field_length = strlen(field);

    if (strncmp(line, field, field_length) == 0) {

        int value_length = strlen(line + field_length);

        if (value_length >= destination_size)
            value_length = destination_size - 1;

        memcpy(
            destination,
            line + field_length,
            value_length
        );

        destination[value_length] = '\0';
    }
}


/*
 * Load metadata for a game from the MarcViiew database.
 *
 * The database is stored on the SD card:
 *
 * sd:/marcviiew_games.txt
 *
 * The Game ID is used to find the correct record.
 */
int load_game_metadata(Game *game) {

    FILE *database;

    char line[DATABASE_LINE_LENGTH];

    int found = 0;

    database = fopen("sd:/marcviiew_games.txt", "r");

    if (database == NULL) {

        return 0;
    }


    while (fgets(line, sizeof(line), database) != NULL) {

        /*
         * Remove the newline at the end of the line.
         */
        line[strcspn(line, "\r\n")] = '\0';


        /*
         * Look for an ID line.
         *
         * Example:
         *
         * ID=RUUE01
         */
        if (strncmp(line, "ID=", 3) == 0) {

            if (strcmp(line + 3, game->id) == 0) {

                found = 1;

                /*
                 * Read the rest of this game record.
                 */
                while (fgets(line, sizeof(line), database) != NULL) {

                    line[strcspn(line, "\r\n")] = '\0';

                    /*
                     * A blank line marks the end
                     * of the game record.
                     */
                    if (strlen(line) == 0)
                        break;

                    copy_database_value(
                        line,
                        "TITLE=",
                        game->title,
                        sizeof(game->title)
                    );

                    copy_database_value(
                        line,
                        "REGION=",
                        game->region,
                        sizeof(game->region)
                    );

                    copy_database_value(
                        line,
                        "DEVELOPER=",
                        game->developer,
                        sizeof(game->developer)
                    );

                    copy_database_value(
                        line,
                        "PUBLISHER=",
                        game->publisher,
                        sizeof(game->publisher)
                    );

                    copy_database_value(
                        line,
                        "RELEASE_DATE=",
                        game->release_date,
                        sizeof(game->release_date)
                    );

                    copy_database_value(
                        line,
                        "SYNOPSIS=",
                        game->synopsis,
                        sizeof(game->synopsis)
                    );
                }

                break;
            }
        }
    }

    fclose(database);

    return found;
}


/*
 * Scan the USB WBFS directory and build our catalogue.
 */
void scan_catalogue() {

    DIR *dir;

    struct dirent *entry;

    game_count = 0;

    dir = opendir("usb:/wbfs");

    if (dir == NULL) {

        return;
    }

    while ((entry = readdir(dir)) != NULL) {

        /*
         * Ignore . and ..
         */
        if (entry->d_name[0] == '.')
            continue;

        /*
         * Stop if our catalogue is full.
         */
        if (game_count >= MAX_GAMES)
            break;

        /*
         * Create a new Game record.
         */
        extract_game_title(
            entry->d_name,
            games[game_count].title
        );

        extract_game_id(
            entry->d_name,
            games[game_count].id
        );

        setup_game_metadata(
            &games[game_count]
        );

        /*
         * Try to find this game's metadata
         * in the MarcViiew database.
         */
        load_game_metadata(
            &games[game_count]
        );

        game_count++;
    }

    closedir(dir);
}


/*
 * Add a line to the game information page.
 */
void add_info_line(
    char info_lines[][INFO_LINE_LENGTH],
    int *line_count,
    const char *text
) {

    if (*line_count >= MAX_INFO_LINES)
        return;

    strncpy(
        info_lines[*line_count],
        text,
        INFO_LINE_LENGTH - 1
    );

    info_lines[*line_count][INFO_LINE_LENGTH - 1] = '\0';

    (*line_count)++;
}


/*
 * Add a label and value to the information page.
 */
void add_info_field(
    char info_lines[][INFO_LINE_LENGTH],
    int *line_count,
    const char *label,
    const char *value
) {

    char line[INFO_LINE_LENGTH];

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


/*
 * Add a long piece of text while wrapping it
 * to fit on the Wii screen.
 */
void add_wrapped_text(
    char info_lines[][INFO_LINE_LENGTH],
    int *line_count,
    const char *text
) {

    const int wrap_width = 70;

    int length = strlen(text);
    int position = 0;

    while (position < length && *line_count < MAX_INFO_LINES) {

        int remaining = length - position;
        int line_length = remaining;

        if (line_length > wrap_width)
            line_length = wrap_width;

        /*
         * Try to break at a space instead of
         * cutting a word in half.
         */
        if (position + line_length < length) {

            int break_position = line_length;

            while (
                break_position > 0 &&
                text[position + break_position] != ' '
            ) {

                break_position--;
            }

            if (break_position > 0)
                line_length = break_position;
        }

        strncpy(
            info_lines[*line_count],
            text + position,
            line_length
        );

        info_lines[*line_count][line_length] = '\0';

        (*line_count)++;

        position += line_length;

        /*
         * Skip the space between wrapped lines.
         */
        while (
            position < length &&
            text[position] == ' '
        ) {

            position++;
        }
    }
}


/*
 * Display information about the selected game.
 *
 * The page can be scrolled with UP and DOWN.
 */
void show_game_information() {

    char info_lines[MAX_INFO_LINES][INFO_LINE_LENGTH];

    int line_count = 0;

    Game *game = &games[catalogue_selection];


    /*
     * Build the information page.
     */
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


    /*
     * Synopsis.
     */
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


    /*
     * Display the page.
     */
    printf("\x1b[2J\x1b[H");

    printf("================================\n");
    printf("       Game Information\n");
    printf("================================\n\n");


    /*
     * Display approximately 17 lines at a time.
     */
    int visible_lines = 17;

    int max_scroll = line_count - visible_lines;

    if (max_scroll < 0)
        max_scroll = 0;

    if (info_scroll > max_scroll)
        info_scroll = max_scroll;

    if (info_scroll < 0)
        info_scroll = 0;


    for (
        int i = info_scroll;
        i < info_scroll + visible_lines && i < line_count;
        i++
    ) {

        printf("%s\n", info_lines[i]);
    }


    printf("\n");

    if (max_scroll > 0) {

        printf(
            "UP / DOWN = Scroll  (%d/%d)\n",
            info_scroll + 1,
            max_scroll + 1
        );

    } else {

        printf("UP / DOWN = Scroll\n");
    }

    printf("B = Back\n");
}


/*
 * Display the main menu.
 */
void show_main_menu() {

    printf("\x1b[2J\x1b[H");

    printf("================================\n");
    printf("          MarcViiew\n");
    printf("  Wii Library Management System\n");
    printf("================================\n\n");

    if (menu_selection == 0)
        printf("> Catalogue\n");
    else
        printf("  Catalogue\n");

    if (menu_selection == 1)
        printf("> Search\n");
    else
        printf("  Search\n");

    if (menu_selection == 2)
        printf("> MARC Records\n");
    else
        printf("  MARC Records\n");

    if (menu_selection == 3)
        printf("> Settings\n");
    else
        printf("  Settings\n");

    printf("\n");

    printf("UP / DOWN = Move    A = Select\n");
    printf("HOME = Exit\n");
}


/*
 * Display the catalogue.
 */
void show_catalogue() {

    printf("\x1b[2J\x1b[H");

    printf("================================\n");
    printf("          Catalogue\n");
    printf("================================\n\n");

    /*
     * Scan the USB every time we enter
     * the catalogue.
     */
    scan_catalogue();

    if (game_count == 0) {

        printf("No games found.\n\n");

        printf("B = Back\n");

        return;
    }

    /*
     * Display the games.
     */
    for (int i = 0; i < game_count; i++) {

        if (i == catalogue_selection)
            printf("> ");
        else
            printf("  ");

        printf("%s\n", games[i].title);
    }

    printf("\n");

    printf("%d game(s) found.\n\n", game_count);

    printf("UP / DOWN = Move\n");
    printf("A = View Game\n");
    printf("B = Back\n");
}


//---------------------------------------------------------------------------------
int main(int argc, char **argv) {
//---------------------------------------------------------------------------------

    // Initialise the video system
    VIDEO_Init();

    // Initialise the controllers
    WPAD_Init();

    // Initialise the Wii filesystem
    fatInitDefault();

    // Obtain the preferred video mode
    rmode = VIDEO_GetPreferredMode(NULL);

    // Allocate memory for the display
    xfb = MEM_K0_TO_K1(
        SYS_AllocateFramebuffer(rmode)
    );

    // Initialise the console
    console_init(
        xfb,
        20,
        20,
        rmode->fbWidth,
        rmode->xfbHeight,
        rmode->fbWidth * VI_DISPLAY_PIX_SZ
    );

    // Set up the video registers
    VIDEO_Configure(rmode);

    // Tell the video hardware where our display memory is
    VIDEO_SetNextFramebuffer(xfb);

    // Make the display visible
    VIDEO_SetBlack(false);

    // Flush the video register changes
    VIDEO_Flush();

    // Wait for video setup to complete
    VIDEO_WaitVSync();

    if (rmode->viTVMode & VI_NON_INTERLACE)
        VIDEO_WaitVSync();


    // Show the title screen
    printf("\x1b[2J\x1b[H");

    printf("================================\n");
    printf("          MarcViiew\n");
    printf("  Wii Library Management System\n");
    printf("================================\n\n");

    printf("        Press A to continue\n");


    while (SYS_MainLoop()) {

        // Read the controller
        WPAD_ScanPads();

        // Get buttons pressed during this frame
        u32 pressed = WPAD_ButtonsDown(0);


        // HOME exits MarcViiew
        if (pressed & WPAD_BUTTON_HOME) {

            exit(0);
        }


        /*
         * A button
         */
        if (pressed & WPAD_BUTTON_A) {


            /*
             * TITLE SCREEN
             */
            if (screen == 0) {

                printf("\x1b[2J\x1b[H");

                printf("Welcome to MarcViiew!\n");

                // Show welcome message for 2 seconds
                usleep(2000000);

                // Switch to the main menu
                screen = 1;

                show_main_menu();
            }


            /*
             * MAIN MENU
             */
            else if (screen == 1) {

                /*
                 * Catalogue selected
                 */
                if (menu_selection == 0) {

                    screen = 2;

                    catalogue_selection = 0;

                    show_catalogue();
                }
            }


            /*
             * CATALOGUE
             */
            else if (screen == 2) {

                /*
                 * Open selected game
                 */
                if (game_count > 0) {

                    screen = 3;

                    info_scroll = 0;

                    show_game_information();
                }
            }
        }


        /*
         * B button
         */
        if (pressed & WPAD_BUTTON_B) {


            /*
             * Game Information -> Catalogue
             */
            if (screen == 3) {

                screen = 2;

                show_catalogue();
            }


            /*
             * Catalogue -> Main Menu
             */
            else if (screen == 2) {

                screen = 1;

                show_main_menu();
            }
        }


        /*
         * DOWN button
         */
        if (pressed & WPAD_BUTTON_DOWN) {


            /*
             * Main menu
             */
            if (screen == 1) {

                if (menu_selection < 3) {

                    menu_selection++;

                    show_main_menu();
                }
            }


            /*
             * Catalogue
             */
            else if (screen == 2) {

                if (catalogue_selection < game_count - 1) {

                    catalogue_selection++;

                    show_catalogue();
                }
            }


            /*
             * Game information
             */
            else if (screen == 3) {

                info_scroll++;

                show_game_information();
            }
        }


        /*
         * UP button
         */
        if (pressed & WPAD_BUTTON_UP) {


            /*
             * Main menu
             */
            if (screen == 1) {

                if (menu_selection > 0) {

                    menu_selection--;

                    show_main_menu();
                }
            }


            /*
             * Catalogue
             */
            else if (screen == 2) {

                if (catalogue_selection > 0) {

                    catalogue_selection--;

                    show_catalogue();
                }
            }


            /*
             * Game information
             */
            else if (screen == 3) {

                info_scroll--;

                show_game_information();
            }
        }


        // Wait for the next frame
        VIDEO_WaitVSync();
    }

    return 0;
}