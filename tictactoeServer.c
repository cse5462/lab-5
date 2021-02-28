/***********************************************************/
/* This program is a 'net-enabled' version of tictactoe.   */
/* Two users, Player 1 and Player 2, send moves back and   */
/* forth, between two computers.                           */
/***********************************************************/

/* #include files go here */
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <strings.h>
#include <netdb.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

/* The protocol version number used. */
#define VERSION 2

/* The number of command line arguments. */
#define NUM_ARGS 2
/* The maximum size of a buffer for the program. */
#define BUFFER_SIZE 100
/* The error code used to signal an invalid move. */
#define ERROR_CODE -1
/* The number of seconds spend waiting before a timeout. */
#define TIMEOUT 15
/* The number of rows for the TicIacToe board. */
#define ROWS 3
/* The number of columns for the TicIacToe board. */
#define COLUMNS 3

/*******************/
/* PLAYER COMMANDS */
/*******************/
/* The command to begin a new game. */
#define NEW_GAME 0x00
/* The command to issue a move. */
#define MOVE 0x01

/* Structure to send and recieve player datagrams. */
struct Buffer {
    char version;   // version number
    char command;   // player command
    char data;      // data for command if applicable
};

void print_error(const char *msg, int errnum, int terminate);
void handle_init_error(const char *msg, int errnum);
void extract_args(char *argv[], int *port);
void print_server_info(const struct sockaddr_in *serverAddr);
int create_endpoint(struct sockaddr_in *socketAddr, unsigned long address, int port);
void set_timeout(int sd, int seconds);
int new_game(int sd, struct sockaddr_in *playerAddr);
void init_shared_state(char board[ROWS][COLUMNS]);
int check_win(char board[ROWS][COLUMNS]);
void print_board(char board[ROWS][COLUMNS]);
int validate_choice(int choice, char board[ROWS][COLUMNS]);
int get_p1_choice();
int get_p2_choice(int sd, const struct sockaddr_in *playerAddr);
int send_p1_move(int sd, const struct sockaddr_in *playerAddr, int move);
int get_player_choice(int sd, const struct sockaddr_in *playerAddr, char board[ROWS][COLUMNS], int player);
void tictactoe(int sd, const struct sockaddr_in *playerAddr, char board[ROWS][COLUMNS]);

/**
 * @brief This program creates and sets up a TicTacToe server which acts as Player 1 in a
 * 2-player game of TicTacToe. This server creates a server socket for the clients to communicate
 * with, listens for remote client UDP DAGAGRAM packets, and then initiates a simple
 * game of TicTacToe in which Player 1 and Player 2 take turns making moves which they send to
 * the other player. If an error occurs before the "New Game" command is received, the program
 * terminates and prints appropriate error messages, otherwise an error message is printed and
 * the program searches for a new player waiting to play.
 * 
 * @param argc Non-negative value representing the number of arguments passed to the program
 * from the environment in which the program is run.
 * @param argv Pointer to the first element of an array of argc + 1 pointers, of which the
 * last one is NULL and the previous ones, if any, point to strings that represent the
 * arguments passed to the program from the host environment. If argv[0] is not a NULL
 * pointer (or, equivalently, if argc > 0), it points to a string that represents the program
 * name, which is empty if the program name is not available from the host environment.
 * @return If the return statement is used, the return value is used as the argument to the
 * implicit call to exit(). The values zero and EXIT_SUCCESS indicate successful termination,
 * the value EXIT_FAILURE indicates unsuccessful termination.
 */
int main(int argc, char *argv[]) {
    int sd, portNumber;
    char board[ROWS][COLUMNS];
    struct sockaddr_in serverAddress;

    /* If arg count correct, extract arguments to their respective variables */
    if (argc != NUM_ARGS) handle_init_error("argc: Invalid number of command line arguments", 0);
    extract_args(argv, &portNumber);

    /* Create server socket and print server information */
    sd = create_endpoint(&serverAddress, INADDR_ANY, portNumber);
    print_server_info(&serverAddress);

    /* Play the TicTacToe game when a player asks for one */
    int newGame = 1;
    while (1) {
        struct sockaddr_in clientAddress;
        if (newGame) printf("[+]Waiting for Player 2 to join...\n");
        /* Remove timout when waiting for new player */
        if (newGame) set_timeout(sd, 0);
        /* Wait for a player to issue "New Game" comamnd */
        if ((newGame = new_game(sd, &clientAddress))) {
            /* Set timout once player has started a new game */
            set_timeout(sd, TIMEOUT);
            /* Initialize the 'game' board and start the 'game' */
            init_shared_state(board);
            tictactoe(sd, &clientAddress, board);
            printf("[+]The game has ended.\n");
        }
    }

    return 0;
}

/**
 * @brief Prints the provided error message and corresponding errno message (if present) and
 * terminates the process if asked to do so.
 * 
 * @param msg The error description message to display.
 * @param errnum This is the error number, usually errno.
 * @param terminate Whether or not the process should be terminated.
 */
void print_error(const char *msg, int errnum, int terminate) {
    /* Check for valid error code and generate error message */
    if (errnum) {
        printf("ERROR: %s: %s\n", msg, strerror(errnum));
    } else {
        printf("ERROR: %s\n", msg);
    }
    /* Exits process if it should be terminated */
    if (terminate) exit(EXIT_FAILURE);
}

/**
 * @brief Prints a string describing the initialization error and provided error number (if
 * nonzero), the correct command usage, and exits the process signaling unsuccessful termination. 
 * 
 * @param msg The error description message to display.
 * @param errnum This is the error number, usually errno.
 */
void handle_init_error(const char *msg, int errnum) {
    print_error(msg, errnum, 0);
    printf("Usage is: tictactoeP1 <remote-port>\n");
    /* Exits the process signaling unsuccessful termination */
    exit(EXIT_FAILURE);
}

/**
 * @brief Extracts the user provided arguments to their respective local variables and performs
 * validation on their formatting. If any errors are found, the function terminates the process.
 * 
 * @param argv Pointer to the first element of an array of argc + 1 pointers, of which the
 * last one is NULL and the previous ones, if any, point to strings that represent the
 * arguments passed to the program from the host environment. If argv[0] is not a NULL
 * pointer (or, equivalently, if argc > 0), it points to a string that represents the program
 * name, which is empty if the program name is not available from the host environment.
 * @param port The remote port number that the server should listen on
 */
void extract_args(char *argv[], int *port) {
    /* Extract and validate remote port number */
    *port = strtol(argv[1], NULL, 10);
    if (*port < 1 || *port != (u_int16_t)(*port)) handle_init_error("remote-port: Invalid port number", 0);
}

/**
 * @brief Prints the server information needed for the client to comminicate with the server.
 * 
 * @param serverAddr The socket address structure for the server comminication endpoint.
 */
void print_server_info(const struct sockaddr_in *serverAddr) {
    int hostname;
    char hostbuffer[BUFFER_SIZE], *IP_addr;
    struct hostent *host_entry;

    /* Retrieve the hostname */
    if ((hostname = gethostname(hostbuffer, sizeof(hostbuffer))) == -1) {
        print_error("print_server_info: gethostname", errno, 1);
    }
    /* Retrieve the host information */
    if ((host_entry = gethostbyname(hostbuffer)) == NULL) {
        print_error("print_server_info: gethostbyname", errno, 1);
    }
    /* Convert the host internet network address to an ASCII string */
    IP_addr = inet_ntoa(*((struct in_addr *)host_entry->h_addr_list[0]));
    /* Print the IP address and port number for the server */
    printf("Server listening at %s on port %hu\n", IP_addr, serverAddr->sin_port);
}

/**
 * @brief Creates the comminication endpoint with the provided IP address and port number. If any
 * errors are found, the function terminates the process.
 * 
 * @param socketAddr The socket address structure created for the comminication endpoint.
 * @param address The IP address for the socket address structure.
 * @param port The port number for the socket address structure.
 * @return The socket descriptor of the created comminication endpoint.
 */
int create_endpoint(struct sockaddr_in *socketAddr, unsigned long address, int port) {
    int sd;
    /* Create socket */
    if ((sd = socket(AF_INET, SOCK_DGRAM, 0)) != -1) {
        socketAddr->sin_family = AF_INET;
        /* Assign IP address to socket */
        socketAddr->sin_addr.s_addr = address;
        /* Assign port number to socket */
        socketAddr->sin_port = htons(port);
    } else {
        print_error("create_endpoint: socket", errno, 1);
    }
    /* Bind socket to communication endpoint */
    if (bind(sd, (struct sockaddr *)socketAddr, sizeof(struct sockaddr_in)) == 0) {
        printf("[+]Server socket created successfully.\n");
    } else {
        print_error("create_endpoint: bind", errno, 1);
    }

    return sd;
}

/**
 * @brief Sets the time to wait before a timeout on recvfrom calls to the specified number
 * of seconds, or turns it off if zero seconds was entered.
 * 
 * @param sd The socket descriptor of the server comminication endpoint.
 * @param seconds The number of seconds to wait before a timeout.
 */
void set_timeout(int sd, int seconds) {
    struct timeval time = {0};
    time.tv_sec = seconds;

    /* Sets the recvfrom timeout option */
    if (setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO, &time, sizeof(time)) < 0) {
        print_error("set_timeout", errno, 0);
    }
}

/**
 * @brief Recieves a datagram from another player and checks to see if it is a valid
 * "New Game" request.
 * 
 * @param sd The socket descriptor of the server comminication endpoint.
 * @param playerAddr 
 * @return True if a "New Game" request has been received, false otherwise. 
 */
int new_game(int sd, struct sockaddr_in *playerAddr) {
    int rv;
    struct Buffer recvBuffer = {0};
    socklen_t fromLength = sizeof(struct sockaddr);

    /* Receive message and address from another player */
    if ((rv = recvfrom(sd, &recvBuffer, sizeof(recvBuffer), 0, (struct sockaddr *)playerAddr, &fromLength)) < 2) {
        if (rv < 0) print_error("new_game", errno, 0);
        return 0;
    } else {
        /* Check if message if a vlid "New Game" request */
        if (recvBuffer.version == VERSION && recvBuffer.command == NEW_GAME) {
            printf("Player 2 at address %s has requested a new game\n", inet_ntoa(playerAddr->sin_addr));
            return 1;
        } else {
            return 0;
        }
    }
}

/**
 * @brief Initializes the starting state of the game board that both players start with.
 * 
 * @param board The array representing the current state of the game board.
 */
void init_shared_state(char board[ROWS][COLUMNS]) {    
    int i, j, count = 1;
    printf("[+]Initializing shared game board.\n");
    /* Initializes the shared state (aka the board)  */
    for (i = 0; i < 3; i++) {
        for (j = 0; j < 3; j++) {
            board[i][j] = count++ + '0';
        }
    }
}

/**
 * @brief Determines if someone has won the game yet or not.
 * 
 * @param board The array representing the current state of the game board.
 * @return The value 1 if a player has won the game, 0 it the game was a draw, and -1 if the
 * game is still going on. 
 */
int check_win(char board[ROWS][COLUMNS]) {
    /**************************************************************************/
    /* Brute force check to see if someone won, or if there is a draw. Return */
    /* a 0 or 1 if the game is 'over' or return -1 if game should go on       */
    /**************************************************************************/
    if (board[0][0] == board[0][1] && board[0][1] == board[0][2]) { // row matches
        return 1;   // return of 1 mean someone won -> game over
    } else if (board[1][0] == board[1][1] && board[1][1] == board[1][2]) { // row matches
        return 1;     
    } else if (board[2][0] == board[2][1] && board[2][1] == board[2][2]) { // row matches
        return 1;     
    } else if (board[0][0] == board[1][0] && board[1][0] == board[2][0]) { // column matches
        return 1;    
    } else if (board[0][1] == board[1][1] && board[1][1] == board[2][1]) { // column matches
        return 1;  
    } else if (board[0][2] == board[1][2] && board[1][2] == board[2][2]) { // column matches
        return 1;
    } else if (board[0][0] == board[1][1] && board[1][1] == board[2][2]) { // diagonal matches
        return 1;    
    } else if (board[2][0] == board[1][1] && board[1][1] == board[0][2]) { // diagonal matches
        return 1;    
    } else if (board[0][0] != '1' && board[0][1] != '2' && board[0][2] != '3' &&
                board[1][0] != '4' && board[1][1] != '5' && board[1][2] != '6' && 
                board[2][0] != '7' && board[2][1] != '8' && board[2][2] != '9') {   // draw
        return 0;   // return of 0 means draw -> game over
    } else {
        return -1;  // return of -1 means keep playing
    }
}

/**
 * @brief Prints out the current state of the game board nicely formatted.
 * 
 * @param board The array representing the current state of the game board.
 */
void print_board(char board[ROWS][COLUMNS]) {
    /*****************************************************************/
    /* Brute force print out the board and all the squares/values    */
    /*****************************************************************/
    /* Print header info */
    printf("\n\n\n\tCurrent TicTacToe Game\n\n");
    printf("Player 1 (X)  -  Player 2 (O)\n\n\n");
    /* Print current state of board */
    printf("     |     |     \n");
    printf("  %c  |  %c  |  %c \n", board[0][0], board[0][1], board[0][2]);
    printf("_____|_____|_____\n");
    printf("     |     |     \n");
    printf("  %c  |  %c  |  %c \n", board[1][0], board[1][1], board[1][2]);
    printf("_____|_____|_____\n");
    printf("     |     |     \n");
    printf("  %c  |  %c  |  %c \n", board[2][0], board[2][1], board[2][2]);
    printf("     |     |     \n\n");
}

/**
 * @brief Determines whether a given move is legal (i.e. number 1-9) and valid (i.e. hasn't
 * already been played) for the current game.
 * 
 * @param choice The player move to be validated.
 * @param board The array representing the current state of the game board.
 * @return True if the given move if valid based on the current board, false otherwise. 
 */
int validate_choice(int choice, char board[ROWS][COLUMNS]) {
    int row, column;
    /* Check to see if the choice is a move on the board */
    if (choice < 1 || choice > 9) {
        print_error("Invalid move: Must be a number [1-9]", 0, 0);
        return 0;
    }
    /* Check to see if the row/column chosen has a digit in it, if */
    /* square 8 has an '8' then it is a valid choice */
    row = (int)((choice-1) / ROWS); 
    column = (choice-1) % COLUMNS;
    if (board[row][column] != (choice + '0')) {
        print_error("Invalid move: Square already taken", 0, 0);
        return 0;
    }
    return 1;
}

/**
 * @brief Gets Player 1's next move.
 * 
 * @return The integer for the square that Player 1 would like to move to. 
 */
int get_p1_choice() {
    int pick = 0;
    char input[BUFFER_SIZE];
    printf("Player 1, enter a number:  ");
    /* Read line of user input */
    fgets(input, sizeof(input), stdin);
    /* Look for integer in input for player's move */
    sscanf(input, "%d", &pick);
    return pick;
}

/**
 * @brief Gets Player 2's next move.
 * 
 * @param sd The socket descriptor of the server comminication endpoint.
 * @param playerAddr The socket address structure for the remote player comminication endpoint.
 * @return The integer for the square that Player 2 would like to move to, or an error code. 
 */
int get_p2_choice(int sd, const struct sockaddr_in *playerAddr) {
    int rv;
    struct Buffer recvBuffer = {0};
    struct sockaddr_in clientAddr = {0};
    socklen_t fromLength = sizeof(struct sockaddr_in);
    uint32_t pAddr = playerAddr->sin_addr.s_addr;
    uint16_t pPort = playerAddr->sin_port;

    printf("Waiting for Player 2 to make a move...\n");
    /* Throw away messages not from player who asked for the game */
    while (clientAddr.sin_addr.s_addr != pAddr || clientAddr.sin_port != pPort) {
        /* Get move from remote player */
        if ((rv = recvfrom(sd, &recvBuffer, sizeof(recvBuffer), 0, (struct sockaddr *)&clientAddr, &fromLength)) <= 0) {
            /* If error occured, check if it was a timeout */
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                print_error("get_p2_choice: Player ran out of time to respond", 0, 0);
                return ERROR_CODE;
            } else {
                if (clientAddr.sin_addr.s_addr == pAddr && clientAddr.sin_port == pPort) {
                    print_error("get_p2_choice", errno, 0);
                    return ERROR_CODE;
                }
            }
        }
    }
    /* Check that the datagram received from the other player was valid */
    if (recvBuffer.version != VERSION || recvBuffer.command != MOVE) {
        if (recvBuffer.version != VERSION) print_error("get_p2_choice: Protocol version not supported", 0, 0);
        if (recvBuffer.command != MOVE) print_error("get_p2_choice: Expected a MOVE command", 0, 0);
        return ERROR_CODE;
    }
    printf("Player 2 chose:  %c\n", recvBuffer.data);
    return (recvBuffer.data - '0');
}

/**
 * @brief Sends Player 1's move to the remote player.
 * 
 * @param sd The socket descriptor of the server comminication endpoint.
 * @param playerAddr The socket address structure for the remote player comminication endpoint.
 * @param move The move to be sent to the remote player.
 * @return The move that was sent, or an error code if there was an issue. 
 */
int send_p1_move(int sd, const struct sockaddr_in *playerAddr, int move) {
    /* Pack move information into structure for datagram */
    struct Buffer sendBuffer = {0};
    sendBuffer.version = VERSION;
    sendBuffer.command = MOVE;
    sendBuffer.data = move + '0';
    /* Send the move to the remote player */
    if (sendto(sd, &sendBuffer, sizeof(sendBuffer), 0, (struct sockaddr *)playerAddr, sizeof(struct sockaddr_in)) < 0) {
        print_error("send_move", errno, 0);
        return ERROR_CODE;
    }
    return move;
}

/**
 * @brief Gets the choice from either Player 1 or 2. If the choice came from Player 1,
 * it also send this choice to the other player.
 * 
 * @param sd The socket descriptor of the server comminication endpoint.
 * @param playerAddr The socket address structure for the remote player comminication endpoint.
 * @param board The array representing the current state of the game board.
 * @param player The value indicating which player's turn it is.
 * @return The valid choice received from either Player 1 or 2, or -1 if an invalid
 * move was recieved from Player 2.
 */
int get_player_choice(int sd, const struct sockaddr_in *playerAddr, char board[ROWS][COLUMNS], int player) {
    /* Get the player's move */
    int choice = (player == 1) ? get_p1_choice() : get_p2_choice(sd, playerAddr);
    /* Attempt to validate move; reprompt if Player 1, otherwise return error */
    if (player == 2 && choice == ERROR_CODE) return ERROR_CODE;
    while (!validate_choice(choice, board)) {
        if (player == 1) {
            choice = get_p1_choice(sd);
        } else {
            return ERROR_CODE;
        }
    }
    /* If Player 1, we need to send the move to the other player */
    if (player == 1) {
        if (send_p1_move(sd, playerAddr, choice) < 0) return ERROR_CODE;
    }
    return choice;
}

/**
 * @brief Plays a simple game of TicTacToe with a remoye player that ends when either someone wins,
 * there is a draw, or the remote player leaves the game.
 * 
 * @param sd The socket descriptor of the server comminication endpoint.
 * @param playerAddr The socket address structure for the remote player comminication endpoint.
 * @param board The array representing the current state of the game board.
 */
void tictactoe(int sd, const struct sockaddr_in *playerAddr, char board[ROWS][COLUMNS]) {
    /***************************************************************************/
    /* This is the meat of the game, you'll look here for how to change it up. */
    /***************************************************************************/
    int player = 1;     // keep track of whose turn it is
    int result, choice; // used for keeping track of choice user makes
    int row, column;
    char mark;          // either an 'X' or an 'O'

    /* Loop, first print the board, then ask the current player to make a move */
    do {
        /* Print the board on the screen */
        print_board(board);
        /* Get the player's move */
        if ((choice = get_player_choice(sd, playerAddr, board, player)) < 0) return;
        /* Depending on who the player is, use either X or O */
        mark = (player == 1) ? 'X' : 'O';
        
        /******************************************************************/
        /* A little math here. You know the squares are numbered 1-9, but */
        /* the program is using 3 rows and 3 columns. We have to do some  */
        /* simple math to convert a 1-9 to the right row/column.          */
        /******************************************************************/
        row = (int)((choice-1) / ROWS); 
        column = (choice-1) % COLUMNS;
        /* Make the move the player chose */
        board[row][column] = mark;

        /* After a move, check to see if someone won! (or if there is a draw) */
        if ((result = check_win(board)) == -1) {
            /* If not, change to other player's turn */
            player = (player == 1) ? 2 : 1;
        }
    } while (result == -1); // -1 means the game is still going 
    
    /* Print out the final board */
    print_board(board);
    
    /* Check end result of the game */
    if (result == 1) {  // means a player won!! congratulate them
        printf("==>\a Player %d wins\n", player);
    } else {
        printf("==>\a It's a draw\n");   // ran out of squares, it is a draw
    }
}
