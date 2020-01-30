#include "rps_server.h"
#include "name_server.h"
#include "syscall.h"
#include "logging.h"

#define MAX_GAME_COUNT 16
#define REQUEST_LENGTH 2

int queued_client;

struct game
{
    int running;

    int p1;
    int p2;

    char p1_move;
    char p2_move;
};

typedef struct game Game;

Game games[MAX_GAME_COUNT];

void Signup()
{
    int rps_id = WhoIs("rps_server");

    char message[REQUEST_LENGTH];
    int msglen = REQUEST_LENGTH;
    char reply[1];

    message[0] = 's';

    Send(rps_id, message, msglen, reply, REQUEST_LENGTH);
}

int Play(char move)
{
    assert(move == ROCK || move == PAPER || move == SCISSORS);

    int rps_id = WhoIs("rps_server");

    char message[REQUEST_LENGTH];
    int msglen = REQUEST_LENGTH;
    char reply[1];

    message[0] = 'p';
    message[1] = move;
    
    Send(rps_id, message, msglen, reply, REQUEST_LENGTH);

    return reply[0];
}

void Quit()
{
    int rps_id = WhoIs("rps_server");

    char message[REQUEST_LENGTH];
    int msglen = REQUEST_LENGTH;
    char reply[1];

    message[0] = 'q';
    
    Send(rps_id, message, msglen, reply, REQUEST_LENGTH);
}

void _signup(int id)
{
    char response[1]; // Empty response

    // See if another client is queued up
    if(queued_client == -1)
    {
        // Since no reply is sent, sender is blocked until another player arrives!
        queued_client = id;
        return;
    } 
    
    // Another client is queued up, play with them :)
    // Find a free Game struct
    Game* free_game = 0;
    for(int i = 0; i != MAX_GAME_COUNT; i++) 
    {
        if(games[i].running == 0)
        {
            free_game = &games[i];
            break;
        }
    }

    if(free_game == 0) FATAL("Maxmimum number of games reached!");
    
    // Allocate the game
    free_game->running = 1;
    free_game->p1 = queued_client;
    free_game->p2 = id;
    free_game->p1_move = 0;
    free_game->p2_move = 0;
    queued_client = -1;

    // Respond to clients
    Reply(free_game->p1, response, 1);
    Reply(free_game->p2, response, 1);
}

void _play(int id, char move)
{
    char response[1];

    if(queued_client == id) return; // Not in a game yet!  Remain blocked.

    // Find game that id is in
    Game* game = 0;
    for(int i = 0; i != MAX_GAME_COUNT; i++) 
    {
        if(games[i].running && (games[i].p1 == id || games[i].p2 == id))
        {
            game = &games[i];
            break;
        }
    }

    // Player hasn't signed up first
    if(game == 0)
    { 
        response[0] = NOT_SIGNED_UP;
        Reply(id, response, 1); 
        return; 
    }

    if(id == game->p1) game->p1_move = move;
    else if(id == game->p2) game->p2_move = move;

    // If only one player left in game and both plays have not been made yet, notify player left
    if((id == game->p1 && game->p2 == -1))
    {
        response[0] = OPPONENT_QUIT;
        Reply(game->p1, response, 1);
    }
    else if(id == game->p2 && game->p1 == -1)
    {
        response[0] = OPPONENT_QUIT;
        Reply(game->p2, response, 1);
    }

    // If both moves made, play!
    if(game->p1_move && game->p2_move)
    {
        int winner = 0;
        if(game->p1_move == ROCK && game->p2_move == PAPER) winner = 2;
        else if(game->p1_move == PAPER    && game->p2_move == ROCK) winner = 1;
        else if(game->p1_move == ROCK     && game->p2_move == SCISSORS) winner = 1;
        else if(game->p1_move == SCISSORS && game->p2_move == ROCK) winner = 2;
        else if(game->p1_move == PAPER    && game->p2_move == SCISSORS) winner = 2;
        else if(game->p1_move == SCISSORS && game->p2_move == PAPER) winner = 1;

        // Respond to clients
        if(winner == 1)
        {
            response[0] = WIN;
            Reply(game->p1, response, 1);
            response[0] = LOSE;
            Reply(game->p2, response, 1);
        }
        else if(winner == 2)
        {
            response[0] = LOSE;
            Reply(game->p1, response, 1);
            response[0] = WIN;
            Reply(game->p2, response, 1);
        }
        else // TIE
        {
            response[0] = TIE;
            Reply(game->p1, response, 1);
            Reply(game->p2, response, 1);
        }

        // Clear moves
        game->p1_move = 0;
        game->p2_move = 0;

        // Prompt user to continue
        bwgetc(COM2);
    }
    else if( !(game->p1_move || game->p2_move) ) assert(0); // Should never be reached
}

void _quit(int id)
{
    char response[1];

    // Find game that id is in
    Game* game = 0;
    for(int i = 0; i != MAX_GAME_COUNT; i++) 
    {
        if(games[i].running && (games[i].p1 == id || games[i].p2 == id))
        {
            game = &games[i];
            break;
        }
    }

    // Currently no response if player hasn't signed up first
    if(game == 0) return;

    if(game->p1 == id) game->p1 = -1;
    else if(game->p2 == id) game->p2 = -1;

    // If game is no longer running, deallocate it.
    if(game->p1 == -1 && game->p2 == -1) 
    {
        game->running = 0;
    }

    // Unblock the quitting task
    Reply(id, response, 1); 

    // If player left in game after quitting and only one move has been done, let them know you quit!
    if((game->p1_move == 0 && game->p2_move != 0) ||
       (game->p1_move != 0 && game->p2_move == 0))
    {
        if(game->p1 != -1)
        {
            response[0] = OPPONENT_QUIT;
            Reply(game->p1, response, 1);
        }
        if(game->p2 != -1)
        {
            response[0] = OPPONENT_QUIT;
            Reply(game->p2, response, 1);
        }
    }
}

void rps_server()
{
    // Register on the name server
    RegisterAs("rps_server");

    queued_client = -1;

    for(int i = 0; i != MAX_GAME_COUNT; i++)
    {
        games[i].running = 0;
        games[i].p1 = -1;
        games[i].p2 = -1;
    } 

    int sender;
    char msg[REQUEST_LENGTH];

    for(;;)
    {
        // Receive rps server request
        Receive(&sender, msg, 2);

        // Execute command and send responses
        if(msg[0] == 's') // Signup
        {
            _signup(sender);
        }
        else if(msg[0] == 'p') // Play
        {
            _play(sender, msg[1]);
        }
        else if(msg[0] == 'q') // Quit
        {
            _quit(sender);
        }

    }
}