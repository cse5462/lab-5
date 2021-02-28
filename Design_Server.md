# TicTacToe Server (Player 1) Design
> This is the design document for the TicTacToe Server ([tictactoeP1.c](https://github.com/CSE-5462-Spring-2021/assigment4-conner-and-ben/blob/main/tictactoeP1.c)).  
> By: Conner Graham

## Table of Contents
- TicTacToe Class Protocol - [Protocol Document](https://docs.google.com/document/d/1Cl-n4nrhrzsqrl4gDFqdTwUC35rcNlwehRR58VY7k6c/edit?usp=sharing)
- [Environment Constants](#environment-constants)
- [High-Level Architecture](#high-level-architecture)
- [Low-Level Architecturet](#low-level-architecture)

## Environment Constants
```C#
VERSION = 2       // protocol version number

NUM_ARGS = 2      // number of command line arguments
TIMEOUT = TBD     // number of seconds spent waiting before a timeout
ROWS = 3          // number of rows for the TicIacToe board
COLUMNS = 3       // number of columns for the TicIacToe board

// COMMANDS
NEW_GAME = 0x00   // command to begin a new game
MOVE = 0x01       // command to issue a move
```

## High-Level Architecture
At a high level, the server application attempts to validate and extract the arguments passed
to the application. It then attempts to create and bind the server endpoint. If everything was
successful, it then starts looking for clients issuing a "New Game" request. If another player
requests a game, the server initializes the game board and begins the TicTacToe game. After
the game is over, the server then starts looking for other players issuing "New Game" requests.
If an error occurs before the connection is established, the program terminates and prints
appropriate error messages, otherwise an error message is printed and the connection is terminated.
```C
int main(int argc, char *argv[]) {
    /* check that the arg count is correct */
    if (!correct) exit(EXIT_FAILURE);
    extract_args(params...);
    create_endpoint(params...);
    /* wait for "New Game" request */
    /* infinite loop waiting for a player to make game request  */
    if (new_game) {
        /* set recv TIMEOUT */
        init_shared_state(params...);   // initialize game board
        tictactoe(params...);   // start TicTacToe game
        /* end game and look for another "New Game" request */
    } else {
        exit(EXIT_FAILURE);
    }
    return 0;
}
```

## Low-Level Architecture
Extracts the user provided arguments to their respective local variables and performs
validation on their formatting. If any errors are found, the function terminates the process.
```C
void extractArgs(params...) {
    /* extract and validate remote port number */
    if (!valid) exit(EXIT_FAILURE);
}
```
Creates the comminication endpoint with the provided IP address and port number. If any
errors are found, the function terminates the process.
```C
int create_endpoint(params...) {
    /* attempt to create socket */
    if (created) {
        /* initialize socket with params from user */
    } else {
        exit(EXIT_FAILURE);
    }
    /* attempt to bind socket to address */
    if (!bind) {
        exit(EXIT_FAILURE);
    }
    return socket-descriptor;
}
```
Recieves a datagram from another player and checks to see if it is a valid "New Game"
request.
```C
int new_game(params...) {
    /* receive datagram from remote player */
    if (!error) {
        /* check for valid "New Game" request */
        if (valid) {
            return TRUE;
        } else {
            return FALSE;
        }
    } else {
        return FALSE;
    }
    return socket-descriptor;
}
```
Plays a simple game of TicTacToe with a remoye player that ends when either someone wins,
there is a draw, or the remote player leaves the game. NOTE: This function was created
by Dr. Ogle (not myself), but I made significant changes to it's structure so I have
included it in my design.
```C
void tictactoe(params...) {
    /* initialize whose turn it is */
    while (game not over) {
        print_board(params...);
        get_player_choice(params...);   // get move from Player 1 or 2
        if (ERROR_CODE) return;
        /* get correct mark for player move */
        /* determine where to move and update game board */
        check_win(params...);
        if (no winner) {
            /* change to other player's turn */
        }
    }
    print_board(params...); // print final state of game
    /* determine who won, if anyone */
}
```
- Determines whether or not the given move is valid based on the current state of the game.
    ```C
    int validate_choice(params...) {
        if (move not numer [1-9]) return FALSE;
        if (move has already been made) return FALSE;
        return TRUE;
    }
    ```
- Gets Player 2's next move.
    ```C
    int get_p2_choice(params...) {
        /* throw away datagrams not from player who made the game request */
        /* receive datagram from current player */
        if (error) {
            if (error is timeout) {
                return ERROR_CODE;
            } else {
                return ERROR_CODE;
            }
        }
        /* check that the datagram received from the other player was valid */
        if (!valid) return ERROR_CODE;
        return (player2 move);
    }
    ```
- Sends Player 1's move to the remote player. Return the move sent, or an error code if there was
  an issue sending the move.
    ```C
    int send_p1_move(params...) {
        /* pack move info into buffer */
        /* send buffer in datagram to the remote player */
        if (error) return ERROR_CODE;
        return move;
    }
    ```
- Returns the validated player intput received from either Player 1 or 2. If the input from the host
  player in invalid, it reprompts until a valid move is made. If the input from the remote player is
  invalid, an error code is returned instead.
    ```C
    int get_player_choice(params...) {
        /* get choice from Player 1 or 2 */
        while (move invalid) {
            /* reprompt for valid move */
            if (Player 2) return ERROR_CODE;
        }
        if (Player 1) {
           send_p1_move(params...)
        }
    }
    ```
