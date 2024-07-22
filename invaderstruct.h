struct Player
{
	int line, col;
	char ch;
} ;

struct Enemy
{
    int line, col;
	char ch; 
    int* upDown;
    int* leftRight;
    int upDownCount;
    int leftRightCount; 
} ;