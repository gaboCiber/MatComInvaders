#include <ncurses.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>

typedef struct  
{
	int line, col;
	char ch;
} player;

typedef struct 
{
    int line, col;
	char ch; 
    int* upDown;
    int* leftRight;
    int upDownCount;
    int leftRightCount; 
} enemy;


player play;

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

int cases[] = {-1, 0 ,1};

void GetRandomPos(enemy *en)
{
    int random = rand();
    en->col += *(en->leftRight + (rand() % en->leftRightCount));
    en->line += *(en->upDown + (rand() % en->upDownCount));
}

void moveEnemy(void *ene)
{
    enemy* en = (enemy *) ene;
    bool insideScreen = 1;

    while (insideScreen)
    {
        move(en->line, en->col);
        addch(' ');
        
        GetRandomPos(&en);

        move(en->line, en->col);
        addch(en->ch);

        insideScreen = en->line > 0 && en->line < LINES && en->col > 0 && en->col < COLS;
        refresh();
        sleep(1);
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

    // Enemy
    pthread_t enemyThread;
    enemy en;
    en.line = 3;
    en.col = COLS/2;
    en.ch = '#';
    int ar1[] = {-1, 0 , 1};
    en.leftRight = &ar1;
    en.leftRightCount = 3;
    int ar2[] = {0 , 1};
    en.upDown = &ar2;
    en.upDownCount = 2;

    move(en.line, en.col);
    addch(en.ch);

    refresh();

    bool insideScreen = 1;
    while (insideScreen)
    {
        move(en.line, en.col);
        addch(' ');
        
        GetRandomPos(&en);
        GetRandomPos(&en);
        
        move(en.line, en.col);
        addch(en.ch);

        insideScreen = en.line > 0 && en.line < LINES && en.col > 0 && en.col < COLS;
        sleep(1);
        refresh();
    }
    

    /*int enemyMov = pthread_create(&enemyThread, NULL, moveEnemy, (void*) &en);
    if(enemyMov != 0)
    {
        perror("Error al crear el hilo");
        return 1;
    }*/

    // Player
    play.line = LINES-1;
    play.col = COLS/2;
    play.ch = '^';
    
    move(play.line,play.col);
    addch(play.ch);

    pthread_t playerThread;    
    int playerMov;
    int ch;
    while ((ch = getch()) != KEY_F(4) && ch != ERR)
    {   
        playerMov = pthread_create(&playerThread, NULL, movePlayer, (void *) ch);
    }    
    

    // Detiene ncurses y restaura el estado del terminal
    endwin();

    return 0;
}