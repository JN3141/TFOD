// encounter.h
// Extra Interface for encounters shared with GameView and DracView
// COMP1927 16s2

#ifndef ENCOUNTER_H
#define ENCOUNTER_H

#include "Globals.h"
#include "Game.h"
#include "Places.h"
#include "GameView.h"

typedef struct _encounters *Encounters;

Encounters getEncounters(GameView currentView);

void getEncountersData(GameView currentView, LocationID where,
                         int *numTraps, int *numVamps);

#endif
