from wiiext import get_wiiext_term


GAME_TDB_TERMS = [
    "action",
    "adventure",
    "sports",
    "racing",
    "platformer",
    "music",
    "party",
    "shooter",
    "puzzle",
    "fighting",
    "simulation",
    "role-playing",
    "karaoke",
    "arcade",
    "rhythm",
    "strategy",
    "dance",
    "kart racing",
    "first-person shooter",
    "health",
    "football",
    "educational",
    "software",
    "board game",
    "beat 'em up",
    "fitness",
    "soccer",
    "golf",
    "3d platformer",
    "trivia",
    "virtual pet",
    "compilation",
    "baseball",
    "demo",
    "basketball",
    "sampler",
    "cards",
    "survival horror",
    "third-person shooter",
    "coaching",
    "hunting",
    "fishing",
    "motorcycle racing",
    "skateboarding",
    "hockey",
    "wrestling",
    "rail shooter",
    "snowboarding",
    "hidden object",
    "boxing",
    "tennis",
    "action rpg",
    "exercise",
    "tactical rpg",
    "flight simulation",
    "multimedia",
    "point-and-click",
    "life simulation",
    "emulator",
    "3d fighting",
    "futuristic racing",
    "cooking",
    "billiards",
    "2d platformer",
    "pinball",
    "bowling",
    "truck racing",
    "drawing",
    "historic",
    "ski",
    "off-road racing",
    "stealth action",
    "fantasy",
    "2d fighting",
    "bike racing",
    "real-time strategy",
    "poker",
    "turn-based strategy",
    "table tennis",
    "shoot 'em up",
    "volleyball",
    "watercraft racing",
    "darts",
    "surfing",
    "chess",
    "sim racing",
    "horror",
    "construction simulation",
    "management simulation",
    "run and gun",
    "interactive fiction",
    "modern day",
    "business simulation",
    "martial arts",
    "sci-fi",
    "cricket",
    "strategy rpg",
    "traditional",
    "wargame",
    "pétanque",
    "roguelike",
    "tower defense",
    "mmorpg",
    "interactive movie",
    "rugby",
    "train simulation"
]


mapped = []
unmapped = []


for term in GAME_TDB_TERMS:

    result = get_wiiext_term(term)

    if result is None:
        unmapped.append(term)
    else:
        mapped.append((term, result))


print("GameTDB terms:", len(GAME_TDB_TERMS))
print("Mapped terms:", len(mapped))
print("Unmapped terms:", len(unmapped))
print()


print("MAPPED TERMS")
print("============")

for source, preferred in mapped:
    print(source, "->", preferred)


print()
print("UNMAPPED TERMS")
print("==============")

for term in unmapped:
    print(term)