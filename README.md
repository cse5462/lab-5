# TicTacToe Program – Datagram Sockets
> This is the README file for [Lab_4](https://osu.instructure.com/courses/97443/files/27903212/download?download_frd=1)

**NAME:** Conner Graham, Ben Nagel  
**DATE:** 02/25/2021

## Table of Contents:
- [Included Files](#included-files)
- [TicTacToe Player 1](#tictactoe-player-1)
  - [Description](#description-p1)
  - [Usage](#usage-p1)
  - [Assumptions](#assumptions-p1)
- [TicTacToe Player 2](#tictactoe-player-2)
  - [Description](#description-p2)
  - [Usage](#usage-p2)
  - [Assumptions](#assumptions-p2)

## Included Files
- [makefile](https://github.com/CSE-5462-Spring-2021/assigment4-conner-and-ben/blob/main/makefile)
- Player 1 (server) Design Document - [Design_P1.md](https://github.com/CSE-5462-Spring-2021/assigment4-conner-and-ben/blob/main/Design_P1.md)
- TicTacToe Player 1 Source Code - [tictactoeP1.c](https://github.com/CSE-5462-Spring-2021/assigment4-conner-and-ben/blob/main/tictactoeP1.c)
- Player 2 (client) Design Document - [Design_P2.md](https://github.com/CSE-5462-Spring-2021/assigment4-conner-and-ben/blob/main/Design_P2.md)
- TicTacToe Player 2 Source Code - [tictactoeP2.c](https://github.com/CSE-5462-Spring-2021/assigment4-conner-and-ben/blob/main/tictactoeP2.c)
## TicTacToe Player 1
> By: Conner Graham

### DESCRIPTION <a name="description-p1"></a>
This lab contains a program called "tictactoeP1" which sets up
the first player (server) of a simple net-enabled TicTacToe game.
This server sets up a communication endpoint for other players to
communicate with, sequentially accepts any "New Game" requests from
other players, initializes a game of TicTacToe when a player
requests one, and then starts playing the game sending moves to the
other player and waiting for the other player's moves in response
until a winner is found or the game is a draw. The specific tasks
the server performs are as follows:
- Create and bind server socket from user provided port
- Print server info and listen for "New Game" requests
- Accept UDP DGRAM message from waiting client
- Initialize new game of TicTacToe
- Play game of TicTacToe with client who requested the game
- Look for "New Game" request from another client once game ends

If the number of arguments is incorrect or the remote port is
invalid, the program prints appropriate messages and shows how to
correctly invoke the program. 

### USAGE <a name="usage-p1"></a>
Start the TicTacToe P1 Server with the command...
```sh
$ tictactoeP1 <local-port>
```

If any of the argument strings contain whitespace, those
arguments will need to be enclosed in quotes.

### ASSUMPTIONS <a name="assumptions-p1"></a>
- It is assumed that the Player 1 (server) will terminate the server
  when they are done playing, in which case clients will need to
  find another serevr to connect to if they sich to play.
- It is assumed that the client will never enter the int -1 (char '/')
  for a move as it is being used as an error code.
- It is assumed that the player making the "New Game" request will
  always send their messages from the same IP address and port number
  used to mkae the request.

## TicTacToe Player 2
> By: Ben Nagel

### DESCRIPTION <a name="description-p2"></a>
This lab contains a program called "tictactoeP2" which creates and sets up a datagram transfer protocal client. This client sends datagrams to the specified server( IP address and port), reads in a datagram from the server and sends a datagram back. This process continues until a winner or a tie has been reached.

The specific tasks the client performs are as
follows:
- Create server socket from user provided IP/port
- Perform datagram transfer over the connection
- Terminate the connection to the server

### USAGE <a name="usage-p2"></a>
Start the TicTacToe P2 Client with the command...
```sh
$ tictactoeP2 <remote-IP> <local-port>
```

If any of the argument strings contain whitespace, those
arguments will need to be enclosed in quotes.

### ASSUMPTIONS <a name="assumptions-p2"></a>
- Client send and recieves a 3 byte datagram(excluding the inital datagram which is 2 bytes)
- A datagram is sent and recevied 
- The spaces on the tictactoe board are 1-9
- Player 1 is the "server": they are the one who calls bind()   
- Player 1 goes first
- On any errors, close the connection 
- It is assumed that the IP addresses 0.0.0.0 and 255.255.255.255 are invalid remote server addresses to connect to as they are reserved values.
