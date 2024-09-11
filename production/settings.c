#include <ncurses.h>

int COLUMNS, ROWS;

void ChooseDifficulty(int x, int y, int difficulty, bool color)
{
    mvaddstr(LINES/2 + x, COLUMNS/2 + y, "           ");
    
    if(color)
        attron(COLOR_PAIR(1));

    switch (difficulty)
    {
        case 0:
            mvaddstr(LINES/2+ x, COLUMNS/2 - 5  + y, " Low ");
            break;
        case 1:
            mvaddstr(LINES/2 + x, COLUMNS/2 - 5 + y, " Medium ");
            break;
        case 2:
            mvaddstr(LINES/2 + x, COLUMNS/2 -5 + y, " High ");
            break;
    default:
        break;
    }

    if(color)
        attroff(COLOR_PAIR(1));
}

void ShowDifficulty(int enemySpeed, int contructionSpeed, int difficulty, int opt)
{
    ChooseDifficulty(0, 17, enemySpeed, opt == 0);
    ChooseDifficulty(1, 22,contructionSpeed, opt == 1);
    ChooseDifficulty(2, 12, difficulty, opt == 2);
    refresh();
}

void ShowSettings()
{
    wclear(stdscr);

    mvaddstr(LINES/2 - 2, COLUMNS/2 - 15, "Settings Options (Press Enter to Save)");
    mvaddstr(LINES/2, COLUMNS/2 - 10, "1 - Enemies's speed:");
    mvaddstr(LINES/2 + 1, COLUMNS/2 - 10, "2 - Construction's speed:");
    mvaddstr(LINES/2 + 2, COLUMNS/2 - 10, "3 - Difficulty:");
    
    int input = 0;
    int opt = 0;

    int enemySpeed = 0, contructionSpeed = 1, difficulty = 0;

    ShowDifficulty(enemySpeed, contructionSpeed, difficulty, opt);

    while( (input = getch()) != 10)
    {

        if(input == 65) // Up
            opt += (opt >= 0) ? -1 : 0;
        else if(input == 66) // Down
            opt += (opt < 3) ? 1: 0;
        else if(input == 67 || input == 68) //Left and Right
        {
            int mov = (input == 68) ? -1 : 1; //Left

            switch (opt)
            {
                case 0:
                    enemySpeed += ( (enemySpeed == 0 && mov == -1 )|| (enemySpeed == 2 && mov == 1)) ? 0 : mov;
                    break;
                case 1:
                    contructionSpeed += ( (contructionSpeed == 0 && mov == -1 )|| (contructionSpeed == 2 && mov == 1)) ? 0 : mov;
                    break;
                case 2:
                    difficulty += ( (difficulty == 0 && mov == -1 )|| (difficulty == 2 && mov == 1)) ? 0 : mov;
                    break;
            }

        }

        ShowDifficulty(enemySpeed, contructionSpeed, difficulty, opt);
    } 
}


int main() {
    initscr(); // Inicializa ncurses
    noecho(); // Desactiva la impresión de teclas
    cbreak(); // Desactiva el buffering de línea

    getmaxyx(stdscr, ROWS, COLUMNS);

    start_color();
    init_pair(1, COLOR_BLACK, COLOR_WHITE);
    curs_set(0);

    ShowSettings();

    endwin(); // Finaliza ncurses
    return 0;
}