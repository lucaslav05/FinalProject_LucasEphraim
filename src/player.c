//
// Created by lucas-laviolette on 11/17/24.
//

#include <ncurses.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>

//States to implement
/*
INIT
WAIT_FOR_CONNECTION
GAME_LOOP
GAME_OVER
CLEANUP
 */
enum State {INIT, WAIT_FOR_CONNECTION, GAME_LOOP, GAME_OVER, CLEANUP};

int main(void)
{
    enum State currentState = INIT;
    int udpSocket;
    struct sockaddr_in opponentAddr;
    int player1_x;
    int player1_y;
    int player2_x;
    int player2_y;
    char message[256];

    while (currentState != CLEANUP)
        switch (currentState) {
            case INIT:
                udpSocket = setup_udp_socket();
                init_ncurses();
                initialize_positions(&player1_x, &player1_y, &player2_x, &player2_y);
                currentState = WAIT_FOR_CONNECTION;
                break;

            case WAIT_FOR_CONNECTION:
                if (wait_for_opponent_message(udpSocket, &opponentAddr, message)) {
                    parse_opponent_info(message, &player2_x, &player2_y);
                    currentState = GAME_LOOP;
                }
                break;

            case GAME_LOOP:
                clear_screen();
                draw_dot(player1_x, player1_y);
                draw_dot(player2_x, player2_y);
                refresh_screen();

                if (read_input(&player1_x, &player1_y)) {
                    char serializedPos[32];
                    serialize_position(player1_x, player1_y, serializedPos);
                    send_position(udpSocket, &opponentAddr,serializedPos);
                }

                if(receive_position(udpSocket, message)) {
                    update_opponent_position(message, &player2_x, &player2_y);
                }

                if(check_game_over(player1_x, player1_y, player2_x, player2_y)) {
                    currentState = GAME_OVER;
                }
                break;

            case GAME_OVER:
                display_game_over_message();
                currentState = CLEANUP;
                break;

            case CLEANUP:
                end_ncurses();
                close_socket(udpSocket);
                break;
        }



    return EXIT_SUCCESS;
}

//Need to write all the functions