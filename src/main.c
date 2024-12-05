#include <arpa/inet.h>
#include <fcntl.h>
#include <ncurses.h>
#include <netinet/in.h>
#include <player.h>
#include <poll.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#ifndef __FreeBSD__
    #include <SDL2/SDL_gamecontroller.h>
    #include <controller.h>
#endif

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))

#define WIN_HEIGHT 20
#define WIN_WIDTH 40
#define WIN_Y 5
#define WIN_X 5

#define SERVER_PORT 9999

#define TIMEOUT 100

#define BASE 10

#define PLAYER1_X 10
#define PLAYER1_Y 20
#define PLAYER2_X 20
#define PLAYER2_Y 10

int  setup_controller_input(void);
void draw_box(WINDOW *win);
void enforce_boundaries(Player *player);
int  setup_udp_socket(uint16_t port);
void handle_timer_movement(Player *player);

int setup_controller_input(void)
{
    int fd = open("/dev/input/js0", O_CLOEXEC);
    if(fd == -1)
    {
        perror("Error opening controller input");
        return -1;
    }
    return fd;
}

// Draw the game box
void draw_box(WINDOW *win)
{
    box(win, 0, 0);
    mvwprintw(win, 0, 2, "Dot game");
    wrefresh(win);
    refresh();
}

// Ensure the player stays within boundaries
void enforce_boundaries(Player *player)
{
    player->x = MAX(WIN_X + 1, MIN(WIN_X + WIN_WIDTH - 2, player->x));
    player->y = MAX(WIN_Y + 1, MIN(WIN_Y + WIN_HEIGHT - 2, player->y));
}

// Set up the UDP socket
int setup_udp_socket(uint16_t port)
{
    int                flags;
    int                sockfd;
    struct sockaddr_in server_addr;

    sockfd = socket(AF_INET, SOCK_DGRAM | SOCK_CLOEXEC, 0);
    if(sockfd == -1)
    {
        perror("Socket creation failed");
        return -1;
    }

    flags = fcntl(sockfd, F_GETFL, 0);
    fcntl(sockfd, F_SETFL, flags | O_NONBLOCK);

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family      = AF_INET;
    server_addr.sin_port        = htons(port);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if(bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("Bind failed");
        close(sockfd);
        return -1;
    }

    return sockfd;
}

void handle_timer_movement(Player *player)
{
    int direction = (int)arc4random() % 4;    // Randomly pick direction
    mvaddch(player->y, player->x, ' ');       // Clear old position
    switch(direction)
    {
        case 0:
            player->y--;
            break;    // UP
        case 1:
            player->y++;
            break;    // DOWN
        case 2:
            player->x--;
            break;    // LEFT
        case 3:
            player->x++;
            break;    // RIGHT
        default:
            break;
    }
    enforce_boundaries(player);
    mvaddch(player->y, player->x, 'o');    // Draw updated position
}

int main(int argc, char *argv[])
{
    int                controller_fd;
    int                urandom;
    unsigned int       seed;
    struct pollfd      fds[2];
    struct sockaddr_in remote_addr;
    int                sockfd;
    uint16_t           remote_port;
    uint16_t           local_port;
    WINDOW            *win;
    int                opt;
    int                player_number = 0;
    int                input_method  = 0;
    char              *remote_ip     = NULL;
    Player             local_player;
    Player             remote_player;
    int                running = 1;

#ifndef __FreeBSD__
    SDL_GameController *controller = NULL;
#endif

    while((opt = getopt(argc, argv, "p:m:i:")) != -1)
    {
        char *endptr;
        switch(opt)
        {
            case 'p':
                player_number = (int)strtol(optarg, &endptr, BASE);
                if(*endptr != '\0' || player_number <= 0 || player_number >= 3)
                {
                    fprintf(stderr, "Invalid player number: %s\n", optarg);
                    return 1;
                }
                break;
            case 'm':
                if(strcmp(optarg, "keyboard") == 0)
                {
                    input_method = 1;
                }
                else if(strcmp(optarg, "controller") == 0)
                {
#ifndef __FreeBSD__
                    input_method = 3;
#else
                    fprintf(stderr, "Controller input is not supported on FreeBSD.\n");
                    return 1;
#endif
                }
                else
                {
                    input_method = 2;
                }
                break;
            case 'i':
                remote_ip = optarg;
                break;
            default:
                fprintf(stderr, "Usage: %s -p <player_number> -m <keyboard|timer> -i <remote_ip>\n", argv[0]);
                return 1;
        }
    }

#ifndef __FreeBSD__
    if(input_method == 3)
    {
        controller    = initializeController();
        controller_fd = setup_controller_input();

        if(controller_fd == -1)
        {
            return 1;
        }
    }
    else
    {
        controller_fd = -1;
    }
#else
    if(input_method == 3)
    {
        fprintf(stderr, "Controller input is not supported on FreeBSD.\n");
        return 1;
    }
    controller_fd = -1;
#endif
    if(!player_number || !remote_ip)
    {
        fprintf(stderr, "Player number and remote IP are required.\n");
        return 1;
    }

    // Set up the screen
    initscr();
    cbreak();
    keypad(stdscr, true);
    noecho();
    curs_set(0);
    win = newwin(WIN_HEIGHT, WIN_WIDTH, WIN_Y, WIN_X);
    draw_box(win);

    urandom = open("/dev/urandom", O_CLOEXEC);
    if(urandom >= 0)
    {
        read(urandom, &seed, sizeof(seed));
        close(urandom);
        srand(seed);
    }

    // Setup UDP socket
    local_port  = SERVER_PORT;
    remote_port = SERVER_PORT;
    sockfd      = setup_udp_socket(local_port);
    if(sockfd < 0)
    {
        endwin();
        return 1;
    }

    memset(&remote_addr, 0, sizeof(remote_addr));
    remote_addr.sin_family = AF_INET;
    remote_addr.sin_port   = htons(remote_port);
    inet_aton(remote_ip, &remote_addr.sin_addr);

    // Poll setup
    fds[0].fd     = sockfd;
    fds[0].events = POLLIN;
    if(input_method == 3)
    {
        fds[1].fd = controller_fd;
    }
    else
    {
        fds[1].fd = STDIN_FILENO;
    }
    fds[1].events = POLLIN;

    // Initialize shared memory with starting positions
    if(player_number == 1)
    {
        local_player.x  = PLAYER1_X;
        local_player.y  = PLAYER1_Y;
        remote_player.x = PLAYER2_X;
        remote_player.y = PLAYER2_Y;
    }
    else
    {
        local_player.x  = PLAYER2_X;
        local_player.y  = PLAYER2_Y;
        remote_player.x = PLAYER1_X;
        remote_player.y = PLAYER1_Y;
    }

    mvaddch(local_player.y, local_player.x, 'o');
    mvaddch(remote_player.y, remote_player.x, 'x');
    draw_box(win);
    refresh();

    // Main game loop
    while(running)
    {
        int ret = poll(fds, 2, TIMEOUT);

        if(ret > 0)
        {
            if(fds[0].revents & POLLIN)
            {
                struct sockaddr_in recv_addr;
                int                last_y;
                int                last_x;
                socklen_t          addr_len = sizeof(recv_addr);

                last_y = remote_player.y;
                last_x = remote_player.x;

                if(recvfrom(sockfd, &remote_player, sizeof(Player), 0, (struct sockaddr *)&recv_addr, &addr_len) > 0)
                {
                    mvaddch(last_y, last_x, ' ');
                    mvaddch(remote_player.y, remote_player.x, 'x');    // Update remote position
                    refresh();
                }
            }

            if(fds[1].revents & POLLIN)
            {
                if(input_method == 1)
                {
                    const int ch = getch();                          // Read the keyboard input
                    mvaddch(local_player.y, local_player.x, ' ');    // Clear old position
                    switch(ch)
                    {
                        case KEY_UP:
                            local_player.y--;
                            break;
                        case KEY_DOWN:
                            local_player.y++;
                            break;
                        case KEY_LEFT:
                            local_player.x--;
                            break;
                        case KEY_RIGHT:
                            local_player.x++;
                            break;
                        case 'q':
                            running = 0;
                            break;
                        default:
                            break;
                    }
                }
#ifndef __FreeBSD__
                else if(input_method == 3)
                {
                    int controllerStatus;

                    mvaddch(local_player.y, local_player.x, ' ');

                    controllerStatus = listenForInput(controller, &local_player);

                    if(controllerStatus == -1)
                    {
                        running = 0;
                    }

                    if(controllerStatus == 0)
                    {
                        enforce_boundaries(&local_player);
                        mvaddch(local_player.y, local_player.x, 'o');    // Draw updated position
                        sendto(sockfd, &local_player, sizeof(Player), 0, (struct sockaddr *)&remote_addr, sizeof(remote_addr));
                        refresh();
                    }
                }
#endif
                enforce_boundaries(&local_player);
                mvaddch(local_player.y, local_player.x, 'o');    // Draw updated position
                sendto(sockfd, &local_player, sizeof(Player), 0, (struct sockaddr *)&remote_addr, sizeof(remote_addr));
                refresh();
            }
        }

        // Timer movement logic
        if(input_method == 2 && time(NULL) % 2 == 0)
        {
            handle_timer_movement(&local_player);
            sendto(sockfd, &local_player, sizeof(Player), 0, (struct sockaddr *)&remote_addr, sizeof(remote_addr));
        }

        refresh();
    }

#ifndef __FreeBSD__
    closeController(controller);
#endif

    // Cleanup
    close(sockfd);
    endwin();
    return 0;
}
