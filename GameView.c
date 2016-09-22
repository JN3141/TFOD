// GameView.c ... GameView ADT implementation

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "Globals.h"
#include "Game.h"
#include "GameView.h"
#include "encounter.h"
#include "Map.h" //... if you decide to use the Map ADT
#include "Places.h"

typedef struct _player {
    int hp;                              // HP of the the player
    int turns;                           // num turns player has taken
    LocationID trail[GAME_START_SCORE];  // player's trail; has
                                         //     GAME_START_SCORE for
                                         //     possible chain of
                                         //     DOUBLE_BACK's and HIDE's
                                         //     N.B. only the front 6
                                         //     are ever really
                                         //     accessible though
    LocationID location;                 // contains the ID of their
                                         //     location
} player;

typedef struct _encounters {
    LocationID location;
    char type;
} encounters;

typedef struct vNode *VList;

struct vNode {
  LocationID  v;    // ALICANTE, etc
  TransportID type; // ROAD, RAIL, BOAT
  VList       next; // link to next node
};

struct MapRep {
  int   nV;         //#vertices
  int   nE;         // #edges
  VList connections[NUM_MAP_LOCATIONS]; // array of lists
};

struct gameView {
    int score;                      // current game score out of 366
    int numTurns;                   // number of turns
    PlayerID currPlayer;            // ID of current player
    Round numRounds;                // number of rounds
    player players[NUM_PLAYERS];    // array of player data structs
    encounters eTrail[TRAIL_SIZE];  // trail of active encounters (T/V)
};

static LocationID placeToID (char x, char y);

// Creates a new GameView to summarise the current state of the game
GameView newGameView(char *pastPlays, PlayerMessage messages[]) {
    // allocate and create the game view
    GameView gameView = malloc(sizeof(struct gameView));

    // set the score
    gameView->score = GAME_START_SCORE;

    // count the number of turns and rounds
    // turns...

    gameView->numTurns = 0;
    gameView->numRounds = 0;


    // setting the current player
    gameView->currPlayer = PLAYER_LORD_GODALMING;

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
        for (t = 0; t < GAME_START_SCORE; t++) {
            gameView->players[p].trail[t] = UNKNOWN_LOCATION;
        }
    }



    // actually haven't taken a turn yet? just return the basis
    if (pastPlays[0] != 'G') {
        return gameView;
    } else {
        int i;
        for (i = 1; pastPlays[i-1] != '\0'; i++) {
            if (i % 8 == 0) {
                gameView->numTurns++;
                if (gameView->numTurns % NUM_PLAYERS == 0) {
                    gameView->numRounds++;
                }
            }
        }
        if (i % 8 == 0) {
            gameView->numTurns++;
            if (gameView->numTurns % NUM_PLAYERS == 0) {
                gameView->numRounds++;
            }
        }
    }

    // now to start updating the stats of the gameView
    char *playTracker;
    int j;
    for (playTracker = pastPlays;
         /*playTracker[8] != '\0';*/
         playTracker != pastPlays + 8 * gameView->numTurns;
         playTracker += 8) {
        p = 0;
        switch (playTracker[0]) {
            case 'G': p = PLAYER_LORD_GODALMING; break;
            case 'S': p = PLAYER_DR_SEWARD; break;
            case 'H': p = PLAYER_VAN_HELSING; break;
            case 'M': p = PLAYER_MINA_HARKER; break;
            case 'D': p = PLAYER_DRACULA; break;
        }
        gameView->players[p].turns++;

        // set location and adjust trail...
        // newest at the front, oldest at the end...
        for (j = GAME_START_SCORE - 1; j > 0; j--) {
            gameView->players[p].trail[j] = gameView->players[p].trail[j-1];
        }
        gameView->players[p].trail[0] =
            placeToID(playTracker[1],playTracker[2]);
        gameView->players[p].location =
            placeToID(playTracker[1],playTracker[2]);

        if (p == PLAYER_DRACULA) { // it's dracula!
            LocationID temp = UNKNOWN_LOCATION;
            // set location for dracula properly if needed...
            if (gameView->players[p].location >= HIDE &&
                       gameView->players[p].location <= TELEPORT) {
                temp = gameView->players[p].location;
                int i = 0;
                int found = FALSE;
                while (found == FALSE) {
                    if (temp == HIDE) {
                        i += 1;
                        temp = gameView->players[p].trail[i];
                    } else if (temp == DOUBLE_BACK_1) {
                        i += 1;
                        temp = gameView->players[p].trail[i];
                    } else if (temp == DOUBLE_BACK_2) {
                        i += 2;
                        temp = gameView->players[p].trail[i];
                    } else if (temp == DOUBLE_BACK_3) {
                        i += 3;
                        temp = gameView->players[p].trail[i];
                    } else if (temp == DOUBLE_BACK_4) {
                        i += 4;
                        temp = gameView->players[p].trail[i];
                    } else if (temp == DOUBLE_BACK_5) {
                        i += 5;
                        temp = gameView->players[p].trail[i];
                    } else if (temp == TELEPORT) {
                        found = TRUE;
                        temp = CASTLE_DRACULA;
                    } else {
                        found = TRUE;
                    }
                }
            }

            // deduct 2 hp if dracula is at sea...
            // add 10 hp if dracula is at CASTLE_DRACULA and not dead...
            if (((validPlace(gameView->players[p].location) == TRUE &&
                isSea(gameView->players[p].location) == TRUE)       ||
                gameView->players[p].location == SEA_UNKNOWN)       ||
                ((validPlace(temp) && isSea(temp))                ||
                  temp == SEA_UNKNOWN)) {
                gameView->players[p].hp -= 2;
            } else if (gameView->players[p].location == CASTLE_DRACULA &&
                       gameView->players[p].hp > 0) {
                gameView->players[p].hp += 10;
            }


            if (playTracker[3] == 'T') {
                // adjust eTrail[]...
                // newest at the front, oldest at the end...

                // shift the array...
                for (j = TRAIL_SIZE - 1; j > 0; j--) {
                    gameView->eTrail[j].location =
                        gameView->eTrail[j-1].location;
                    gameView->eTrail[j].type =
                        gameView->eTrail[j-1].type;
                }

                // add the new encounter...
                gameView->eTrail[0].location =
                    placeToID(playTracker[1],playTracker[2]);
                gameView->eTrail[0].type = 'T';

            } else if (playTracker[4] == 'V') {

                // adjust eTrail[]...
                // newest at the front, oldest at the end...

                // shift the array...
                for (j = TRAIL_SIZE - 1; j > 0; j--) {
                    gameView->eTrail[j].location =
                        gameView->eTrail[j-1].location;
                    gameView->eTrail[j].type =
                        gameView->eTrail[j-1].type;
                }

                // add the new encounter...
                gameView->eTrail[0].location =
                    placeToID(playTracker[1],playTracker[2]);
                gameView->eTrail[0].type = 'V';


            }

            // deduct 1 point from the game score...
            if (gameView->players[p].hp > 0) {
                gameView->score -= 1;
            }

				// MATURED Vampire this isn't working properly
				//etrail doesnt work here
				//printf(" trail type %c\n",gameView->eTrail[TRAIL_SIZE-1].type);
            if (gameView->eTrail[TRAIL_SIZE-1].type == 'V') {
                gameView->score = gameView->score-13;
            }

        } else { // it's a hunter!
            // they were staying at the hospital the previous turn!
            if (gameView->players[p].hp == 0 && (gameView->players[p].trail[1] ==
                ST_JOSEPH_AND_ST_MARYS))  {
                gameView->players[p].hp = 9;
            }

            // immature vampires...
            if (playTracker[3] == 'V') {
                for (j = 0; j < TRAIL_SIZE - 1; j++) {
                    if (gameView->eTrail[j].location ==
                        placeToID(playTracker[1],playTracker[2]) &&
                        gameView->eTrail[j].type == 'T') {
                        break;
                    }
                }
                gameView->eTrail[j].location = NOWHERE;
                gameView->eTrail[j].type = '.';
            }

            // traps...
            if (playTracker[3] == 'T' && gameView->players[p].hp > 0) {
                // deduct hp...
                gameView->players[p].hp -= 2;

                // remove from eTrail[]...
                for (j = 0; j < TRAIL_SIZE - 1; j++) {
                    if (gameView->eTrail[j].location ==
                        placeToID(playTracker[1],playTracker[2]) &&
                        gameView->eTrail[j].type == 'T') {
                        break;
                    }
                }
                gameView->eTrail[j].location = NOWHERE;
                gameView->eTrail[j].type = '.';
            }
    	    if (playTracker[4] == 'T' && gameView->players[p].hp > 0) {
                // deduct hp...
                gameView->players[p].hp -= 2;

                // remove from eTrail[]...
                for (j = 0; j < TRAIL_SIZE - 1; j++) {
                    if (gameView->eTrail[j].location ==
                        placeToID(playTracker[1],playTracker[2]) &&
                        gameView->eTrail[j].type == 'T') {
                        break;
                    }
                }
                gameView->eTrail[j].location = NOWHERE;
                gameView->eTrail[j].type = '.';
            }
            if (playTracker[5] == 'T' && gameView->players[p].hp > 0) {
                // deduct hp...
                gameView->players[p].hp -= 2;

                // remove from eTrail[]...
                for (j = 0; j < TRAIL_SIZE - 1; j++) {
                    if (gameView->eTrail[j].location ==
                        placeToID(playTracker[1],playTracker[2]) &&
                        gameView->eTrail[j].type == 'T') {
                        break;
                    }
                }
                gameView->eTrail[j].location = NOWHERE;
                gameView->eTrail[j].type = '.';
    	    }
            if (playTracker[6] == 'T' && gameView->players[p].hp > 0) {
                // deduct hp...
                gameView->players[p].hp -= 2;

                // remove from eTrail[]...
                for (j = 0; j < TRAIL_SIZE - 1; j++) {
                    if (gameView->eTrail[j].location ==
                        placeToID(playTracker[1],playTracker[2]) &&
                        gameView->eTrail[j].type == 'T') {
                        break;
                    }
                }
                gameView->eTrail[j].location = NOWHERE;
                gameView->eTrail[j].type = '.';
            }

            // dracula encounter...
            if ((playTracker[3] == 'D' || playTracker[4] == 'D' ||
                 playTracker[5] == 'D' || playTracker[6] == 'D') &&
                 gameView->players[p].hp > 0) {
                gameView->players[p].hp -= 4;
                gameView->players[PLAYER_DRACULA].hp -= 10;
            }

            // resting in a city...
            if ((gameView->players[p].trail[0] ==
                gameView->players[p].trail[1]) &&
                gameView->players[p].hp > 0) {
                gameView->players[p].hp += 3;
                if (gameView->players[p].hp > 9) {
                    gameView->players[p].hp = 9;
                }
            }

            // automagical hospital teleport...
            if (gameView->players[p].hp <= 0 && (gameView->players[p].trail[1] !=
                    ST_JOSEPH_AND_ST_MARYS)) {
                gameView->players[p].hp = 0;
                for (j = GAME_START_SCORE - 1; j > 0; j--) {
                    gameView->players[p].trail[j] =
                        gameView->players[p].trail[j-1];
                }
                gameView->players[p].trail[0] =
                    ST_JOSEPH_AND_ST_MARYS;
                gameView->players[p].location =
                    ST_JOSEPH_AND_ST_MARYS;
        	    gameView->score -= 6;
            }
        }
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
    return currentView->numRounds;
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
               currentView->players[player].hp <= 9);
    }
    return currentView->players[player].hp;
}

// Get the current location id of a given player
LocationID getLocation(GameView currentView, PlayerID player) {
    assert(currentView != NULL);
    assert(player >= PLAYER_LORD_GODALMING && player < NUM_PLAYERS);
    /*if (currentView->players[player].location == UNKNOWN_LOCATION &&
        player >= PLAYER_LORD_GODALMING && player <= PLAYER_MINA_HARKER) {
        assert(getRound(currentView) == 0);
    } else if (player >= PLAYER_LORD_GODALMING &&
               player <= PLAYER_MINA_HARKER) {
        assert(currentView->players[player].location >= 0 &&
               currentView->players[player].location <= 70);
    } else {
        assert((currentView->players[player].location >= CITY_UNKNOWN &&
               currentView->players[player].location <= TELEPORT)     ||
               currentView->players[player].location == NOWHERE);
    }*/
    return currentView->players[player].location;
}

//// Functions that return information about the history of the game

// Fills the trail array with the location ids of the last 6 turns
void getHistory(GameView currentView, PlayerID player,
                LocationID trail[TRAIL_SIZE]) {
    assert(currentView != NULL);
    int i;
    for (i = 0; i < TRAIL_SIZE; i++) {
        trail[i] = currentView->players[player].trail[i];
    }
    return;
}

//// Functions that query the map to find information about connectivity

// Returns an array of LocationIDs for all directly connected locations
LocationID *connectedLocations(GameView currentView, int *numLocations,
                               LocationID from, PlayerID player,
                               Round round, int road, int rail,
                               int sea) {

    // Make sure the passed in data isn't BS
    assert(currentView != NULL);
    assert(from >= MIN_MAP_LOCATION);

    // Initial setup of the values to be counted
    int seen[NUM_MAP_LOCATIONS] = {0};
    LocationID *locationArray = malloc(NUM_MAP_LOCATIONS * sizeof(LocationID));
    int num = 1;
    int skipLoop = 0;
    int railDepth = (round + player) % 4;
    int i;
    Map g = newMap();
    locationArray[0] = from;
    VList currPrim = g->connections[from];

    while (currPrim != NULL) {
   	    // refreshing the skip
       	skipLoop = FALSE;

       	// dracula can't be at the hospital!
       	if (currPrim->v == ST_JOSEPH_AND_ST_MARYS &&
            player == PLAYER_DRACULA) {
       		skipLoop = TRUE;
       	}

       	if (skipLoop != TRUE) {
       		// dracula can't go on rail
       		// "sum mod 4 is 0: No train move is permitted for the Hunter
            // this turn."
       		if (player != PLAYER_DRACULA && rail && currPrim->type == RAIL &&
                (railDepth >= 1)) {
                seen[currPrim->v] = 1;
       		} else if (road && currPrim->type == ROAD &&
                       seen[currPrim->v] != 1) {
                seen[currPrim->v] = 'R';
       		} else if (sea && currPrim->type == BOAT &&
                       seen[currPrim->v] != 1) {
                seen[currPrim->v] = 'B';
       		}
        }
        currPrim = currPrim->next;
    }

    if (railDepth >= 2) {
        for (i = 0; i < NUM_MAP_LOCATIONS; i++) {
            if (seen[i] == 1) {
                currPrim = g->connections[i];
                while (currPrim != NULL) {
                    if (currPrim->type == RAIL) {
               			seen[currPrim->v] = 2;
               		}
                    currPrim = currPrim->next;
                }
            }
        }
    }

    if (railDepth >= 3) {
        for (i = 0; i < NUM_MAP_LOCATIONS; i++) {
            if (seen[i] == 2) {
                currPrim = g->connections[i];
                while (currPrim != NULL) {
                    if (currPrim->type == RAIL) {
               			seen[currPrim->v] = 3;
               		}
                    currPrim = currPrim->next;
                }
            }
        }
    }

    for (i = 0; i <= NUM_MAP_LOCATIONS; i++) {
        if ((seen[i] != 1 && seen[i] != 2 && seen[i] != 3 &&
            seen[i] != 'R' && seen[i] != 'B') || i == from) {
            continue;
        }
        locationArray[num] = i;
        num++;
    }

    *numLocations = num;

    return locationArray;
}

Encounters getEncounters(GameView currentView) {
    return (Encounters)(currentView->eTrail);
}

void getEncountersData(GameView currentView, LocationID where,
                         int *numTraps, int *numVamps) {
    *numTraps = 0;
    *numVamps = 0;
    int j;
    for (j = 0; j < TRAIL_SIZE - 1; j++) {
        if (currentView->eTrail[j].location == where) {
            if (currentView->eTrail[j].type == 'T') {
                (*numTraps)++;
            }
            if (currentView->eTrail[j].type == 'V') {
                (*numVamps)++;
            }
        }
    }
    return;
}

static LocationID placeToID (char x, char y) {

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
        ID = SEA_UNKNOWN;
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
    if (ID == UNKNOWN_LOCATION) {
        ID = abbrevToID(playerLocation);
    }

    free(playerLocation);

    return ID;
}
