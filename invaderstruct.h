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
    int shipModel;
} ;

struct EnemyToBield
{
    int indexAtEnemyList;
    int shipModel;
    bool hasBeenSaved;
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