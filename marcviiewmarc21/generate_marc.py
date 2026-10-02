import os

from wiiext import get_wiiext_term


program_folder = os.path.dirname(
    os.path.abspath(__file__)
)

database_file = os.path.join(
    program_folder,
    "marcviiew_games.txt"
)

marc_file = os.path.join(
    program_folder,
    "marcviiew_marc.txt"
)


def parse_games():

    games = []

    with open(
        database_file,
        "r",
        encoding="utf-8"
    ) as database:

        lines = database.readlines()

    current_game = None

    for line in lines:

        line = line.strip()

        if line == "[GAME]":

            if current_game is not None:
                games.append(current_game)

            current_game = {}

            continue

        if not line:
            continue

        if "=" not in line:
            continue

        field, value = line.split(
            "=",
            1
        )

        if current_game is not None:

            current_game[field] = value

    if current_game is not None:
        games.append(current_game)

    return games


def get_title_indicator(title):

    title_lower = title.lower()

    if title_lower.startswith("the "):
        return 4

    if title_lower.startswith("an "):
        return 3

    if title_lower.startswith("a "):
        return 2

    return 0


def get_wiiext_genres(raw_genre):

    if not raw_genre:
        return []

    raw_terms = raw_genre.split(",")

    preferred_terms = []

    for raw_term in raw_terms:

        raw_term = raw_term.strip()

        preferred_term = get_wiiext_term(
            raw_term
        )

        if preferred_term is not None:

            if preferred_term not in preferred_terms:

                preferred_terms.append(
                    preferred_term
                )

    return preferred_terms


games = parse_games()

print(
    "Games found:",
    len(games)
)

print()


with open(
    marc_file,
    "w",
    encoding="utf-8"
) as output:

    for game in games:

        game_id = game.get(
            "ID",
            ""
        )

        title = game.get(
            "TITLE",
            ""
        )

        publisher = game.get(
            "PUBLISHER",
            ""
        )

        release_date = game.get(
            "RELEASE_DATE",
            ""
        )

        synopsis = game.get(
            "SYNOPSIS",
            ""
        )

        raw_genre = game.get(
            "GENRE",
            ""
        )


        # ======================================================
        # RECORD
        # ======================================================

        output.write(
            "[RECORD]\n"
        )

        output.write(
            "GAME_ID="
            + game_id
            + "\n"
        )

        output.write(
            "\n"
        )


        # ======================================================
        # 001 - CONTROL NUMBER
        # ======================================================

        output.write(
            "[001]\n"
        )

        output.write(
            "VALUE="
            + game_id
            + "\n"
        )

        output.write(
            "\n"
        )


        # ======================================================
        # 245 - TITLE
        # ======================================================

        output.write(
            "[245]\n"
        )

        output.write(
            "IND1=0\n"
        )

        output.write(
            "IND2="
            + str(
                get_title_indicator(title)
            )
            + "\n"
        )

        output.write(
            "$a="
            + title
            + "\n"
        )

        output.write(
            "\n"
        )


        # ======================================================
        # 264 - PRODUCTION / PUBLICATION
        # ======================================================

        output.write(
            "[264]\n"
        )

        output.write(
            "IND1=#\n"
        )

        output.write(
            "IND2=1\n"
        )

        output.write(
            "$b="
            + publisher
            + "\n"
        )

        output.write(
            "$c="
            + release_date
            + "\n"
        )

        output.write(
            "\n"
        )


        # ======================================================
        # 300 - PHYSICAL DESCRIPTION
        # ======================================================

        output.write(
            "[300]\n"
        )

        output.write(
            "IND1=#\n"
        )

        output.write(
            "IND2=#\n"
        )

        output.write(
            "$a=1 Wii optical disc\n"
        )

        output.write(
            "\n"
        )


        # ======================================================
        # 500 - GENERAL NOTE
        # ======================================================

        output.write(
            "[500]\n"
        )

        output.write(
            "IND1=#\n"
        )

        output.write(
            "IND2=#\n"
        )

        output.write(
            "$a="
            + synopsis
            + "\n"
        )

        output.write(
            "\n"
        )


        # ======================================================
        # 542 - INFORMATION ABOUT COPYRIGHT
        # ======================================================

        output.write(
            "[542]\n"
        )

        output.write(
            "IND1=#\n"
        )

        output.write(
            "IND2=#\n"
        )

        if release_date:

            release_year = release_date.split(
                "-"
            )[0]

            output.write(
                "$i="
                + release_year
                + "\n"
            )

        if publisher:

            output.write(
                "$k="
                + publisher
                + "\n"
            )

        output.write(
            "$s=GameTDB\n"
        )

        output.write(
            "\n"
        )


        # ======================================================
        # 655 - GENRE / FORM
        # ======================================================

        wiiext_genres = get_wiiext_genres(
            raw_genre
        )

        for genre in wiiext_genres:

            output.write(
                "[655]\n"
            )

            output.write(
                "IND1=#\n"
            )

            output.write(
                "IND2=7\n"
            )

            output.write(
                "$a="
                + genre
                + "\n"
            )

            output.write(
                "$2=wiiext\n"
            )

            output.write(
                "\n"
            )


        output.write(
            "\n"
        )


print(
    "MARC records created!"
)

print(
    marc_file
)