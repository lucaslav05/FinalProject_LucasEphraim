//
// Created by lucas-laviolette on 12/4/24.
//

#ifndef CONTROLLER_H
#define CONTROLLER_H
#include "player.h"
#ifndef __FreeBSD__
#include <SDL2/SDL.h>
#endif

SDL_GameController *initializeController(void);

int listenForInput(SDL_GameController const *controller, Player *player);
// void listenForInput(Player *player);

void closeController(SDL_GameController *controller);

int getControllerInput(Player *player);
// int getControllerInput(void);

#endif    // CONTROLLER_H
