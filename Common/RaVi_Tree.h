#define OWeight 4
#define MaxNdCt 9
#define MinNdCt 5

extern vector *PrMax; extern unsigned long int *maxp;
extern vector *PrMin; extern unsigned long int *minp;

extern int MaxSplitLevel;

void      RV_MinMax(RV_PrP p, vector *Min, vector *Max);
void      RV_SplitNode(BSPplane *Node, int lastaxis, int level);
BSPplane *RV_CreateTree(PrLP p);
