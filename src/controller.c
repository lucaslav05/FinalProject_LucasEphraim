//
// Created by lucas-laviolette on 12/4/24.
//

#include <SDL2/SDL.h>
#include <controller.h>
#include <player.h>
#include <stdio.h>

// #define DEAD_ZONE 15000

int getControllerInput(Player *player)
{
    enum ButtonMapping
    {
        BUTTONA,
        BUTTONB,
        BUTTONX,
        BUTTONY,
        BUTTONLOGO,
        BUTTONUP = 11,
        BUTTONDOWN,
        BUTTONLEFT,
        BUTTONRIGHT
    };

    SDL_Event           event;
    SDL_GameController *controller     = NULL;
    int                 positionChange = 0;

    if(SDL_Init(SDL_INIT_GAMECONTROLLER) != 0)
    {
        printf("SDL_Init Error: %s\n", SDL_GetError());
        return EXIT_FAILURE;
    }
    if(SDL_NumJoysticks() > 0)
    {
        controller = SDL_GameControllerOpen(0);
        if(!controller)
        {
            printf("Could not open game controller:%s\n", SDL_GetError());
            SDL_Quit();
            return EXIT_FAILURE;
        }
    }
    else
    {
        printf("No game controllers connected.\n");
        SDL_Quit();
        return EXIT_FAILURE;
    }

    while(!positionChange)
    {
        // Handle events but don't block
        while(SDL_PollEvent(&event))
        {
            if(event.type == SDL_QUIT)
            {
                SDL_GameControllerClose(controller);
                SDL_Quit();
                return EXIT_SUCCESS;
            }

            if(event.type == SDL_CONTROLLERBUTTONDOWN || event.type == SDL_CONTROLLERBUTTONUP)
            {
                if(event.cbutton.button == BUTTONDOWN && event.type == SDL_CONTROLLERBUTTONDOWN)
                {
                    player->y++;
                    positionChange = 1;
                    break;
                }
                if(event.cbutton.button == BUTTONUP && event.type == SDL_CONTROLLERBUTTONDOWN)
                {
                    player->y--;
                    positionChange = 1;
                    break;
                }
                if(event.cbutton.button == BUTTONLEFT && event.type == SDL_CONTROLLERBUTTONDOWN)
                {
                    player->x--;
                    positionChange = 1;
                    break;
                }
                if(event.cbutton.button == BUTTONRIGHT && event.type == SDL_CONTROLLERBUTTONDOWN)
                {
                    player->x++;
                    positionChange = 1;
                    break;
                }
            }
        }
    }

    // Cleanup and exit
    SDL_GameControllerClose(controller);
    SDL_Quit();
    return EXIT_SUCCESS;
}

//     if(event.type == SDL_CONTROLLERAXISMOTION)
//     {
//         const int axis  = event.caxis.axis;
//         const int value = event.caxis.value;
//
//         if(axis == 0 || axis == 2)
//         {
//             xValue = value;
//         }
//         else if(axis == 1 || axis == 3)
//         {
//             yValue = value;
//         }
//
//         if(abs(xValue) > DEAD_ZONE && abs(yValue) > DEAD_ZONE)
//         {
//             if(xValue > 0 && yValue < 0)
//             {
//                 printf("up-right\n");
//             }
//             else if(xValue < 0 && yValue < 0)
//             {
//                 printf("up-left\n");
//             }
//             else if(xValue > 0 && yValue > 0)
//             {
//                 printf("down-right\n");
//             }
//             else if(xValue < 0 && yValue > 0)
//             {
//                 printf("down-left\n");
//             }
//         }
//         else if(abs(xValue) > DEAD_ZONE)
//         {
//             if(xValue > 0)
//             {
//                 printf("right\n");
//             }
//             else
//             {
//                 printf("left\n");
//             }
//         }
//         else if(abs(yValue) > DEAD_ZONE)
//         {
//             if(yValue < 0)
//             {
//                 printf("up\n");
//             }
//             else
//             {
//                 printf("down\n");
//             }
//         }
//         else
//         {
//             // Default case if no axis matched
//             printf("DEAD_ZONE\n");
//         }
//     }
// }