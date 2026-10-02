import os
import xml.etree.ElementTree as ET


program_folder = os.path.dirname(
    os.path.abspath(__file__)
)

xml_file = os.path.join(
    program_folder,
    "wiitdb.xml"
)

database_file = os.path.join(
    program_folder,
    "marcviiew_games.txt"
)


tree = ET.parse(xml_file)
root = tree.getroot()

games = root.findall("game")

print(
    "Games found:",
    len(games)
)

print()


def get_game_data(game):

    game_data = {}

    # ----------------------------------------------------------
    # Basic GameTDB information
    # ----------------------------------------------------------

    game_data["id"] = game.findtext(
        "id",
        default=""
    )

    game_data["region"] = game.findtext(
        "region",
        default=""
    )

    game_data["developer"] = game.findtext(
        "developer",
        default=""
    )

    game_data["publisher"] = game.findtext(
        "publisher",
        default=""
    )

    # ----------------------------------------------------------
    # GameTDB type
    # ----------------------------------------------------------

    game_data["type"] = game.findtext(
        "type",
        default=""
    ).strip()

    # ----------------------------------------------------------
    # Distribution
    #
    # GameTDB type is preferred for known digital releases.
    # ROM filenames provide a secondary physical/digital clue.
    # Unknown cases are left blank rather than guessed.
    # ----------------------------------------------------------

    type_lower = game_data["type"].lower()

    if type_lower in (
        "wiiware",
        "virtual console"
    ):

        game_data["distribution"] = "DIGITAL"

    else:

        rom = game.find("rom")

        if rom is not None:

            rom_name = rom.get(
                "name",
                ""
            ).lower()

            if rom_name.endswith(".wad"):

                game_data["distribution"] = "DIGITAL"

            elif rom_name.endswith((
                ".iso",
                ".wbfs",
                ".ciso"
            )):

                game_data["distribution"] = "PHYSICAL"

            else:

                game_data["distribution"] = ""

        else:

            game_data["distribution"] = ""

    # ----------------------------------------------------------
    # GameTDB genre
    # ----------------------------------------------------------

    genre_element = game.find("genre")

    if genre_element is not None:

        game_data["genre"] = (
            genre_element.text or ""
        ).strip()

    else:

        game_data["genre"] = ""

    # ----------------------------------------------------------
    # English title and synopsis
    # ----------------------------------------------------------

    english_locale = game.find(
        "locale[@lang='EN']"
    )

    if english_locale is not None:

        game_data["title"] = english_locale.findtext(
            "title",
            default=""
        )

        game_data["synopsis"] = english_locale.findtext(
            "synopsis",
            default=""
        )

    else:

        game_data["title"] = ""
        game_data["synopsis"] = ""

    # ----------------------------------------------------------
    # Release date
    # ----------------------------------------------------------

    date = game.find("date")

    if date is not None:

        year = date.get(
            "year",
            ""
        )

        month = date.get(
            "month",
            ""
        )

        day = date.get(
            "day",
            ""
        )

        game_data["release_date"] = (
            year
            + "-"
            + month
            + "-"
            + day
        )

    else:

        game_data["release_date"] = ""

    return game_data


all_games = []

for game in games:

    game_data = get_game_data(
        game
    )

    all_games.append(
        game_data
    )


print(
    "Games processed:",
    len(all_games)
)

print()


with open(
    database_file,
    "w",
    encoding="utf-8"
) as database:

    for game in all_games:

        database.write(
            "[GAME]\n"
        )

        database.write(
            "ID="
            + game["id"]
            + "\n"
        )

        database.write(
            "TITLE="
            + game["title"]
            + "\n"
        )

        database.write(
            "REGION="
            + game["region"]
            + "\n"
        )

        database.write(
            "DEVELOPER="
            + game["developer"]
            + "\n"
        )

        database.write(
            "PUBLISHER="
            + game["publisher"]
            + "\n"
        )

        database.write(
            "RELEASE_DATE="
            + game["release_date"]
            + "\n"
        )

        database.write(
            "GENRE="
            + game["genre"]
            + "\n"
        )

        database.write(
            "TYPE="
            + game["type"]
            + "\n"
        )

        database.write(
            "DISTRIBUTION="
            + game["distribution"]
            + "\n"
        )

        database.write(
            "SYNOPSIS="
            + game["synopsis"]
            + "\n"
        )

        database.write(
            "\n"
        )


print(
    "Database created!"
)

print(
    database_file
)
