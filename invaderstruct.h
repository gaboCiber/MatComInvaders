struct Player
{
	int line, col;
	char ch;
} ;

struct Bullet
{
    int line, col;
	char ch;
};


struct Enemy
{
    int line, col;
	char ch; 
    int* upDown;
    int* leftRight;
    int upDownCount;
    int leftRightCount; 
    int indexAtEnemyList;
    int constructionTime;
} ;