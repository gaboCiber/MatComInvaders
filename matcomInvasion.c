#include <ncurses.h>
#include <pthread.h>

struct player {
	int line, col;
	char ch;
};

struct player play;

void movePlayer(void *ch)
{
    if( (ch == KEY_RIGHT || ch == KEY_LEFT) )
    {
        
        move(play.line,play.col);
        addch(' ');

        play.col += (ch == KEY_RIGHT) ? 1 : -1;

        if(play.col <= 0)
            play.col = 1;
        else if (play.col >= COLS)
            play.col = COLS - 1;

        move(play.line,play.col);
        addch(play.ch);
    }
}

int main() {
    initscr(); // Inicializa ncurses
    keypad(stdscr, TRUE);
    curs_set(0);

    move(0,(COLS/2)-9);
    addstr("--MatCOM INVADERS--");
    move (0,1);
    addstr("SCORE: ");

    refresh();

    play.line = LINES-1;
    play.col = COLS/2;
    play.ch = '^';
    
    move(play.line,play.col);
    addch(play.ch);
    
    pthread_t playerThreat;
    int playerMov;
    int ch;
    while ((ch = getch()) != KEY_F(4) && ch != ERR)
    {   
        playerMov = pthread_create(&playerThreat, NULL, movePlayer, (void *) ch);
    }    

    // Detiene ncurses y restaura el estado del terminal
    endwin();

    return 0;
}