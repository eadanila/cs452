#include "rps_client.h"
#include "rps_server.h"
#include "logging.h"

const char* print_match_result(int r)
{
    switch(r)
    {
        case WIN:
        return "WON";
        break;
        case LOSE:
        return "LOST";
        break;
        case TIE:
        return "TIED";
        break;
        case NOT_SIGNED_UP:
        return "WASNT SIGNED UP";
        break;
        case OPPONENT_QUIT:
        return "OPPONENT QUIT";
        break;
    }

    return "<UNSUPPORTED MATCH RESULT>";
}

void rps_rock_client()
{
    Signup(); print("rps_rock_client signed up.\n\r");
    int r = Play(ROCK); print("rps_rock_client played rock and %s.\n\r", print_match_result(r));
    Quit(); print("rps_rock_client quit.\n\r");
}

void rps_paper_client()
{
    Signup(); print("rps_paper_client signed up.\n\r");
    int r = Play(PAPER); print("rps_paper_client played paper and %s.\n\r", print_match_result(r));
    Quit(); print("rps_paper_client quit.\n\r");
}

void rps_scissors_client()
{
    Signup(); print("rps_scissors_client signed up.\n\r");
    int r = Play(SCISSORS); print("rps_scissors_client played scissors and %s.\n\r", print_match_result(r));
    Quit(); print("rps_scissors_client quit.\n\r");
}

void rps_rock_lover_client()
{
    Signup(); print("rps_rock_lover_client signed up.\n\r");
    int r = Play(ROCK); print("rps_rock_lover_client played rock and %s.\n\r", print_match_result(r));
        r = Play(ROCK); print("rps_rock_lover_client played rock and %s.\n\r", print_match_result(r));
        r = Play(ROCK); print("rps_rock_lover_client played rock and %s.\n\r", print_match_result(r));
    Quit(); print("rps_rock_lover_client quit.\n\r");
}

void rps_paper_lover_client()
{
    Signup(); print("rps_paper_lover_client signed up.\n\r"); // TODO Handle trying to play twice in a row???
    int r = Play(PAPER); print("rps_paper_lover_client played paper and %s.\n\r", print_match_result(r));
        r = Play(PAPER); print("rps_paper_lover_client played paper and %s.\n\r", print_match_result(r));
        r = Play(PAPER); print("rps_paper_lover_client played paper and %s.\n\r", print_match_result(r));
    Quit(); print("rps_paper_lover_client quit.\n\r");
}

void rps_quitter_client()
{
    Signup(); print("rps_quitter_client signed up.\n\r"); // TODO Handle trying to play twice in a row???
    Quit(); print("rps_quitter_client quit.\n\r");
}

void rps_abusive_client()
{
    Quit(); print("rps_abusive_client quit before signing up.\n\r");
    int r = Play(PAPER); print("rps_abusive_client played paper and %s\n\r", print_match_result(r));

    Signup(); print("rps_abusive_client signed up.\n\r");
    Quit(); print("rps_abusive_client quit.\n\r");
    r = Play(PAPER); print("rps_abusive_client played paper and %s\n\r", print_match_result(r));
}

void rps_player_1()
{
    Signup(); print("rps_player_1 signed up.\n\r");
    int r = Play(PAPER); print("rps_player_1 played paper and %s.\n\r", print_match_result(r));
        r = Play(PAPER); print("rps_player_1 played paper and %s.\n\r", print_match_result(r));
        r = Play(PAPER); print("rps_player_1 played paper and %s.\n\r", print_match_result(r));
        r = Play(PAPER); print("rps_player_1 played paper and %s.\n\r", print_match_result(r));
    Quit(); print("rps_player_1 quit.\n\r");
}

void rps_player_2()
{
    Signup(); print("rps_player_2 signed up.\n\r");
    int r = Play(ROCK); print("rps_player_2 played rock and %s.\n\r", print_match_result(r));
        r = Play(ROCK); print("rps_player_2 played rock and %s.\n\r", print_match_result(r)); bwgetc(COM2);
        r = Play(ROCK); print("rps_player_2 played rock and %s.\n\r", print_match_result(r)); bwgetc(COM2);
        r = Play(ROCK); print("rps_player_2 played rock and %s.\n\r", print_match_result(r)); bwgetc(COM2);
    Quit(); print("rps_player_2 quit.\n\r");
}

void rps_player_3()
{
    Signup(); print("rps_player_3 signed up.\n\r");
    int r = Play(ROCK); print("rps_player_3 played rock and %s.\n\r", print_match_result(r));
        r = Play(ROCK); print("rps_player_3 played rock and %s.\n\r", print_match_result(r));
        r = Play(ROCK); print("rps_player_3 played rock and %s.\n\r", print_match_result(r));
    Quit(); print("rps_player_3 quit.\n\r");
}

void rps_player_4()
{
    Signup(); print("rps_player_4 signed up.\n\r");
    int r = Play(ROCK); print("rps_player_4 played rock and %s.\n\r", print_match_result(r));
        r = Play(ROCK); print("rps_player_4 played rock and %s.\n\r", print_match_result(r));
        r = Play(ROCK); print("rps_player_4 played rock and %s.\n\r", print_match_result(r));
    Quit(); print("rps_player_4 quit.\n\r\n\r");
}

void rps_try_everything_1()
{
    Signup(); print("player_1 signed up.\n\r");
    int r = Play(ROCK); print("player_1 played rock and %s.\n\r", print_match_result(r));
        r = Play(ROCK); print("player_1 played rock and %s.\n\r", print_match_result(r));
        r = Play(ROCK); print("player_1 played rock and %s.\n\r", print_match_result(r));
        r = Play(PAPER); print("player_1 played paper and %s.\n\r", print_match_result(r));
        r = Play(PAPER); print("player_1 played paper and %s.\n\r", print_match_result(r));
        r = Play(PAPER); print("player_1 played paper and %s.\n\r", print_match_result(r));
        r = Play(SCISSORS); print("player_1 played scissors and %s.\n\r", print_match_result(r));
        r = Play(SCISSORS); print("player_1 played scissors and %s.\n\r", print_match_result(r));
        r = Play(SCISSORS); print("player_1 played scissors and %s.\n\r", print_match_result(r));
    Quit(); print("player_1 quit.\n\r");
}
void rps_try_everything_2()
{
    Signup(); print("player_2 signed up.\n\r");
    int r = Play(ROCK); print("player_2 played rock and %s.\n\r", print_match_result(r));   
        r = Play(PAPER); print("player_2 played paper and %s.\n\r", print_match_result(r)); 
        r = Play(SCISSORS); print("player_2 played scissors and %s.\n\r", print_match_result(r)); 
        r = Play(ROCK); print("player_2 played rock and %s.\n\r", print_match_result(r)); 
        r = Play(PAPER); print("player_2 played paper and %s.\n\r", print_match_result(r)); 
        r = Play(SCISSORS); print("player_2 played scissors and %s.\n\r", print_match_result(r)); 
        r = Play(ROCK); print("player_2 played rock and %s.\n\r", print_match_result(r)); 
        r = Play(PAPER); print("player_2 played paper and %s.\n\r", print_match_result(r)); 
        r = Play(SCISSORS); print("player_2 played scissors and %s.\n\r", print_match_result(r));
    Quit(); print("player_2 quit.\n\r");
}