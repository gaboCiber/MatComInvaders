#include <stdio.h>
#include <time.h>

int main() {
    clock_t start_time = clock();
    clock_t end_time;
    double elapsed_time;

    while (1) {
        end_time = clock();
        elapsed_time = ((double) (end_time - start_time)) / CLOCKS_PER_SEC;
        double time = end_time - start_time;
        if (elapsed_time >= 1.0) {
            // Código que quieres ejecutar cada segundo
            printf("Ha pasado 1 segundo\n");

            // Reiniciar el tiempo de inicio
            start_time = clock();
        }
    }

    return 0;
}