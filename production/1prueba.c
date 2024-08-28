#include <unistd.h>
#include <stdio.h>

void cas(char *cas)
{
    cas[0] = 'a';
}

int main()
{
    char path[20] = "/home/gabo/";
    cas(path);
    printf("%s \n", path);
    return 0;
}