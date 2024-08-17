struct Player
{
	int line, col;
	char ch;
    int hp;
    int totalHp;
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
    int movementIndex; 
    int indexAtEnemyList;
    int shipModel;
} ;

struct EnemyToBield
{
    int col, line;
    int indexAtEnemyList;
    int shipModel;
};

struct HangarNode
{
    int shipModel;
    int buildTime;
    struct HangarNode *next;
};

struct FreeEnemyBlock
{
    int index;
    int length;
    struct FreeEnemyBlock *next;
};