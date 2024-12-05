//
// Created by lucas-laviolette on 12/4/24.
//

#ifndef __FreeBSD__
    #include <SDL2/SDL.h>
#endif

#include <controller.h>
#include <player.h>
#include <stdio.h>

// #define DEAD_ZONE 15000

#ifndef __FreeBSD__
SDL_GameController *initializeController(void)
{
    SDL_GameController *controller;

    if(SDL_Init(SDL_INIT_GAMECONTROLLER) != 0)
    {
        printf("Could not initialize controller.\n");
        return NULL;
    }

    if(SDL_NumJoysticks() <= 0)
    {
        printf("No controller detected.\n");
        SDL_Quit();
        return NULL;
    }

    controller = SDL_GameControllerOpen(0);

    if(controller == NULL)
    {
        printf("Could not open controller.\n");
        SDL_Quit();
        return NULL;
    }

    return controller;
}

void closeController(SDL_GameController *controller)
{
    if(controller != NULL)
    {
        SDL_GameControllerClose(controller);
    }
    SDL_Quit();
}

int listenForInput(SDL_GameController const *controller, Player *player)
// void listenForInput(Player *player)
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

    enum JoystickDirection
    {
        LEFTJOYSTICKHORIZONTAL,
        LEFTJOYSTICKVERTICAL,
        RIGHTJOYSTICKHORIZONTAL,
        RIGHTJOYSTICKVERTICAL,
    };

    const Uint32 timeout = 100;
    SDL_Event    event;
    const Uint32 startTime = SDL_GetTicks();

    if(controller == NULL)
    {
        printf("No controller detected.\n");
        return -1;
    }

    while(SDL_GetTicks() - startTime < timeout)
    {
        if(SDL_PollEvent(&event))
        {
            if(event.type == SDL_QUIT)
            {
                printf("Quit event received. Exiting...\n");
                return -1;
            }

            if(event.type == SDL_CONTROLLERBUTTONDOWN || event.type == SDL_CONTROLLERBUTTONUP)
            {
                if(event.cbutton.button == BUTTONDOWN && event.type == SDL_CONTROLLERBUTTONUP)
                {
                    player->y++;
                    return 0;
                }
                if(event.cbutton.button == BUTTONUP && event.type == SDL_CONTROLLERBUTTONUP)
                {
                    player->y--;
                    return 0;
                }
                if(event.cbutton.button == BUTTONLEFT && event.type == SDL_CONTROLLERBUTTONUP)
                {
                    player->x--;
                    return 0;
                }
                if(event.cbutton.button == BUTTONRIGHT && event.type == SDL_CONTROLLERBUTTONUP)
                {
                    player->x++;
                    return 0;
                }
                if(event.cbutton.button == BUTTONLOGO && event.type == SDL_CONTROLLERBUTTONUP)
                {
                    return -1;
                }
            }
        }
        SDL_Delay(1);
    }
    return 1;
}

// int getControllerInput(Player *player)
// // int getControllerInput(void)
// {
//     enum ButtonMapping
//     {
//         BUTTONA,
//         BUTTONB,
//         BUTTONX,
//         BUTTONY,
//         BUTTONLOGO,
//         BUTTONUP = 11,
//         BUTTONDOWN,
//         BUTTONLEFT,
//         BUTTONRIGHT
//     };
//
//     enum JoystickDirection
//     {
//         LEFTJOYSTICKHORIZONTAL,
//         LEFTJOYSTICKVERTICAL,
//         RIGHTJOYSTICKHORIZONTAL,
//         RIGHTJOYSTICKVERTICAL,
//     };
//
//     SDL_Event           event;
//     SDL_GameController *controller = NULL;
//     // int                 xValue         = 0;
//     // int                 yValue         = 0;
//     int positionChange = 0;
//
//     if(SDL_Init(SDL_INIT_GAMECONTROLLER) != 0)
//     {
//         printf("SDL_Init Error: %s\n", SDL_GetError());
//         return EXIT_FAILURE;
//     }
//     if(SDL_NumJoysticks() > 0)
//     {
//         controller = SDL_GameControllerOpen(0);
//         if(!controller)
//         {
//             printf("Could not open game controller:%s\n", SDL_GetError());
//             SDL_Quit();
//             return EXIT_FAILURE;
//         }
//     }
//     else
//     {
//         printf("No game controllers connected.\n");
//         SDL_Quit();
//         return EXIT_FAILURE;
//     }
//
//     while(!positionChange)
//     // while(1)
//     {
//         while(SDL_PollEvent(&event))
//         {
//             if(event.type == SDL_QUIT)
//             {
//                 SDL_GameControllerClose(controller);
//                 SDL_Quit();
//                 return EXIT_SUCCESS;
//             }
//
//             if(event.type == SDL_CONTROLLERBUTTONDOWN || event.type == SDL_CONTROLLERBUTTONUP)
//             {
//                 // printf("Button event: button %d%s\n", event.cbutton.button, event.type == SDL_CONTROLLERBUTTONDOWN ? " pressed" : " released");
//
//                 if(event.cbutton.button == BUTTONDOWN && event.type == SDL_CONTROLLERBUTTONUP)
//                 {
//                     // printf("up\n");
//                     player->y++;
//                     positionChange = 1;
//                     break;
//                 }
//                 if(event.cbutton.button == BUTTONUP && event.type == SDL_CONTROLLERBUTTONUP)
//                 {
//                     // printf("down\n");
//                     player->y--;
//                     positionChange = 1;
//                     break;
//                 }
//                 if(event.cbutton.button == BUTTONLEFT && event.type == SDL_CONTROLLERBUTTONUP)
//                 {
//                     // printf("left\n");
//                     player->x--;
//                     positionChange = 1;
//                     break;
//                 }
//                 if(event.cbutton.button == BUTTONRIGHT && event.type == SDL_CONTROLLERBUTTONUP)
//                 {
//                     // printf("right\n");
//                     player->x++;
//                     positionChange = 1;
//                     break;
//                 }
//             }
//             // Joystick stuff goes here
//         }
//     }
//
//     // Cleanup and exit
//     SDL_GameControllerClose(controller);
//     SDL_Quit();
//     return EXIT_SUCCESS;
// }

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
#endif
