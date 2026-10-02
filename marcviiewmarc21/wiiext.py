# Wiiext
# Wii-specific controlled vocabulary for video game genres and forms.
#
# Wiiext is an independent vocabulary created for MarcViiew.
# GameTDB terms are used as source terms and are mapped to
# Wiiext preferred terms.


WIIEXT_TERMS = {

    # ==========================================================
    # CORE GENRES
    # ==========================================================

    "Action games": {
        "type": "genre",
        "source_terms": ["action"],
        "broader": "Video games",
        "narrower": [
            "Beat 'em up games",
            "Stealth action games",
            "Run and gun games"
        ]
    },

    "Adventure games": {
        "type": "genre",
        "source_terms": ["adventure"],
        "broader": "Video games",
        "narrower": [
            "Point-and-click adventure games",
            "Interactive fiction"
        ]
    },

        "Arcade games": {
        "type": "genre",
        "source_terms": ["arcade"],
        "broader": "Video games",
        "narrower": [
            "Pinball games"
        ]
    },

    "Pinball games": {
        "type": "genre",
        "source_terms": ["pinball"],
        "broader": "Arcade games"
    },

    "Fighting games": {
        "type": "genre",
        "source_terms": ["fighting"],
        "broader": "Action games",
        "narrower": [
            "2D fighting games",
            "3D fighting games"
        ]
    },

    "Music games": {
        "type": "genre",
        "source_terms": ["music"],
        "broader": "Video games",
        "narrower": [
            "Rhythm games",
            "Karaoke games",
            "Dance games"
        ]
    },

    "Party games": {
        "type": "genre",
        "source_terms": ["party"],
        "broader": "Video games"
    },

    "Platform games": {
        "type": "genre",
        "source_terms": ["platformer"],
        "broader": "Action games",
        "narrower": [
            "2D platform games",
            "3D platform games"
        ]
    },

    "Puzzle games": {
        "type": "genre",
        "source_terms": ["puzzle"],
        "broader": "Video games",
        "narrower": [
            "Hidden object games"
        ]
    },

    "Racing games": {
        "type": "genre",
        "source_terms": ["racing"],
        "broader": "Sports games",
        "narrower": [
            "Kart racing games",
            "Motorcycle racing games",
            "Futuristic racing games",
            "Off-road racing games",
            "Truck racing games",
            "Bike racing games",
            "Watercraft racing games",
            "Sim racing games"
        ]
    },

    "Role-playing games": {
        "type": "genre",
        "source_terms": ["role-playing"],
        "broader": "Video games",
        "narrower": [
            "Action role-playing games",
            "Tactical role-playing games",
            "Strategy role-playing games"
        ]
    },

    "Simulation games": {
        "type": "genre",
        "source_terms": ["simulation"],
        "broader": "Video games",
        "narrower": [
            "Life simulation games",
            "Business simulation games",
            "Construction simulation games",
            "Management simulation games",
            "Flight simulation games",
            "Train simulation games",
            "Virtual pet games",
            "Cooking games"
        ]
    },

    "Sports games": {
        "type": "genre",
        "source_terms": ["sports"],
        "broader": "Video games",
        "narrower": [
            "Football games",
            "Soccer games",
            "Golf games",
            "Baseball games",
            "Basketball games",
            "Hockey games",
            "Tennis games",
            "Bowling games",
            "Boxing games",
            "Wrestling games",
            "Table tennis games",
            "Volleyball games",
            "Cricket games",
            "Rugby games",
            "Darts games",
            "Billiards games",
            "Pétanque games",
            "Skateboarding games",
            "Snowboarding games",
            "Skiing games",
            "Surfing games",
            "Fishing games",
            "Hunting games",
            "Martial arts games",
            "Exercise games",
            "Fitness games",
            "Coaching games"
        ]
    },

    "Strategy games": {
        "type": "genre",
        "source_terms": ["strategy"],
        "broader": "Video games",
        "narrower": [
            "Real-time strategy games",
            "Turn-based strategy games",
            "Tower defense games",
            "Wargames"
        ]
    },


    # ==========================================================
    # PLATFORM GAMES
    # ==========================================================

    "2D platform games": {
        "type": "genre",
        "source_terms": ["2d platformer"],
        "broader": "Platform games"
    },

    "3D platform games": {
        "type": "genre",
        "source_terms": ["3d platformer"],
        "broader": "Platform games"
    },


    # ==========================================================
    # RACING
    # ==========================================================

    "Kart racing games": {
        "type": "genre",
        "source_terms": ["kart racing"],
        "broader": "Racing games"
    },

    "Motorcycle racing games": {
        "type": "genre",
        "source_terms": ["motorcycle racing"],
        "broader": "Racing games"
    },

    "Futuristic racing games": {
        "type": "genre",
        "source_terms": ["futuristic racing"],
        "broader": "Racing games"
    },

    "Off-road racing games": {
        "type": "genre",
        "source_terms": ["off-road racing"],
        "broader": "Racing games"
    },

    "Truck racing games": {
        "type": "genre",
        "source_terms": ["truck racing"],
        "broader": "Racing games"
    },

    "Bike racing games": {
        "type": "genre",
        "source_terms": ["bike racing"],
        "broader": "Racing games"
    },

    "Watercraft racing games": {
        "type": "genre",
        "source_terms": ["watercraft racing"],
        "broader": "Racing games"
    },

    "Sim racing games": {
        "type": "genre",
        "source_terms": ["sim racing"],
        "broader": "Racing games"
    },


    # ==========================================================
    # ROLE-PLAYING
    # ==========================================================

    "Action role-playing games": {
        "type": "genre",
        "source_terms": ["action rpg"],
        "broader": "Role-playing games"
    },

    "Tactical role-playing games": {
        "type": "genre",
        "source_terms": ["tactical rpg"],
        "broader": "Role-playing games"
    },

    "Strategy role-playing games": {
        "type": "genre",
        "source_terms": ["strategy rpg"],
        "broader": "Role-playing games"
    },


    # ==========================================================
    # SHOOTER / COMBAT
    # ==========================================================

    "Shooter games": {
        "type": "genre",
        "source_terms": ["shooter"],
        "broader": "Action games",
        "narrower": [
            "First-person shooter games",
            "Third-person shooter games",
            "Rail shooter games",
            "Shoot 'em up games",
            "Run and gun games"
        ]
    },

    "First-person shooter games": {
        "type": "genre",
        "source_terms": ["first-person shooter"],
        "broader": "Shooter games"
    },

    "Third-person shooter games": {
        "type": "genre",
        "source_terms": ["third-person shooter"],
        "broader": "Shooter games"
    },

    "Rail shooter games": {
        "type": "genre",
        "source_terms": ["rail shooter"],
        "broader": "Shooter games"
    },

    "Shoot 'em up games": {
        "type": "genre",
        "source_terms": ["shoot 'em up"],
        "broader": "Shooter games"
    },

    "Run and gun games": {
        "type": "genre",
        "source_terms": ["run and gun"],
        "broader": "Shooter games"
    },

    "Beat 'em up games": {
        "type": "genre",
        "source_terms": ["beat 'em up"],
        "broader": "Action games"
    },

    "Stealth action games": {
        "type": "genre",
        "source_terms": ["stealth action"],
        "broader": "Action games"
    },

    "2D fighting games": {
        "type": "genre",
        "source_terms": ["2d fighting"],
        "broader": "Fighting games"
    },

    "3D fighting games": {
        "type": "genre",
        "source_terms": ["3d fighting"],
        "broader": "Fighting games"
    },

    "Survival horror games": {
        "type": "genre",
        "source_terms": ["survival horror"],
        "broader": "Horror games"
    },


    # ==========================================================
    # MUSIC / RHYTHM
    # ==========================================================

    "Rhythm games": {
        "type": "genre",
        "source_terms": ["rhythm"],
        "broader": "Music games"
    },

    "Karaoke games": {
        "type": "genre",
        "source_terms": ["karaoke"],
        "broader": "Music games"
    },

    "Dance games": {
        "type": "genre",
        "source_terms": ["dance"],
        "broader": "Music games"
    },


    # ==========================================================
    # SIMULATION
    # ==========================================================

    "Life simulation games": {
        "type": "genre",
        "source_terms": ["life simulation"],
        "broader": "Simulation games"
    },

    "Business simulation games": {
        "type": "genre",
        "source_terms": ["business simulation"],
        "broader": "Simulation games"
    },

    "Construction simulation games": {
        "type": "genre",
        "source_terms": ["construction simulation"],
        "broader": "Simulation games"
    },

    "Management simulation games": {
        "type": "genre",
        "source_terms": ["management simulation"],
        "broader": "Simulation games"
    },

    "Flight simulation games": {
        "type": "genre",
        "source_terms": ["flight simulation"],
        "broader": "Simulation games"
    },

    "Train simulation games": {
        "type": "genre",
        "source_terms": ["train simulation"],
        "broader": "Simulation games"
    },

    "Virtual pet games": {
        "type": "genre",
        "source_terms": ["virtual pet"],
        "broader": "Simulation games"
    },

    "Cooking games": {
        "type": "genre",
        "source_terms": ["cooking"],
        "broader": "Simulation games"
    },


    # ==========================================================
    # SPORTS / ACTIVITIES
    # ==========================================================

    "Football games": {
        "type": "genre",
        "source_terms": ["football"],
        "broader": "Sports games"
    },

    "Soccer games": {
        "type": "genre",
        "source_terms": ["soccer"],
        "broader": "Sports games"
    },

    "Golf games": {
        "type": "genre",
        "source_terms": ["golf"],
        "broader": "Sports games"
    },

    "Baseball games": {
        "type": "genre",
        "source_terms": ["baseball"],
        "broader": "Sports games"
    },

    "Basketball games": {
        "type": "genre",
        "source_terms": ["basketball"],
        "broader": "Sports games"
    },

    "Hockey games": {
        "type": "genre",
        "source_terms": ["hockey"],
        "broader": "Sports games"
    },

    "Tennis games": {
        "type": "genre",
        "source_terms": ["tennis"],
        "broader": "Sports games"
    },

    "Bowling games": {
        "type": "genre",
        "source_terms": ["bowling"],
        "broader": "Sports games"
    },

    "Boxing games": {
        "type": "genre",
        "source_terms": ["boxing"],
        "broader": "Sports games"
    },

    "Wrestling games": {
        "type": "genre",
        "source_terms": ["wrestling"],
        "broader": "Sports games"
    },

    "Table tennis games": {
        "type": "genre",
        "source_terms": ["table tennis"],
        "broader": "Sports games"
    },

    "Volleyball games": {
        "type": "genre",
        "source_terms": ["volleyball"],
        "broader": "Sports games"
    },

    "Cricket games": {
        "type": "genre",
        "source_terms": ["cricket"],
        "broader": "Sports games"
    },

    "Rugby games": {
        "type": "genre",
        "source_terms": ["rugby"],
        "broader": "Sports games"
    },

    "Darts games": {
        "type": "genre",
        "source_terms": ["darts"],
        "broader": "Sports games"
    },

    "Billiards games": {
        "type": "genre",
        "source_terms": ["billiards"],
        "broader": "Sports games"
    },

    "Pétanque games": {
        "type": "genre",
        "source_terms": ["pétanque"],
        "broader": "Sports games"
    },

    "Skateboarding games": {
        "type": "genre",
        "source_terms": ["skateboarding"],
        "broader": "Sports games"
    },

    "Snowboarding games": {
        "type": "genre",
        "source_terms": ["snowboarding"],
        "broader": "Sports games"
    },

    "Skiing games": {
        "type": "genre",
        "source_terms": ["ski"],
        "broader": "Sports games"
    },

    "Surfing games": {
        "type": "genre",
        "source_terms": ["surfing"],
        "broader": "Sports games"
    },

    "Fishing games": {
        "type": "genre",
        "source_terms": ["fishing"],
        "broader": "Sports games"
    },

    "Hunting games": {
        "type": "genre",
        "source_terms": ["hunting"],
        "broader": "Sports games"
    },

    "Martial arts games": {
        "type": "genre",
        "source_terms": ["martial arts"],
        "broader": "Sports games"
    },

    "Exercise games": {
        "type": "genre",
        "source_terms": ["exercise"],
        "broader": "Sports games"
    },

    "Fitness games": {
        "type": "genre",
        "source_terms": ["fitness"],
        "broader": "Sports games"
    },

    "Coaching games": {
        "type": "genre",
        "source_terms": ["coaching"],
        "broader": "Sports games"
    },


    # ==========================================================
    # BOARD / CARD / TRIVIA
    # ==========================================================

    "Board games": {
        "type": "genre",
        "source_terms": ["board game"],
        "broader": "Video games"
    },

    "Card games": {
        "type": "genre",
        "source_terms": ["cards"],
        "broader": "Video games"
    },

    "Poker games": {
        "type": "genre",
        "source_terms": ["poker"],
        "broader": "Card games"
    },

    "Chess games": {
        "type": "genre",
        "source_terms": ["chess"],
        "broader": "Board games"
    },

    "Trivia games": {
        "type": "genre",
        "source_terms": ["trivia"],
        "broader": "Video games"
    },


    # ==========================================================
    # OTHER GAME TYPES
    # ==========================================================

    "Point-and-click adventure games": {
        "type": "genre",
        "source_terms": ["point-and-click"],
        "broader": "Adventure games"
    },

    "Interactive fiction": {
        "type": "genre",
        "source_terms": ["interactive fiction"],
        "broader": "Adventure games"
    },

    "Interactive movies": {
        "type": "genre",
        "source_terms": ["interactive movie"],
        "broader": "Video games"
    },

    "Hidden object games": {
        "type": "genre",
        "source_terms": ["hidden object"],
        "broader": "Puzzle games"
    },

    "Drawing games": {
        "type": "genre",
        "source_terms": ["drawing"],
        "broader": "Creative software"
    },

    "Traditional games": {
        "type": "genre",
        "source_terms": ["traditional"],
        "broader": "Video games"
    },

    "Roguelike games": {
        "type": "genre",
        "source_terms": ["roguelike"],
        "broader": "Video games"
    },

    "MMORPGs": {
        "type": "genre",
        "source_terms": ["mmorpg"],
        "broader": "Role-playing games"
    },


    # ==========================================================
    # STRATEGY
    # ==========================================================

    "Real-time strategy games": {
        "type": "genre",
        "source_terms": ["real-time strategy"],
        "broader": "Strategy games"
    },

    "Turn-based strategy games": {
        "type": "genre",
        "source_terms": ["turn-based strategy"],
        "broader": "Strategy games"
    },

    "Tower defense games": {
        "type": "genre",
        "source_terms": ["tower defense"],
        "broader": "Strategy games"
    },

    "Wargames": {
        "type": "genre",
        "source_terms": ["wargame"],
        "broader": "Strategy games"
    },


    # ==========================================================
    # HORROR / THEMES / SETTINGS
    # ==========================================================

    "Horror games": {
        "type": "genre",
        "source_terms": ["horror"],
        "broader": "Video games",
        "narrower": [
            "Survival horror games"
        ]
    },

    "Fantasy": {
        "type": "theme",
        "source_terms": ["fantasy"],
        "broader": "Video game themes"
    },

    "Science fiction": {
        "type": "theme",
        "source_terms": ["sci-fi"],
        "broader": "Video game themes"
    },

    "Historic settings": {
        "type": "theme",
        "source_terms": ["historic"],
        "broader": "Video game settings"
    },

    "Modern-day settings": {
        "type": "theme",
        "source_terms": ["modern day"],
        "broader": "Video game settings"
    },


    # ==========================================================
    # FORM / SOFTWARE TYPES
    # ==========================================================

    "Compilations": {
        "type": "form",
        "source_terms": ["compilation"],
        "broader": "Video game software"
    },

    "Demos": {
        "type": "form",
        "source_terms": ["demo"],
        "broader": "Video game software"
    },

    "Samplers": {
        "type": "form",
        "source_terms": ["sampler"],
        "broader": "Video game software"
    },

    "Educational software": {
        "type": "form",
        "source_terms": ["educational"],
        "broader": "Software"
    },

    "Multimedia software": {
        "type": "form",
        "source_terms": ["multimedia"],
        "broader": "Software"
    },

    "Emulators": {
        "type": "form",
        "source_terms": ["emulator"],
        "broader": "Software"
    },

    "Software": {
        "type": "form",
        "source_terms": ["software"],
        "broader": "Video game software"
    }
}


# ==============================================================
# LOOKUP FUNCTIONS
# ==============================================================


def get_wiiext_term(source_term):

    source_term = source_term.lower().strip()

    for preferred_term, data in WIIEXT_TERMS.items():

        source_terms = [
            term.lower()
            for term in data["source_terms"]
        ]

        if source_term in source_terms:
            return preferred_term

    return None


def get_term_data(preferred_term):

    return WIIEXT_TERMS.get(preferred_term)


# ==============================================================
# TEST
# ==============================================================


test_terms = [
    "platformer",
    "3d platformer",
    "action rpg",
    "first-person shooter",
    "kart racing",
    "survival horror",
    "pinball"
]


for test_term in test_terms:

    result = get_wiiext_term(test_term)

    print("GameTDB term:", test_term)
    print("Wiiext term:", result)
    print()