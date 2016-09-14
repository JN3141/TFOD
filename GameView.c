// GameView.c ... GameView ADT implementation

#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "Globals.h"
#include "Game.h"
#include "GameView.h"
#include "Map.h" //... if you decide to use the Map ADT

typedef struct _player {
    int hp;                         // HP of the the player
    int turns;                      // num turns player has taken
    LocationID trail[TRAIL_SIZE];   // player's trail
    LocationID location;            // contains the ID of their location
} player;

struct gameView {
    int score;                      // current game score out of 366
    int turnNum;                    // number of turns
    PlayerID currPlayer;            // ID of current player
    Round roundNum;                 // number of rounds
    player players[NUM_PLAYERS];    // array of player data structs
};

static LocationID abbrevToID (char x, char y);

// Creates a new GameView to summarise the current state of the game
GameView newGameView(char *pastPlays, PlayerMessage messages[]) {
    // allocate and create the game view
    GameView gameView = malloc(sizeof(struct gameView));

    // set the score
    gameView->score = GAME_START_SCORE;

    // count the number of turns and rounds
    // turns...

    gameView->turnNum = 0;
    gameView->round = 0;

    if (pastPlays[0] == 'G') {
        int i;
        for (i = 1; pastPlays[i-1] != '\0'; i++) { ///////////////// not sure if null-terminated string
            if (i % 8 == 0) {
                gameView->turnNum++;
                if (gameView->turnNum % NUM_PLAYERS == 0) {
                    gameView->round++;
                }
            }
        }
    }

    // setting the current player
    gameView->currPlayer == PLAYER_LORD_GODALMING;

    // setting player stats
    PlayerID p;
    int t;
    for (p = PLAYER_LORD_GODALMING; p <= PLAYER_DRACULA; p++) {
        if (p >= PLAYER_LORD_GODALMING && p <= PLAYER_MINA_HARKER) {
            // it's a hunter!
            gameView->players[p].hp = GAME_START_HUNTER_LIFE_POINTS;
        } else if (p == PLAYER_DRACULA) {
            // it's Dracula!
            gameView->players[p].hp = GAME_START_BLOOD_POINTS;
        }
        gameView->players[p].turns = 0;
        gameView->players[p].location = UNKNOWN_LOCATION;
        for (t = 0; t < TRAIL_SIZE; t++) {
            gameView->players[p].trail[t] = UNKNOWN_LOCATION;
        }
    }


    // actually haven't taken a turn yet? just return the basis
    if (turnNum <= 0) {
        return gameView;
    }

    // so we've gone somewhat into the game? time to update
    // set the current turn
    gameView->currPlayer = gameView->numTurns % NUM_PLAYERS;

    // increment player turns
    for (p = PLAYER_LORD_GODALMING; p <= PLAYER_DRACULA; p++) {
        // every player would have made at least as many turns as rounds
        gameView->players[p].turns = gameView->round;

        // for every turn beyond the end of the last round
        if (p < (gameView->numTurns % NUM_PLAYERS)) {
            gameView->players[p].turns++;
        }
    }

    // now to start updating the stats of the gameView
    char *playTracker;
    for (playTracker = pastPlays;
         playTracker != pastPlays + 8 * gameView->turnNum;
         pastPlays += 8) {
        p = 0;
        switch (playTracker[0]) {
            case 'G': p = PLAYER_LORD_GODALMING; break;
            case 'S': p = PLAYER_DR_SEWARD; break;
            case 'H': p = PLAYER_VAN_HELSING; break;
            case 'M': p = PLAYER_MINA_HARKER; break;
            case 'D': p = PLAYER_DRACULA; break;
        }

        // set location and adjust trail...
        for (i = 0, i < TRAIL_SIZE - 1, i++) {
            gameView->players[p].trail[i] = gameView->players[p].trail[i+1];
        }
        gameView->players[p].trail[TRAIL_SIZE - 1] =
            abbrevToID(playTracker[1],playTracker[2]);
        gameView->players[p].location =
            abbrevToID(playTracker[1],playTracker[2]);

        if (p == PLAYER_DRACULA) { // it's dracula!
            // set location for dracula properly...
            if (gameView->players[p].location >= DOUBLE_BACK_1 &&
                       gameView->players[p].location <= TELEPORT) {
                switch (gameView->players[p].location) {
                    case 'DOUBLE_BACK_1':
                        gameView->players[p].location =
                            gameView->players[p].trail[4];
                        break;
                    case 'DOUBLE_BACK_2':
                        gameView->players[p].location =
                            gameView->players[p].trail[3];
                        break;
                    case 'DOUBLE_BACK_3':
                        gameView->players[p].location =
                            gameView->players[p].trail[2];
                        break;
                    case 'DOUBLE_BACK_4':
                        gameView->players[p].location =
                            gameView->players[p].trail[1];
                        break;
                    case 'DOUBLE_BACK_5':
                        gameView->players[p].location =
                            gameView->players[p].trail[0];
                        break;
                    case 'TELEPORT':
                        gameView->players[p].location =
                            CASTLE_DRACULA;
                        break;
                }
                gameView->players[p].trail[5] =
                    gameView->players[p].location;
            }

            // deduct 2 hp if dracula is at sea...
            // add 10 hp if dracula is at CASTLE_DRACULA and not dead...
            if (isSea(gameView->players[p].location) == TRUE) {
                gameView->playerID[p].health -= 2;
            } else if (gameView->players[p].location == CASTLE_DRACULA &&
                       gameView->players[p].hp > 0) {
                gameView->players[p].hp += 10;
            }

            // deduce 1 point from the game score...
            if (gameView->players[p] > 0) {
                gameView->score -= 1;
            }
        } else { // it's a hunter!
            // traps...
            if (playTracker[3] == 'T' && gameView->players[p].hp > 0) {
                gameView->players[p].hp -= 2;
            }
    	    if (playTracker[4] == 'T' && gameView->players[p].hp > 0) {
    		    gameView->players[p].hp -= 2;
            }
            if (playTracker[5] == 'T' && gameView->players[p].hp > 0) {
    		    gameView->players[p].hp -= 2;
    	    }

            // dracula encounter...
            if ((playTracker[3] == 'D' || playTracker[4] == 'D' ||
                 playTracker[5] == 'D' || playTracker[6] == 'D') &&
                 gameView->players[p].hp > 0) {
                gameView->players[p].hp -= 4;
                gameView->players[PLAYER_DRACULA].hp -= 10;
            }

            // resting in a city...
            if ((strcmp(gameView->players.trail[TRAIL_SIZE - 1],
                        gameView->players.trail[TRAIL_SIZE - 2]) == 0) &&
                gameView->players[p].hp > 0) {
                gameView->players[p].hp += 3;
                if (gameView->players[p].hp > 9) {
                    gameView->players[p].hp = 9;
                }
            }

            // automagical hospital teleport...
            if (gameView->players[p].hp <= 0) {
                gameView->playerID[current].hp = 9;
                for (i = 0, i < TRAIL_SIZE - 1, i++) {
                    gameView->players[p].trail[i] =
                        gameView->players[p].trail[i+1];
                }
                gameView->players[p].trail[TRAIL_SIZE - 1] =
                    ST_JOSEPH_AND_ST_MARYS;
                gameView->players[p].location =
                    ST_JOSEPH_AND_ST_MARYS;
        	    gameView->gameScore -= 6;
    	    }

    return gameView;
}


// Frees all memory previously allocated for the GameView toBeDeleted
void disposeGameView(GameView toBeDeleted) {
    assert(toBeDeleted != NULL);
    free(toBeDeleted);
    return;
}


//// Functions to return simple information about the current state of the game

// Get the current round
Round getRound(GameView currentView) {
    assert(currentView != NULL);
    return currentView->roundNum;
}

// Get the id of current player - ie whose turn is it?
PlayerID getCurrentPlayer(GameView currentView) {
    assert(currentView != NULL);
    assert(currentView->currPlayer >= PLAYER_LORD_GODALMING &&
           currentView->currPlayer <= PLAYER_DRACULA);
    return currentView->currPlayer;
}

// Get the current score
int getScore(GameView currentView) {
    assert(currentView != NULL);
    assert(currentView->score <= 366);
    int gameScore;
    if (currentView->score <= 0) {
        gameScore = 0;
    } else {
        gameScore = currentView->score;
    }
    return gameScore;
}

// Get the current health points for a given player
int getHealth(GameView currentView, PlayerID player) {
    assert(currentView != NULL);
    if (player == PLAYER_DRACULA) {
        assert(currentView->players[player].hp > 0);
    } else {
        assert(currentView->players[player].hp >= 0 &&
               currentView->players[player].hp <= 9)
    }
    return currentView->players[player].hp;
}

// Get the current location id of a given player
LocationID getLocation(GameView currentView, PlayerID player) {
    assert(currentView != NULL);
    if (currentView->players[player].location == UNKNOWN_LOCATION) {
        assert(getRound(currentView) == 0);
    } else if (player >= PLAYER_LORD_GODALMING &&
               player <= PLAYER_MINA_HARKER) {
        assert(currentView->players[player].location >= 0 &&
               currentView->players[player].location <= 70);
    } else {
        assert(currentView->players[player].location == CITY_UNKNOWN  ||
               currentView->players[player].location == SEA_UNKNOWN   ||
               currentView->players[player].location == HIDE          ||
               currentView->players[player].location == DOUBLE_BACK_N ||
               currentView->players[player].location == TELEPORT      ||
               currentView->players[player].location == LOCATION_UNKNOWN);
    }
    return currentView->players[player].location;
}

//// Functions that return information about the history of the game

// Fills the trail array with the location ids of the last 6 turns
void getHistory(GameView currentView, PlayerID player,
                LocationID trail[TRAIL_SIZE]) {
    assert(currentView != NULL);
    int turn;
    for (turn = 0; turn < TRAIL_SIZE; turn++) {
        trail[turn] = currentView->players[player].trail[turn];
        turn++;
    }
    return;
}

//// Functions that query the map to find information about connectivity

// Returns an array of LocationIDs for all directly connected locations
//////////////////////////////////// THIS NEEDS TO BE CHANGED
// TODO
LocationID *connectedLocations(GameView currentView, int *numLocations,
                               LocationID from, PlayerID player,
                               Round round, int road, int rail,
                               int sea) {
   // Make sure the passed in data isn't BS
   assert(currentView != NULL);
   assert(start >= MIN_MAP_LOCATION && start <= MAX_MAP_LOCATION);
   assert(end >= MIN_MAP_LOCATION && end <= MAX_MAP_LOCATION);
   // Initial setup of the values to be counted
   int num = 0;
   int arrayC = 0;
   VList currPrim = g->connections[start];

   while (currPrim != NULL) { // Iterate through the 'start' list
       if (currPrim->v == end) { // Found 'end'? End here
           type[arrayC] = currPrim->type;
           num++;
           arrayC++;
           break;
       } else if (isSea(currPrim->v) == TRUE && // 'Start' and 'end'
                   isLand(start) == TRUE &&     // both 'land', and
                   isLand(end) == TRUE) {       // found a 'sea'
           VList currSec = g->connections[currPrim->v];
           while (currSec != NULL) { // Iterate through 'sea' list
               if (currSec->v == end) { // Found 'end'? End here
                   type[arrayC] = currSec->type;
                   num++;
                   arrayC++;
                   break;
               }
               currSec = currSec->next;
           }
       }
       currPrim = currPrim->next;
   }
   return num;
}

static LocationID abbrevToID (char x, char y) {

    // create array of locations
    char *abbrev[NUM_LOCATIONS] =
    {"AS", "AL", "AM", "AT", "AO", "BA", "BI", "BB", "BE", "BR", "BS",
     "BO", "BU", "BC", "BD", "CA", "CG", "CD", "CF", "CO", "CN", "DU",
     "ED", "EC", "FL", "FR", "GA", "GW", "GE", "GO", "GR", "HA", "IO",
     "IR", "KL", "LE", "LI", "LS", "LV", "LO", "MA", "MN", "MR", "MS",
     "MI", "MU", "NA", "NP", "NS", "NU", "PA", "PL", "PR", "RO", "SA",
     "SN", "SR", "SJ", "SO", "JM", "ST", "SW", "SZ", "TO", "TS", "VA",
     "VR", "VE", "VI", "ZA", "ZU"};

    // put the characters x and y into a null-terminated string
    char *playerLocation = malloc(sizeof(char)*3);
    playerLocation[0] = x;
    playerLocation[1] = y;
    playerLocation[2] = '\0';

    // first check if its any of the dracula specific possibilities
    int ID;
    if (strcmp(playerLocation, "C?") == 0) {
        ID = CITY_UNKNOWN;
    } else if (strcmp(playerLocation, "S?") == 0) {
        ID = SEA_UNKNOWN
    } else if (strcmp(playerLocation, "HI") == 0) {
        ID = HIDE;
    } else if (strcmp(playerLocation, "D1") == 0) {
        ID = DOUBLE_BACK_1;
    } else if (strcmp(playerLocation, "D2") == 0) {
        ID = DOUBLE_BACK_2;
    } else if (strcmp(playerLocation, "D3") == 0) {
        ID = DOUBLE_BACK_3;
    } else if (strcmp(playerLocation, "D4") == 0) {
        ID = DOUBLE_BACK_4;
    } else if (strcmp(playerLocation, "D5") == 0) {
        ID = DOUBLE_BACK_5;
    } else if (strcmp(playerLocation, "TP") == 0) {
        ID = TELEPORT;
    } else {
        ID = UNKNOWN_LOCATION;
    }

    // find the ID from abbrevs[]
    int start = MIN_MAP_LOCATION;
    int end = MAX_MAP_LOCATION;
    while (strcmp(abbrev[start],abbrev[end]) <= 0) {
        mid = (start + end)/2;
        if (strcmp(name,abbrev[mid]) < 0) {
            end = mid - 1;
        } else if (strcmp(name,abbrev[mid]) > 0) {
            start = mid + 1;
        } else {
            ID = abbrev[mid];
        }
    }

    free(playerLocation);

    return ID;
}
