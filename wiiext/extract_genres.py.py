import os
import xml.etree.ElementTree as ET
from collections import Counter

program_folder = os.path.dirname(os.path.abspath(__file__))

xml_file = os.path.join(
    program_folder,
    "wiitdb.xml"
)


tree = ET.parse(xml_file)
root = tree.getroot()

games = root.findall("game")

genres = Counter()


for game in games:

    genre = game.find("genre")

    if genre is None or not genre.text:
        continue

    game_genres = genre.text.split(",")

    for term in game_genres:

        term = term.strip()

        if term:
            genres[term] += 1


print("Games found:", len(games))
print("Unique individual genre terms:", len(genres))
print()

print("GameTDB Wii genre terms:")
print("------------------------")


for genre, count in genres.most_common():

    print(f"{genre}: {count}")