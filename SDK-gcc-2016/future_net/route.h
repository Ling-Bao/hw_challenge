#ifndef __ROUTE_H__
#define __ROUTE_H__

#define MYINF 9999
#define MAX_VERTEX 600
#define MAX_VISITED 7

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <math.h>
#include <time.h>

/****邻接表结构体***本来想开始用十字链表的可是由于有多条重复路所以用要单独用邻接表与逆邻接表*/

typedef struct ArcNode
{
int headVex,tailVex;//弧头尾结点
struct ArcNode *nextLink;//弧头以及弧尾相同的链域
int cost;//弧的权值
float Ncost;
int arcId;//弧的编号
}ArcNode;//弧结点
typedef struct VexNode
{
//int vexId;//顶点编号,以数组位置表示
bool key;//关键点用true表示，否则为false;
int outDegree;//出度
int inDegree;//入度
ArcNode *firstLink;//第一条出弧与入弧链域
}VexNode;//顶点结点
typedef struct OrGraph
{
VexNode clist[MAX_VERTEX];//该有向图的顶点不超过600个
int vexNum,arcNum;//有向图当前的顶点数与弧度数
}OrGraph;


/*********************堆节点结构体*****************/


//最小堆节点
typedef struct MinHeapNode
{
	int v;//下标，即节点id
	int dist;//到达此结点的代价
}MinHeapNode;
//最小堆
typedef struct MinHeap
{
	int size;//堆中节点数
	int capacity;//堆中容量
	int *pos;//pos[i]表示顶点i在堆中所在的下标，方便堆的调整
	struct MinHeapNode **array;
}MinHeap;
// 创建一个最小堆节点


/***************测试****结合dij求解问题,要对dij结果排序***/


typedef struct ArticalNode
{
	int v;//下标，即节点id
	int nextV;
	int dist;//到达此结点的代价	
}ArticalNode;


/****************节点度排序*******************/
typedef struct NodeDegree
{
	int v;
	int inDegree;
	int outDegree;
}NodeDegree;


void search_route(char *graph[5000], int edge_num, char *condition);


/******************************************邻接表***************************************************++必备++*/


/*根据topo产生邻接表*/
void createOG(OrGraph *G, char *topo[5000],int edge_num, bool isReverse);
/***解析demand字符串为start_end以及必经点个点和相应的点***/
void explain_demand(int post[3],int unavoid[],char *demand);


/********************************建立最小堆，堆调整，出堆，判断堆是否为空等**************************++必备++*/


struct MinHeapNode* newMinHeapNode(int v, int dist);
// A utility function to create a Min Heap
struct MinHeap* createMinHeap(int capacity);
// 交换两个最小堆的节点
void swapMinHeapNode(struct MinHeapNode** a, struct MinHeapNode** b);
//在位置 idx 调整堆，初始堆要调整为小顶堆
void minHeapify(struct MinHeap* minHeap, int idx);
// 推是否为空
int isEmpty(struct MinHeap* minHeap);
// 弹出堆顶的节点(即最小的节点)
struct MinHeapNode* extractMin(struct MinHeap* minHeap);
// 当节点v的距离更新后(变小了)调整堆
void decreaseKey(struct MinHeap* minHeap, int v, int dist);
// 判断节点v是否在堆中
bool isInMinHeap(struct MinHeap *minHeap, int v);



/********************************************求最短路径主要算法1***************************************++必备++*/
/**
*1、将没有加入到路中的点入堆
*2、第一个点设为起始点，因为是最小的
*3、每次选一个离原点最近的点出来更新其它点到原点的距离，直到遇到第一个必经节点，退出
*4、然后以这个必经节点为开始迭代更新
*/

int dijkstraClosed(OrGraph *G,int src,float dist[],int path[],int arc[],bool flag[], int *keyNum,int endv,bool isEnd);
bool minPath(OrGraph *G,int start_end[],float dist[],int arc[],bool flag[],int path[]);
//int dijkstraClosedMidst(OrGraph *G,int src,int dist[],int path[],int arc[],bool flag[], int *keyNum,int exNode,bool isEnd);
bool minPathMidst(OrGraph *G,int start_end[],int mid,float dist[],int arc[],bool flag[],int path[],int unvoid[]);
//转换起始点与初始点，在用逆邻接表时要把原来的终点转换为起点
void changeStart_end(int se[]);

/**************************************************回溯路径查找****************************************--必备--*/

//int locate(ArticalNode m[],int scope,int findNum);
//int findArc(OrGraph *G,int pnode,int nnode);
int findArcCost(OrGraph *G,int pnode,int arcId);
float calCost(OrGraph *G,int end,int path[],int arc[]);
/********************************************求最短路径主要算法2--基于1********************************++必备++*/


//循环减支，减去必经出度与入度为1的后继和前驱的入度和出度.注意标记，避免循环减支
//先不考虑标记看看,才得40分，减支是有问题的，标记看看
//减支目前存在问题，因为逆邻接表和正邻接表相互影响
void cut(OrGraph *G1,OrGraph *G2,int unvoid[],int start_end[],bool *mark1,bool *mark2);
void cutNeighborOutD(OrGraph *G1,OrGraph *G2,int v,bool *mark,int end);
void cutNeighborInD(OrGraph *G1,OrGraph *G2,int v,bool *mark,int beg);

void removeZero(OrGraph *G1,OrGraph *G2, int start_end[],bool *mark1,bool *mark2);
void removeOutZeroNode(OrGraph *G1,OrGraph *G2,int v,bool *mark,int end);
void removeInZeroNode(OrGraph *G1,OrGraph *G2, int v,bool *mark,int beg);

void removeBENode(OrGraph *G1,OrGraph *G2,int start_end[]);//去除开始点的前驱和结束点的后继并调整度
void removeBeginNode(OrGraph *G1, OrGraph *G2,int begin);
void removeEndNode(OrGraph *G1,int end);


//改进：你说的那个方法，先找其它点再找入度大的我觉得不错，怎么绕过最短的，是个问题，标记
//绕过最短的以一个综合的取值，以堆的形式挑选


/*********************************************综合评价权值**********************************************************/
/*入度1-16的点个数分布为正态，出度1-8的点个数分布为半个正态，弧度cost为1-20的弧个数分布基本上为均匀分布
利用公式ncost=outdegree(indegree+w);indegree标准化，w服从1-20的均匀分布
先求入度的均值和方差，再将w标准化，但是对于w的标准化先用20后期再进行改动,此时均值为（20+1）/2=10.5，方差为（20-1）^2/12=30,开根号约为5.5
由于正态标准化有负值，所以要平移一下u轴
*/
void updateNCost(OrGraph *G, int iAlgorithm);
void updateNCostJoin(OrGraph *G, float join[], int iAlgorithm);

float calNodeInDeAverage(OrGraph *G);
float calNodeInDeStdDe(OrGraph *G,float ave);
float calNodeOutDeAverage(OrGraph *G);
float calNodeOutDeStdDe(OrGraph *G,float aveO);
int dijkstraClosedNcost(OrGraph *G,int src,float dist[],int path[],int arc[],bool flag[], int *keyNum,int endv,bool isEnd);
bool minPathMidstNcost(OrGraph *G,OrGraph *RG, int start_end[],int mid,float dist[],int arc[],bool flag[],int path[],int unvoid[]);
bool minPathNcost(OrGraph *G,int arr_se[],float dist[],int arc[],bool flag[],int path[]);
void solve(OrGraph *G, int start_end[], OrGraph *RG , int Rstart_end[], float dist[] ,int arc[] ,bool flag[], int path[],int path_min[],int arc_min[],int unvoid[],bool *isR);
void set_mArr(OrGraph *G);
//双关键字计数排序，先以入度（关键字二）排序，再以出度（关键字一）排序，对于计数排序由于只计数，却没有记录下数对应的结构体所以不方便使用



#endif
