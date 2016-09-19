// encounter.c ... encounter ADT implementation

#include <stdlib.h>
#include <assert.h>
#include "Globals.h"
#include "Game.h"
#include "GameView.h"
#include "encounter.h"

typedef struct _encounters {
    LocationID location;
    char type;
} encounters;

encounters eTrail[TRAIL_SIZE-1];

Encounters getEncounters(GameView currentView) {
    return (Encounters)(currentView->eTrail);
}

void getEncountersData(GameView currentView, LocationID where,
                         int *numTraps, int *numVamps) {
    int j;
    for (j = 0; j < TRAIL_SIZE - 1; j++) {
        if (currentView->eTrail[j].location == where) {
            if (currentView->eTrail[j].type == 'T') {
                (*numTraps)++;
            } else if (currentView->eTrail[j].type == 'V') {
                (*numVamps)++;
            }
        }
    }
    return;
}
