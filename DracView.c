// DracView.c ... DracView ADT implementation

#include <stdlib.h>
#include <assert.h>
#include "Globals.h"
#include "Game.h"
#include "GameView.h"
#include "DracView.h"
#include "encounter.h"
// #include "Map.h" ... if you decide to use the Map ADT

struct dracView {
    GameView gV;                    // generated by newGameView()
    Encounters eTrail;              // pointer to the trail of active
                                    //     encounters (T/V) in gV;
                                    //     of note, it is of size
                                    //     TRAIL_SIZE
};


// Creates a new DracView to summarise the current state of the game
DracView newDracView(char *pastPlays, PlayerMessage messages[]) {

    DracView dracView = malloc(sizeof(struct dracView));
    dracView->gV = newGameView(pastPlays, messages);
    dracView->eTrail = getEncounters(dracView->gV);

    return dracView;
}


// Frees all memory previously allocated for the DracView toBeDeleted
void disposeDracView(DracView toBeDeleted) {
    assert(toBeDeleted != NULL);
    assert(toBeDeleted->gV != NULL);
    disposeGameView(toBeDeleted->gV);
    free(toBeDeleted);
}


//// Functions to return simple information about the current state of the game

// Get the current round
Round giveMeTheRound(DracView currentView) {
    return getRound(currentView->gV);
}

// Get the current score
int giveMeTheScore(DracView currentView) {
    return getScore(currentView->gV);
}

// Get the current health points for a given player
int howHealthyIs(DracView currentView, PlayerID player) {
    assert(player >= PLAYER_LORD_GODALMING && player <= PLAYER_DRACULA);
    return getHealth(currentView->gV, player);
}

// Get the current location id of a given player
LocationID whereIs(DracView currentView, PlayerID player) {
    assert(player >= PLAYER_LORD_GODALMING && player <= PLAYER_DRACULA);
    return getLocation(currentView->gV, player);
}

// Get the most recent move of a given player
void lastMove(DracView currentView, PlayerID player,
                 LocationID *start, LocationID *end) {
    LocationID trail[TRAIL_SIZE];
    getHistory(currentView->gV, player, trail);
    *start = trail[TRAIL_SIZE-2];
    *end = trail[TRAIL_SIZE-1];
    return;
}

// Find out what minions are placed at the specified location
void whatsThere(DracView currentView, LocationID where,
                         int *numTraps, int *numVamps) {
    getEncountersData(currentView->gV, where, numTraps, numVamps);
    return;
}

//// Functions that return information about the history of the game

// Fills the trail array with the location ids of the last 6 turns
void giveMeTheTrail(DracView currentView, PlayerID player,
                            LocationID trail[TRAIL_SIZE]) {
    getHistory(currentView->gV,player, trail);
    return;
}

//// Functions that query the map to find information about connectivity

// What are my (Dracula's) possible next moves (locations)
LocationID *whereCanIgo(DracView currentView, int *numLocations, int road, int sea) {
    LocationID trail[TRAIL_SIZE];
    getHistory(currentView->gV, PLAYER_DRACULA, trail);
    LocationID * connections =
        connectedLocations(currentView->gV, numLocations,
                           getLocation(currentView->gV,PLAYER_DRACULA),
                           PLAYER_DRACULA, getRound(currentView->gV),
                           road, FALSE, sea);
    int i;
    int j;
    for (j = 0; j < TRAIL_SIZE - 1; j++) {
        for (i = 0; connections[i] != '\0'; i++) {
            if (trail[j] == connections[i]) {
                connections[i] = NOWHERE;
            }
        }
    }
    return connections;
}

// What are the specified player's next possible moves
LocationID *whereCanTheyGo(DracView currentView, int *numLocations,
                           PlayerID player, int road, int rail, int sea) {
    if (player == PLAYER_DRACULA) {
        return whereCanIgo(currentView, numLocations, road, sea);
    }
    return connectedLocations(currentView->gV, numLocations,
                              getLocation(currentView->gV,player),
                              player, getRound(currentView->gV),
                              road, rail, sea);
}
