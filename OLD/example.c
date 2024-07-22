#include <stdlib.h>
#include <stdio.h>
#include <time.h>

struct enemy
{
    int line, col;
	char ch; 
    int* upDown;
    int* leftRight;
    int upDownCount;
    int leftRightCount; 
};

int GetRandomNumber()
{
    int cas[] = {-1, 0 ,1};
    int random = rand();
    printf("%d ", random);
    return cas[random % 3];
}

int main()
{
    struct enemy en;
    int arr[] = {0 , 1, -1};
    en.leftRight = &arr;
    
    for(int i = 0; i < 3; i++)
    {
        printf("%d \n", *(en.leftRight + i));
    }
    
}