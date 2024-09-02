#include <ncurses.h>

int COLUMNS, ROWS;

void drawMatCom(int x, int y) {
    mvprintw(y, x, "      __  __    _    _____  ____  _____   __  __");
    mvprintw(y + 1, x, "     |  \\/  |  / \\  |_   _|/ ___ /  _  \\ |  \\/  |");
    mvprintw(y + 2, x, "     | |\\/| | / _ \\   | |  | |   | | | | | |\\/| |");
    mvprintw(y + 3, x, "     | |  | |/ ___ \\  | |  | |__ | |_| | | |  | |");
    mvprintw(y + 4, x, "     |_|  |_/_/   \\_\\ |_|  \\____ \\_____/ |_|  |_|");
    mvprintw(y + 5, x, "                                                            ");
    mvprintw(y + 6, x, " ___ _   _ __       ___     _____   ___   _____   _   _");
    mvprintw(y + 7, x, "|_ _| \\ | |\\ \\     / / \\   / ____| |_ _| /  _  \\ | \\ | |");
    mvprintw(y + 8, x, " | ||  \\| | \\ \\   / / _ \\  |(___    | |  | | | | |  \\| |");
    mvprintw(y + 9, x, " | || |\\  |  \\ \\_/ / ___ \\  ____)|  | |  | |_| | | |\\  | ");
   mvprintw(y + 10, x, "|___|_| \\_|   \\___/_/   \\_\\|_____/ |___| \\_____/ |_| \\_| ");
}

void drawSpaceship(int x, int y) {
    mvprintw(y, x, "     /\\");
    mvprintw(y + 1, x, "    /  \\");
    mvprintw(y + 2, x, "   /____\\");
    mvprintw(y + 3, x, "  |      |");
    mvprintw(y + 4, x, "  |  __  |");
    mvprintw(y + 5, x, "  | |  | |");
    mvprintw(y + 6, x, "  | |__| |");
    mvprintw(y + 7, x, "  |______|");
    mvprintw(y + 8, x, " /|      |\\");
    mvprintw(y + 9, x, "/_|______|_\\");
}

void drawAlien(int x, int y) {
    mvprintw(y, x, "   _____ ");
    mvprintw(y + 1, x, "  /     \\");
    mvprintw(y + 2, x, " | () () |");
    mvprintw(y + 3, x, "  \\  ^  /");
    mvprintw(y + 4, x, "   |||||");
    mvprintw(y + 5, x, "   |||||");
}

int main() {
    initscr(); // Inicializa ncurses
    noecho(); // Desactiva la impresión de teclas
    cbreak(); // Desactiva el buffering de línea

    getmaxyx(stdscr, ROWS, COLUMNS);

    drawSpaceship(5, ROWS / 2 - 9);
    drawMatCom(COLUMNS / 2 - 25, ROWS / 2 - 10);
    drawAlien(COLUMNS - 20, ROWS / 2 - 8);

    refresh(); // Refresca la pantalla para mostrar los cambios
    getch(); // Espera a que el usuario presione una tecla

    endwin(); // Finaliza ncurses
    return 0;
}