// HunterView.c ... HunterView ADT implementation

#include <stdlib.h>
#include <assert.h>
#include "Globals.h"
#include "Game.h"
#include "GameView.h"
#include "HunterView.h"
// #include "Map.h" ... if you decide to use the Map ADT

struct hunterView {
    GameView gV;                    // generated by newGameView()
    PlayerID player;                // ID of the player
};


// Creates a new HunterView to summarise the current state of the game
HunterView newHunterView(char *pastPlays, PlayerMessage messages[]) {
    HunterView hunterView = malloc(sizeof(struct hunterView));
    hunterView->gV = newGameView(pastPlays, messages);
    hunterView->player = getCurrentPlayer(hunterView->gV);
    return hunterView;
}


// Frees all memory previously allocated for the HunterView toBeDeleted
void disposeHunterView(HunterView toBeDeleted) {
    assert(toBeDeleted != NULL);
    assert(toBeDeleted->gV != NULL);
    disposeGameView(toBeDeleted->gV);
    free(toBeDeleted);
}


//// Functions to return simple information about the current state of the game

// Get the current round
Round giveMeTheRound(HunterView currentView) {
    return getRound(currentView->gV);
}

// Get the id of current player
PlayerID whoAmI(HunterView currentView) {
    return currentView->player;
}

// Get the current score
int giveMeTheScore(HunterView currentView) {
    return getScore(currentView->gV);
}

// Get the current health points for a given player
int howHealthyIs(HunterView currentView, PlayerID player) {
    assert(player >= PLAYER_LORD_GODALMING && player <= PLAYER_DRACULA);
    return getHealth(currentView->gV, player);
}

// Get the current location id of a given player
LocationID whereIs(HunterView currentView, PlayerID player) {
    assert(player >= PLAYER_LORD_GODALMING && player <= PLAYER_DRACULA);
    return getLocation(currentView->gV, player);
}

//// Functions that return information about the history of the game

// Fills the trail array with the location ids of the last 6 turns
void giveMeTheTrail(HunterView currentView, PlayerID player,
                            LocationID trail[TRAIL_SIZE]) {
    return getHistory(currentView->gV, player, trail);
}

//// Functions that query the map to find information about connectivity

// What are my possible next moves (locations)
LocationID *whereCanIgo(HunterView currentView, int *numLocations,
                        int road, int rail, int sea) {
    return connectedLocations(currentView->gV, numLocations,
                              getLocation(currentView->gV,currentView->player),
                              currentView->player, getRound(currentView->gV),
                              road, rail, sea);
}

// What are the specified player's next possible moves
LocationID *whereCanTheyGo(HunterView currentView, int *numLocations,
                           PlayerID player, int road, int rail, int sea) {
    if (player == PLAYER_DRACULA                             &&
        (getLocation(currentView->gV, player) < ADRIATIC_SEA &&
         getLocation(currentView->gV, player) > ZURICH)        ) {
        return connectedLocations(currentView->gV, numLocations,
                                  getLocation(currentView->gV,currentView->player),
                                  currentView->player, getRound(currentView->gV),
                                  FALSE, FALSE, FALSE);
    }
    return connectedLocations(currentView->gV, numLocations,
                              getLocation(currentView->gV,currentView->player),
                              currentView->player, getRound(currentView->gV),
                              road, rail, sea);
}
