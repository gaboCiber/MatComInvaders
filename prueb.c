#include <time.h>
#include <stdio.h>
int getRamdomNumberInterval(int min, int max)
{
    return min + rand() % (max + 1 - min);
}

int main()
{
    for (size_t i = 0; i < 10; i++)
    {
        printf("%d\n", getRamdomNumberInterval(0,2));
    }
    
}