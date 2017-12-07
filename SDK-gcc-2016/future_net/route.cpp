#include "route.h"
#include "lib_record.h"

static float abc = 0;
static bool kmark=false;
static float mcost=MYINF;

/*************************************************建邻接表********************************************/
/*根据topo产生邻接表*/
void createOG(OrGraph *G, char *topo[5000],int edge_num, bool isReverse){
	int val[4];
	//int j=0;
	int nv=0;
	for(int i=0;i<4;i++)
	{
		val[i]=-1;//初始化数组，用数组记录分割，val[0]为序号，val[1]为源点，val[2]为终点，val[3]为代价。
	}
	ArcNode *p,*q;
	char *tmp;
	char sep[]=",";
	bool isHaveVex[MAX_VERTEX];
	memset(isHaveVex,0,sizeof(isHaveVex));

	for(int i=0; i<edge_num; i++)
	{
		tmp=strdup(topo[i]);
		val[0]=atoi(strsep(&tmp,sep));
		val[1]=atoi(strsep(&tmp,sep));
		val[2]=atoi(strsep(&tmp,sep));
		val[3]=atoi(tmp);
		p=(ArcNode *)malloc(sizeof(ArcNode));
		p->arcId=val[0];
		if(isReverse)
		{
			p->tailVex=val[2];
			p->headVex=val[1];//逆向连接构建逆邻接表，headVex弧的头节点，val[1]表示源节点，val[2]表示终点
			
		}
		else
		{
			p->tailVex=val[1];
			p->headVex=val[2];//正向连接，构建邻接表
			
		}
		
		p->cost=val[3];
		p->nextLink=NULL;

		++G->clist[p->tailVex].outDegree;
		++G->clist[p->headVex].inDegree;
		
		if(!isHaveVex[p->headVex])
		{
			isHaveVex[p->headVex]=true;
			++nv;
		}
		if(!isHaveVex[p->tailVex])
		{
			isHaveVex[p->tailVex]=true;
			//G->clist[val[1]].firstLink=p;//不能这样因为有时，这个点计数了，但是没有链接，顶点无链接不一定无计数，所以可能导至有些点无链接
			++nv;
		}
		if(G->clist[p->tailVex].firstLink==NULL)
		{
			G->clist[p->tailVex].firstLink=p;
		}
		
		else
		{ 
			q=G->clist[p->tailVex].firstLink;
			while(q->nextLink)
			{
				q=q->nextLink;
			}
			q->nextLink=p;
		}
	}
	G->vexNum=nv;
	G->arcNum=edge_num;
}
/*******************************************************************************************************/

/*******************************************解析命令字符串********************************************/
/***解析demand字符串为start_end以及必经点个点和相应的点***/
void explain_demand(int post[3],int unavoid[],char *demand){
	char sep1[]=",";
	char sep2[]="|";
	post[0]=atoi(strsep(&demand,sep1));//start point
	post[1]=atoi(strsep(&demand,sep1));//end point
	int i=0;
	while(demand)
	{
	   unavoid[i++]=atoi(strsep(&demand,sep2));
	};
	post[2]=i;
}
/*******************************************************************************************************/

/********************************建立最小堆，堆调整，出堆，判断堆是否为空等***************************/
struct MinHeapNode* newMinHeapNode(int v, int dist) {
	struct MinHeapNode* minHeapNode = (struct MinHeapNode*) malloc(
			sizeof(struct MinHeapNode));
	minHeapNode->v = v;
	minHeapNode->dist = dist;
	return minHeapNode;
}

struct MinHeap* createMinHeap(int capacity){
	struct MinHeap* minHeap = (struct MinHeap*) malloc(sizeof(struct MinHeap));
	minHeap->pos = (int *) malloc(capacity * sizeof(int));
	minHeap->size = 0;
	minHeap->capacity = capacity;
	minHeap->array = (struct MinHeapNode**) malloc(
			capacity * sizeof(struct MinHeapNode*));
	return minHeap;
}
// 交换两个最小堆的节点
void swapMinHeapNode(struct MinHeapNode** a, struct MinHeapNode** b) {
	struct MinHeapNode* t = *a;
	*a = *b;
	*b = t;
}

void minHeapify(struct MinHeap* minHeap, int idx) {
	int smallest, left, right;
	smallest = idx;
	left = 2 * idx + 1;
	right = 2 * idx + 2;

	if (left < minHeap->size&& minHeap->array[left]->dist < minHeap->array[smallest]->dist)
		smallest = left;

	if (right < minHeap->size&& minHeap->array[right]->dist < minHeap->array[smallest]->dist)
		smallest = right;

	if (smallest != idx) {
		// 需要交换的节点
		MinHeapNode *smallestNode = minHeap->array[smallest];
		MinHeapNode *idxNode = minHeap->array[idx];

		//交换下标
		minHeap->pos[smallestNode->v] = idx;
		minHeap->pos[idxNode->v] = smallest;

		//交换节点
		swapMinHeapNode(&minHeap->array[smallest], &minHeap->array[idx]);

		minHeapify(minHeap, smallest);
	}
}
// 推是否为空
int isEmpty(struct MinHeap* minHeap) {
	return minHeap->size == 0;
}
// 弹出堆顶的节点(即最小的节点)
struct MinHeapNode* extractMin(struct MinHeap* minHeap) {
	if (isEmpty(minHeap))
		return NULL;

	struct MinHeapNode* root = minHeap->array[0];

	struct MinHeapNode* lastNode = minHeap->array[minHeap->size - 1];
	minHeap->array[0] = lastNode;

	// 更新下标
	minHeap->pos[root->v] = minHeap->size - 1;
	minHeap->pos[lastNode->v] = 0;

	// 减少堆的大小
	--minHeap->size;
	minHeapify(minHeap, 0);
	return root;
}
// 当节点v的距离更新后(变小了)调整堆
void decreaseKey(struct MinHeap* minHeap, int v, int dist) {
	//获取节点 v 在 堆中的下标
	int i = minHeap->pos[v];

	minHeap->array[i]->dist = dist;

	while (i && minHeap->array[i]->dist < minHeap->array[(i - 1) / 2]->dist) {
		minHeap->pos[minHeap->array[i]->v] = (i - 1) / 2;
		minHeap->pos[minHeap->array[(i - 1) / 2]->v] = i;
		swapMinHeapNode(&minHeap->array[i], &minHeap->array[(i - 1) / 2]);
		i = (i - 1) / 2;
	}
}
// 判断节点v是否在堆中
bool isInMinHeap(struct MinHeap *minHeap, int v) {
	//printf("kankan1:---------------k\n");
	if (minHeap->pos[v] < minHeap->size)
		return true;
	return false;
}

/*******************************************************************************************************/


/******************************************求最短路径主要算法*****************************************/

int dijkstraClosed(OrGraph *G,int src,float dist[],int path[],int arc[],bool flag[], int *keyNum,int endv,bool isEnd)
{
	//最小优先队列，数组的方式,以后换feib堆。
	int vNum=G->vexNum;
	MinHeap* minHeap=createMinHeap(vNum);
	//初始化堆包含所有的顶点
	int i,j=1;
	minHeap->array[0]=newMinHeapNode(src,dist[src]);
	
	if(!isEnd)
	{	
		for(i=0;i<vNum;++i)
		{
			
			if(!flag[i]&&i!=endv)//不加终点
			{
				dist[i]=MYINF;//dist[i]为MYINF可能
				minHeap->array[j]=newMinHeapNode(i,dist[i]);
				minHeap->pos[i]=j;
				++j;
				path[i]=-1;
			}
		}
	}
	else{
		for(i=0;i<vNum;++i)
		{
			if(!flag[i])//包含终点
			{

			dist[i]=MYINF;
			minHeap->array[j]=newMinHeapNode(i,dist[i]);
			minHeap->pos[i]=j;
			++j;
			path[i]=-1;
			}
		}
	}
	G->clist[src].key=false;//源必为flase不然就出去了
	minHeap->size = j;
	
	int v;
	while (!isEmpty(minHeap)) {
		MinHeapNode* minHeapNode = extractMin(minHeap);
		int u = minHeapNode->v;
		free(minHeapNode);
		if(G->clist[u].key)//加入堆中的是所有的flag为false的点。
		{
				
			--*keyNum;
			return u;
		}
		
		ArcNode* pCrawl = G->clist[u].firstLink;
		while (pCrawl != NULL) {

			v = pCrawl->headVex;
			//检醒所连接的点都是没有在路径上的，如果在的话则找下一个，加入堆中的节点都是还没有在路径上的
			if(flag[v])
			{
				pCrawl = pCrawl->nextLink;
				continue;
			}/**/
			
			//如果是endv的话但是不是最后一个结点，就下一个
			if(v==endv&&!isEnd)
			{
				pCrawl = pCrawl->nextLink;
				continue;
			}
			//如果不是最后一个必经节点不可以更新endv的距离
			
			if(isInMinHeap(minHeap, v)&& dist[u] != MYINF&& pCrawl->cost + dist[u] < dist[v])
			{
				//这里堆中点连接的点不是得经过的点则更新，他连接点的信息
				dist[v] = dist[u] + pCrawl->cost;
				path[v]=u;
				arc[v]=pCrawl->arcId;//arc[v]表示u->v的弧
				//距离更新了之后，要调整最小堆
				decreaseKey(minHeap, v, dist[v]);//这里的v是节点编号，前面pos[v]=j;

			}
			
			pCrawl = pCrawl->nextLink;
		}
	}

	return -1;
}
int dijkstraClosedNcost(OrGraph *G,int src,float dist[],int path[],int arc[],bool flag[], int *keyNum,int endv,bool isEnd){
	//最小优先队列，数组的方式,以后换feib堆。
	int vNum=G->vexNum;
	//我先只找到必经节点的最短距离，那么我不打endv加进来不就可以了吗，还费事，等找了这些之后再找最后一个必经到最后一个结点的值
	MinHeap* minHeap=createMinHeap(vNum);
	//初始化堆包含所有的顶点
	int i,j=1;
	minHeap->array[0]=newMinHeapNode(src,dist[src]);
	

	if(!isEnd)
	{	
		for(i=0;i<vNum;++i)
		{
			
			if(!flag[i]&&i!=endv)//不加终点
			{
				dist[i]=MYINF;//dist[i]为MYINF可能
				minHeap->array[j]=newMinHeapNode(i,dist[i]);
				minHeap->pos[i]=j;
				++j;
				path[i]=-1;
			}
		}
	}
	else{
		for(i=0;i<vNum;++i)
		{
			if(!flag[i])//包含终点
			{

			dist[i]=MYINF;
			minHeap->array[j]=newMinHeapNode(i,dist[i]);
			minHeap->pos[i]=j;
			++j;
			path[i]=-1;
			}
		}
	}
	G->clist[src].key=false;//源必为flase不然就出去了
	minHeap->size = j;
	int v;
	while (!isEmpty(minHeap)) {
		MinHeapNode* minHeapNode = extractMin(minHeap);
		int u = minHeapNode->v;
		free(minHeapNode);
		if(G->clist[u].key)//加入堆中的是所有的flag为false的点。
		{
				
			--*keyNum;
			return u;
		}
		ArcNode* pCrawl = G->clist[u].firstLink;
		while (pCrawl != NULL) {

			v = pCrawl->headVex;
			//如果u结点连接的节点v为经过的节点则跳过这一个,如果v节点为endv节点的话，由于对endv节点是最后一次加入进去，且设为flag=false;
			//检醒所连接的点都是没有在路径上的，如果在的话则找下一个，加入堆中的节点都是还没有在路径上的
			if(flag[v])
			{
				pCrawl = pCrawl->nextLink;
				continue;
			}/**/
			
			//如果是endv的话但是不是最后一个结点，就下一个
			if(v==endv&&!isEnd)
			{
				pCrawl = pCrawl->nextLink;
				continue;
			}
			//如果不是最后一个必经节点不可以更新endv的距离
			
			if(isInMinHeap(minHeap, v)&& dist[u] != MYINF&& pCrawl->Ncost + dist[u] < dist[v])
			{
				dist[v] = dist[u] + pCrawl->Ncost;
				path[v]=u;
				arc[v]=pCrawl->arcId;//arc[v]表示u->v的弧
				decreaseKey(minHeap, v, dist[v]);//这里的v是节点编号，前面pos[v]=j;
			}
			
			pCrawl = pCrawl->nextLink;
		}
	}

	return -1;
}
bool minPath(OrGraph *G,int arr_se[],float dist[],int arc[],bool flag[],int path[]){
	//a->b
	int m,k,keyNum,endv,begn;
	bool isEnd=false;
	begn=arr_se[0];
	endv=arr_se[1];
	keyNum=arr_se[2];

	for(int o=0;o<G->vexNum;o++)
	{
		flag[o]=false;
		path[o]=-1;
		arc[o]=-1;
		dist[o]=MYINF;
	}
		/*------------------------------------求到初始点的第一个最短的必经节点---------------------------------*/
	flag[arr_se[0]]=true;//初始点标记为经过
	path[arr_se[0]]=arr_se[0];//前驱节点为自己
	dist[arr_se[0]]=0;//与前驱节点距离为零
	
	while(begn!=-1&&keyNum>0)
	{
		//printf("************循环第%d次哈哈v=%d,keyNum=%d***************\n",++i,begn,keyNum);
		m=begn;
		//dijkstraClosed(OrGraph *G,int src,int *dist,int *path,int *arc, int *keyNum,int endv,bool isEnd,int box[20])
		begn=dijkstraClosed(G,begn,dist,path,arc,flag,&keyNum,endv,isEnd);
		k=begn;
		
		while(m!=k&&k>=0)//这里要>=0由于0节点的点前驱也要赋值
		{
			
			flag[k]=true;
			k=path[k];
			
		}

	}
	isEnd=true;
	if(begn>=0)
	{
		
		dijkstraClosed(G,begn,dist,path,arc,flag,&keyNum,endv,isEnd);
	}
	

	if(path[arr_se[1]]!=-1)
	{
		return true;
	}
	return false;	
}
bool minPathNcost(OrGraph *G,int arr_se[],float dist[],int arc[],bool flag[],int path[]){
	//a->b
	int m,k,keyNum,endv,begn;
	bool isEnd=false;
	begn=arr_se[0];
	endv=arr_se[1];
	keyNum=arr_se[2];

	for(int o=0;o<G->vexNum;o++)
	{
		flag[o]=false;
		path[o]=-1;
		arc[o]=-1;
		dist[o]=MYINF;
	}
		/*------------------------------------求到初始点的第一个最短的必经节点---------------------------------*/
	flag[arr_se[0]]=true;//初始点标记为经过
	path[arr_se[0]]=arr_se[0];//前驱节点为自己
	dist[arr_se[0]]=0;//与前驱节点距离为零
	
	while(begn!=-1&&keyNum>0)
	{
		//printf("************循环第%d次哈哈v=%d,keyNum=%d***************\n",++i,begn,keyNum);
		m=begn;
		//dijkstraClosed(OrGraph *G,int src,int *dist,int *path,int *arc, int *keyNum,int endv,bool isEnd,int box[20])
		begn=dijkstraClosedNcost(G,begn,dist,path,arc,flag,&keyNum,endv,isEnd);
		k=begn;
		
		while(m!=k&&k>=0)//这里要>=0由于0节点的点前驱也要赋值
		{
		
			flag[k]=true;
			k=path[k];
			
		}

		
	}
	//G->clist[start_end[1]].key=true;//设置返回条件
	isEnd=true;
	if(begn>=0)
	{
		
		dijkstraClosedNcost(G,begn,dist,path,arc,flag,&keyNum,endv,isEnd);
	}
	

	if(path[arr_se[1]]!=-1)
	{
		return true;
	}
	return false;	
}
bool minPathMidst(OrGraph *G,OrGraph *RG, int start_end[],int mid,float dist[],int arc[],bool flag[],int path[],int unvoid[]){
	//a->b
	int m,n,k,keyNum;
	//ArcNode* w;
	bool isEnd=false;
	keyNum=start_end[2];
	n=mid;
	//printf("1__%d\n", mid);

	for(int o=0; o<start_end[2]; o++)
	{
		G->clist[unvoid[o]].key=true;
		RG->clist[unvoid[o]].key=true;
	}
	G->clist[start_end[0]].key=true;//第一个节点也是必经点，因为在上面算法中需要找必经节点的前一个必经节点。
	G->clist[start_end[1]].key=true;
	RG->clist[start_end[0]].key=true;//第一个节点也是必经点，因为在上面算法中需要找必经节点的前一个必经节点。
	RG->clist[start_end[1]].key=true;
	//初始化标记
	for(int o=0;o<G->vexNum;o++)
	{
		flag[o]=false;
		path[o]=-1;
		arc[o]=-1;
		dist[o]=MYINF;
	}
	flag[mid]=true;//初始点标记为经过
	//printf("%d\n",mid);
	path[mid]=mid;//前驱节点为自己
	dist[mid]=0;//与前驱节点距离为零


	while(n!=-1&&n!=start_end[0])//先逆向找头结点
	{
		m=n;
		n=dijkstraClosed(RG,n,dist,path,arc,flag,&keyNum,start_end[1],isEnd);//排除开始节点进入
		k=n;
		while(m!=k&&k>=0)//这里要>=0由于0节点的点前驱也要赋值
		{

			flag[k]=true;
			k=path[k];

		}
	}

	if(path[start_end[0]]!=-1)
	{
		
	
		n=mid;
		while(n!=-1&&keyNum>0)
		{
			m=n;
			n=dijkstraClosed(G,n,dist,path,arc,flag,&keyNum,start_end[1],isEnd);//中点到开始点，由于这是逆邻接表，所以要最后到开始点
			k=n;

			while(m!=k&&k>=0)//这里要>=0由于0节点的点前驱也要赋值
			{
				flag[k]=true;
				k=path[k];
			}
		}
		if(n>=0)
		{
			isEnd=true;
			n=dijkstraClosed(G,n,dist,path,arc,flag,&keyNum,mid,isEnd);
		}
	}

	
	if(path[start_end[1]]!=-1)//!=-1
	{
		
		n=start_end[0];

		m=arc[n];
		while(n!=path[n]&&n!=path[path[n]])
		{
			k=arc[path[n]];
			arc[path[n]]=m;
			m=k;
			n=path[n];

		}
		arc[start_end[0]]=m;
		n=start_end[0];
		m=path[n];
		while(m!=path[m]&&m!=path[path[m]])
		{
			k=path[m];
			path[m]=n;
			n=m;
			m=k;
		}
		path[m]=n;
		path[start_end[0]]=start_end[0];
		
		return true;
	}
	else
	{
		for(int o=0; o<start_end[2]; o++)
		{
			G->clist[unvoid[o]].key=true;
			RG->clist[unvoid[o]].key=true;
		}
		G->clist[start_end[0]].key=true;//第一个节点也是必经点，因为在上面算法中需要找必经节点的前一个必经节点。
		G->clist[start_end[1]].key=true;
		RG->clist[start_end[0]].key=true;//第一个节点也是必经点，因为在上面算法中需要找必经节点的前一个必经节点。
		RG->clist[start_end[1]].key=true;
		for(int o=0;o<G->vexNum;o++)
		{
			flag[o]=false;
			path[o]=-1;
			arc[o]=-1;
			dist[o]=MYINF;
		}
		flag[mid]=true;//初始点标记为经过
		path[mid]=mid;//前驱节点为自己
		dist[mid]=0;//与前驱节点距离为零

		isEnd=false;

		keyNum=start_end[2];
		n=mid;
		
		while(n!=-1&&n!=start_end[1])//从中间点到终点如遇到就返回
		{
			m=n;
			n=dijkstraClosed(G,n,dist,path,arc,flag,&keyNum,start_end[0],isEnd);//排除开始节点进入
			k=n;
			while(m!=k&&k>=0)//这里要>=0由于0节点的点前驱也要赋值
			{
				flag[k]=true;
				k=path[k];
			}

		}
		if(path[start_end[1]]!=-1)//正序时出来时终点，就是前面找到终点了的话，就开始找头
		{
			
			
			n=mid;

			while(n!=-1&&keyNum>0)
			{

				m=n;
				n=dijkstraClosed(RG,n,dist,path,arc,flag,&keyNum,start_end[0],isEnd);//中点到开始点，由于这是逆邻接表，所以要最后到开始点
				k=n;
				while(m!=k&&k>=0)//这里要>=0由于0节点的点前驱也要赋值
				{
					flag[k]=true;
					k=path[k];

				}
			}
			
			if(n>=0)
			{
				isEnd=true;
				dijkstraClosed(RG,n,dist,path,arc,flag,&keyNum,mid,isEnd);
				
			}
		}

		if(path[start_end[0]]!=-1)//如果头结点也找到了那就对了
		{
			n=start_end[0];
			m=arc[n];
			while(n!=path[n]&&n!=path[path[n]])
			{
				k=arc[path[n]];//记录前驱的弧
				arc[path[n]]=m;//前驱的弧，变成正向的位置，依次挪一位
				m=k;
				n=path[n];
			}
			arc[start_end[0]]=m;

			n=start_end[0];
			m=path[n];
			while(m!=path[m]&&m!=path[path[m]])
			{
				k=path[m];
				path[m]=n;
				n=m;
				m=k;
			}
			path[m]=n;
			path[start_end[0]]=start_end[0];
		
			return true;
		}
	}
	return false;	
}
bool minPathMidstNcost(OrGraph *G,OrGraph *RG, int start_end[],int mid,float dist[],int arc[],bool flag[],int path[],int unvoid[]){
	//a->b
	int m,n,k,keyNum;
	bool isEnd=false;
	keyNum=start_end[2];
	n=mid;

	for(int o=0; o<start_end[2]; o++)
	{
		G->clist[unvoid[o]].key=true;
		RG->clist[unvoid[o]].key=true;
	}
	G->clist[start_end[0]].key=true;//第一个节点也是必经点，因为在上面算法中需要找必经节点的前一个必经节点。
	G->clist[start_end[1]].key=true;
	RG->clist[start_end[0]].key=true;//第一个节点也是必经点，因为在上面算法中需要找必经节点的前一个必经节点。
	RG->clist[start_end[1]].key=true;
	//初始化标记
	for(int o=0;o<G->vexNum;o++)
	{
		flag[o]=false;
		path[o]=-1;
		arc[o]=-1;
		dist[o]=MYINF;
	}
	flag[mid]=true;//初始点标记为经过
	path[mid]=mid;//前驱节点为自己
	dist[mid]=0;//与前驱节点距离为零


	while(n!=-1&&n!=start_end[0])//先逆向找头结点
	{
		m=n;
		n=dijkstraClosedNcost(RG,n,dist,path,arc,flag,&keyNum,start_end[1],isEnd);//排除开始节点进入
		k=n;
		while(m!=k&&k>=0)//这里要>=0由于0节点的点前驱也要赋值
		{

			flag[k]=true;
			k=path[k];

		}
	}

	if(path[start_end[0]]!=-1)
	{//先通过逆邻接表找到起点，然后再去找终点
		
		
		n=mid;
		while(n!=-1&&keyNum>0)
		{
			m=n;
			n=dijkstraClosedNcost(G,n,dist,path,arc,flag,&keyNum,start_end[1],isEnd);//中点到开始点，由于这是逆邻接表，所以要最后到开始点
			k=n;

			while(m!=k&&k>=0)//这里要>=0由于0节点的点前驱也要赋值
			{
				flag[k]=true;
				k=path[k];
			}
		}
		if(n>=0)
		{
			isEnd=true;
			n=dijkstraClosedNcost(G,n,dist,path,arc,flag,&keyNum,mid,isEnd);
		}
	}

	
	if(path[start_end[1]]!=-1)//!=-1
	{
		
		//如果终点找到了的话，则进行相应表的数据的整理，由于逆序与下序所以表记录的不一样，下面转化为一样的
		n=start_end[0];

		m=arc[n];
		while(n!=path[n]&&n!=path[path[n]])
		{
			k=arc[path[n]];
			arc[path[n]]=m;
			m=k;
			n=path[n];

		}
		arc[start_end[0]]=m;
		n=start_end[0];
		m=path[n];
		while(m!=path[m]&&m!=path[path[m]])
		{
			k=path[m];
			path[m]=n;
			n=m;
			m=k;
		}
		path[m]=n;
		path[start_end[0]]=start_end[0];
		
		return true;
	}
	else
	{
		
		for(int o=0; o<start_end[2]; o++)
		{
			G->clist[unvoid[o]].key=true;
			RG->clist[unvoid[o]].key=true;
		}
		G->clist[start_end[0]].key=true;//第一个节点也是必经点，因为在上面算法中需要找必经节点的前一个必经节点。
		G->clist[start_end[1]].key=true;
		RG->clist[start_end[0]].key=true;//第一个节点也是必经点，因为在上面算法中需要找必经节点的前一个必经节点。
		RG->clist[start_end[1]].key=true;
		//初始化标记
		for(int o=0;o<G->vexNum;o++)
		{
			flag[o]=false;
			path[o]=-1;
			arc[o]=-1;
			dist[o]=MYINF;
		}
		flag[mid]=true;//初始点标记为经过
		path[mid]=mid;//前驱节点为自己
		dist[mid]=0;//与前驱节点距离为零

		isEnd=false;

		keyNum=start_end[2];
		n=mid;
		
		while(n!=-1&&n!=start_end[1])//从中间点到终点如遇到就返回
		{
			m=n;
			n=dijkstraClosedNcost(G,n,dist,path,arc,flag,&keyNum,start_end[0],isEnd);//排除开始节点进入
			k=n;
			
			while(m!=k&&k>=0)//这里要>=0由于0节点的点前驱也要赋值
			{
				//把经过的点标记为真，以示用过，由于n可能为-1所以用m;
				flag[k]=true;
				k=path[k];
			}

		}
		
		if(path[start_end[1]]!=-1)//正序时出来时终点，就是前面找到终点了的话，就开始找头
		{
			
			
			n=mid;

			while(n!=-1&&keyNum>0)
			{

				m=n;
				n=dijkstraClosedNcost(RG,n,dist,path,arc,flag,&keyNum,start_end[0],isEnd);//中点到开始点，由于这是逆邻接表，所以要最后到开始点
				k=n;
				while(m!=k&&k>=0)//这里要>=0由于0节点的点前驱也要赋值
				{
					flag[k]=true;
					k=path[k];

				}
			}
			
			if(n>=0)
			{
				isEnd=true;
				dijkstraClosedNcost(RG,n,dist,path,arc,flag,&keyNum,mid,isEnd);
			}
		}

		if(path[start_end[0]]!=-1)//如果头结点也找到了那就对了
		{
			n=start_end[0];
			m=arc[n];
			while(n!=path[n]&&n!=path[path[n]])
			{
				k=arc[path[n]];//记录前驱的弧
				arc[path[n]]=m;//前驱的弧，变成正向的位置，依次挪一位
				m=k;
				n=path[n];
			}
			arc[start_end[0]]=m;

			n=start_end[0];
			m=path[n];
			while(m!=path[m]&&m!=path[path[m]])
			{
				k=path[m];
				path[m]=n;
				n=m;
				m=k;
			}
			path[m]=n;
			path[start_end[0]]=start_end[0];
		
			return true;
		}
	}
	return false;	
}

/***********************************************************************************************************/

/**************************************************逆邻接表工具函数*****************************************/
//转换起始点与初始点，在用逆邻接表时要把原来的终点转换为起点
void changeStart_end(int se[]){
	int tmp;
	tmp=se[1];
	se[1]=se[0];
	se[0]=tmp;
}


/**************************************************执行入口函数*********************************************/

void search_route(char *topo[5000], int edge_num, char *demand){
	
	/***********************************准备工作***********************************************/
	
	OrGraph *RG=(OrGraph *)calloc(1,sizeof(OrGraph));	
	OrGraph *G=(OrGraph *)calloc(1,sizeof(OrGraph));
	OrGraph *RG_C=(OrGraph *)calloc(1,sizeof(OrGraph));	
	OrGraph *G_C=(OrGraph *)calloc(1,sizeof(OrGraph));

	/*-----------------解析demand命令，找出始末点，必经点个数，以及存放必经点------*/
	int start_end[3];//正数组
	int Rstart_end[3];//逆数组
	int unvoid[50];//unvoid为必经点数组
	explain_demand(start_end,unvoid,demand);
	for(int i=0;i<3;i++){Rstart_end[i]=start_end[i];}//赋值
	changeStart_end(Rstart_end);
	
	
	/*-----------------------------建立逆邻接表，或逆邻接表--------------------*/
	
	bool isRev=false;
	createOG(G, topo,edge_num,isRev);
	createOG(G_C, topo,edge_num,isRev);
	isRev=true;
	createOG(RG, topo,edge_num,isRev);
	createOG(RG_C, topo,edge_num,isRev);

	/*-------------------------------------------初始化必经点------------------------*/
	for(int i=0; i<Rstart_end[2]; i++)
	{
		RG->clist[unvoid[i]].key=true;
		G->clist[unvoid[i]].key=true;

		RG_C->clist[unvoid[i]].key=true;
		G_C->clist[unvoid[i]].key=true;
	}
	RG->clist[Rstart_end[0]].key=true;//第一个节点也是必经点，因为在上面算法中需要找必经节点的前一个必经节点。
	RG->clist[Rstart_end[1]].key=true;//终点为必经节点，在路径检测时间会用到
	G->clist[start_end[0]].key=true;//第一个节点也是必经点，因为在上面算法中需要找必经节点的前一个必经节点。
	G->clist[start_end[1]].key=true;//终点为必经节点，在路径检测时间会用到

	RG_C->clist[Rstart_end[0]].key=true;//第一个节点也是必经点，因为在上面算法中需要找必经节点的前一个必经节点。
	RG_C->clist[Rstart_end[1]].key=true;//终点为必经节点，在路径检测时间会用到
	G_C->clist[start_end[0]].key=true;//第一个节点也是必经点，因为在上面算法中需要找必经节点的前一个必经节点。
	G_C->clist[start_end[1]].key=true;//终点为必经节点，在路径检测时间会用到

	/*------------------------------------入度或出度为1的必经节点其前驱和后继的减枝-------—--------------------------*/
	bool *mark1=(bool *)calloc(G->vexNum+1,sizeof(bool));//假定这个点是连续的才行啊，不行就改成MAX_VEX
	bool *mark2=(bool *)calloc(G->vexNum+1,sizeof(bool));//假定这个点是连续的才行啊，不行就改成MAX_VEX
	removeBENode(G,RG_C,start_end);
	removeBENode(RG,G_C,Rstart_end);
	removeZero(G, RG_C,start_end,mark1,mark2);//这个加上还有错，晕
	removeZero(RG,G_C, Rstart_end,mark1,mark2);
	//printf("999999999\n");
	cut(G,RG_C,unvoid,start_end,mark1,mark2);//剪枝,应该至少没有影响才对，还有问题
	cut(RG,G_C,unvoid,Rstart_end,mark1,mark2);
	

/*-----------------------------------------------初始化标记数组----------------------------------------*/
	
	int path[RG->vexNum];//前驱数组
	bool flag[RG->vexNum];//标记数组
	float dist[RG->vexNum];//距离数组
	int arc[RG->vexNum];//弧标记

	//float cost=MYINF;
	int path_min[RG->vexNum];
	int arc_min[RG->vexNum];
	memset(path_min,-1,sizeof(path_min));
	memset(arc_min,-1,sizeof(arc_min));
	
/*------------------------------------------------开始计算----------------------------------------------*/
	int iBLTest;
//A、B、C、D……为不同网络选路模型
	if(G->vexNum>50 && G->vexNum<=100)
	{
//A
		//每次产生一个固定的随机数，如果用全随机的话可能求到最优但不够稳定
		//srand((int)time(0));
		iBLTest=0;
		do{
		
			updateNCost(G,6);
			updateNCost(RG,6);
			solve(G, start_end, RG, Rstart_end, dist, arc, flag, path, path_min, arc_min, unvoid,&isRev);

		}while(++iBLTest < 5);
	}

	if(G->vexNum>100 && G->vexNum<=150)
	{
//B
		//srand((int)time(0));
		iBLTest=0;
		do{
			abc=0.173;
			updateNCost(G,7);
			abc=0.240;
			updateNCost(RG,7);
			solve(G, start_end, RG, Rstart_end, dist, arc, flag, path, path_min, arc_min, unvoid,&isRev);

		}while(++iBLTest < 1);
	}
		
//C	
	if(G->vexNum>150 && G->vexNum<=200)
	{	
		//srand((int)time(0));
		iBLTest=0;
		do{
			abc=0.449;
			updateNCost(G,8);
			abc=0.827;
			updateNCost(RG,8);
			solve(G, start_end, RG, Rstart_end, dist, arc, flag, path, path_min, arc_min, unvoid,&isRev);

		}while(++iBLTest < 1);
	}	
//E	
	if(G->vexNum>200 && G->vexNum<=250)
	{	
	//srand((int)time(0));
		iBLTest=0;
		do{
			abc=0.275;
			updateNCost(G,9);
			abc=0.21661;
			updateNCost(RG,9);
			solve(G, start_end, RG, Rstart_end, dist, arc, flag, path, path_min, arc_min, unvoid,&isRev);

		}while(++iBLTest < 1);

	}	
//F	
	if(G->vexNum>250 && G->vexNum<=300)
	{
		iBLTest=0;
		do{
			abc=0.421;
			updateNCost(G,10);
			abc=0.329;
			updateNCost(RG,10);
			solve(G, start_end, RG, Rstart_end, dist, arc, flag, path, path_min, arc_min, unvoid,&isRev);

		}while(++iBLTest < 1);
	
	}	
//MA、MB……多种模型取最优
	if(G->vexNum<=50)
	{
//MA		
		solve(G, start_end, RG, Rstart_end, dist, arc, flag, path, path_min, arc_min, unvoid,&isRev);
//MB
		updateNCost(G,1);
		updateNCost(RG,1);
		solve(G, start_end, RG, Rstart_end, dist, arc, flag, path, path_min, arc_min, unvoid,&isRev);
//MC
		updateNCost(G,2);
		updateNCost(RG,2);
		solve(G, start_end, RG, Rstart_end, dist, arc, flag, path, path_min, arc_min, unvoid,&isRev);	
//MD
		//float pare[14][3]={{2,1,1},{-0.919,1.6,2.58},{0.23,1,1},{-0.3,1,1},{0.6,-0.1,0.9},{2,-0.5,0},{0.2,0.5,0},{0.11,0,-0.1},{0.41,0.3,0.1},{0.1,2,5},{-0.919,1.5,-0.68},{0.919,1.6,-1.58},{2.11,-1.4,0.8},{0.01,0.3,1.54}};
		//模型经验参数
		float pare[13][3]={{2,1,1},{-0.919,1.6,2.58},{0.23,1,1},{-0.3,1,1},{0.6,-0.1,0.9},{2,-0.5,0},{0.2,0.5,0},{0.11,0,-0.1},{0.41,0.3,0.1},{0.1,2,5},{-0.919,1.5,-0.68},{2.11,-1.4,0.8},{0.01,0.3,1.54}};
//ME		
		for(int i=0;i<13;i++)
		{
			updateNCostJoin(G,pare[i],1);
			updateNCostJoin(RG,pare[i],1);
			solve(G, start_end, RG, Rstart_end, dist, arc, flag, path, path_min, arc_min, unvoid,&isRev);

		}
//MF 换模型
		for(int i=0;i<13;i++)
		{
			updateNCostJoin(G,pare[i],2);
			updateNCostJoin(RG,pare[i],2);
			solve(G, start_end, RG, Rstart_end, dist, arc, flag, path, path_min, arc_min, unvoid,&isRev);
		}				
//MG
		//srand((int)time(0));
		iBLTest=0;
		do{			
			updateNCost(G,4);
			updateNCost(RG,4);
			solve(G, start_end, RG, Rstart_end, dist, arc, flag, path, path_min, arc_min, unvoid,&isRev);
		}while(++iBLTest < 10);
//MH
		//srand((int)time(0));
		updateNCost(G,5);
		updateNCost(RG,5);
		solve(G, start_end, RG, Rstart_end, dist, arc, flag, path, path_min, arc_min, unvoid,&isRev);
//MI
		//srand((int)time(0));
		iBLTest=0;
		do{		
			updateNCost(G,3);
			updateNCost(RG,3);
			solve(G, start_end, RG, Rstart_end, dist, arc, flag, path, path_min, arc_min, unvoid,&isRev);
		}while(++iBLTest < 20);
	}
	//针对不同规模
	//srand((int)time(0));
	if(edge_num > 1300 && edge_num <=2300)
	{
//EA
		if(start_end[2] <=22)
		{
			//srand((int)time(0));
			iBLTest=0;
			do{			
				updateNCost(G,12);
				updateNCost(RG,12);				
				solve(G, start_end, RG, Rstart_end, dist, arc, flag, path, path_min, arc_min, unvoid,&isRev);
			}while(++iBLTest < 100);
		}
//EB
		else if(start_end[2] >22 && start_end[2] <=25)
		{
			//srand((int)time(0));
			iBLTest=0;
			do{
			
				updateNCost(G,13);
				updateNCost(RG,13);

				solve(G, start_end, RG, Rstart_end, dist, arc, flag, path, path_min, arc_min, unvoid,&isRev);

			}while(++iBLTest < 100);
		}
//EC
		else if(start_end[2]>25 && start_end[2] <= 50)
		{
			//srand((int)time(0));
			iBLTest=0;
			do{
			
				updateNCost(G,11);
				updateNCost(RG,11);
				
				solve(G, start_end, RG, Rstart_end, dist, arc, flag, path, path_min, arc_min, unvoid,&isRev);

			}while(++iBLTest < 30);

		}
		
	}
	//针对边数大的
	if(edge_num>2300)	
	{
//ED
		//if(start_end[2]>20)
		//{
			//srand((int)time(0));
			iBLTest=0;
			do{
				updateNCost(G,14);
				updateNCost(RG,14);
				solve(G, start_end, RG, Rstart_end, dist, arc, flag, path, path_min, arc_min, unvoid,&isRev);
			}while(++iBLTest < 50);//150,200	
		//}
//EF
		//if(start_end[2]<=20)
		//{
			//srand((int)time(0));
			iBLTest=0;
			do{
				updateNCost(G,15);
				updateNCost(RG,15);
				solve(G, start_end, RG, Rstart_end, dist, arc, flag, path, path_min, arc_min, unvoid,&isRev);
			}while(++iBLTest < 50);//150,200	
		//}
	}
	//都没找到值返回
	if(kmark==false)
	{
		return;
	}
/*------------------------------------------------------整理数据，准备输出--------------------------------*/
	int nn=-1;
	OrGraph *R;
	if(isRev)
	{
		nn=Rstart_end[1];//终点
		R=RG;
	}
	else{
		nn=start_end[1];//终点
		R=G;
	}
	
	int i=0;
	int result[R->vexNum];

	while(nn!=path_min[nn]&&nn>=0)
	{
		result[i++]=arc_min[nn];
		nn=path_min[nn];

	}
	//输出，是以哪种计算得到的结果定
	if(isRev)
	{
		for(int j=0;j<i;j++)
		{
			record_result(result[j]);
		}	
	}
	else{
		for(int j=i-1;j>=0;j--)
		{
			record_result(result[j]);
		}	
	}
}

void cut(OrGraph *G1,OrGraph *G2,int unvoid[],int start_end[],bool *mark1,bool *mark2)
{
	//还需要初始化标记

	cutNeighborInD(G1,G2,start_end[1],mark1,start_end[0]);//结束节点入度为1的减枝
	cutNeighborOutD(G1,G2,start_end[0],mark2,start_end[1]);//开始节点出度为1的减枝

	for(int i=0;i<start_end[2];i++)//还可以加功能把初始点入度为0，终点出度为零
	{
		//邻接表通过逆邻接表找到前驱使入度为1的点的前驱的出度也为1，至于要把出度为1的后继的入度也改为1，就要搜索了
		cutNeighborInD(G1,G2,unvoid[i],mark1,start_end[0]);//剪去邻接表中点unvoid[i]的前驱的出度
		cutNeighborOutD(G1,G2,unvoid[i],mark2,start_end[1]);//剪去邻接表中点unvoid[i]的后继的入度
		
	}
}
//入度是向上剪枝，出度是向下剪枝
void cutNeighborInD(OrGraph *G1,OrGraph *G2,int v,bool *mark1,int beg)
{
	//该点的入度为1则把其前驱结点的其它出度删去，G1，G2互为逆邻接表
	
	if(G1->clist[v].inDegree==1&&!mark1[v])
	{
		ArcNode *tmp1,*tmp2;//静态局部变量，只在这个函数中定义一次。
		mark1[v]=true;
		int vt;
		tmp1=G2->clist[v].firstLink;//该点u逆邻上的第一条弧,其连接的就是v的前驱
		tmp2=G1->clist[tmp1->headVex].firstLink;//上面弧连接的点的弧
		while(tmp2&&tmp2->nextLink && tmp2->nextLink->headVex!=v)
		{
			--G1->clist[tmp2->nextLink->headVex].inDegree;
			vt=tmp2->nextLink->headVex;
			tmp2->nextLink=tmp2->nextLink->nextLink;

			if(G1->clist[vt].inDegree==1 && G1->clist[vt].key)//这里可以不用前面那个度判断，这样加上只是为了避免进入函数而降低执行速度
			{
				cutNeighborInD(G1,G2,vt,mark1,beg);
			}
			else if(G1->clist[vt].inDegree==0 && vt!=beg)
			{
				removeInZeroNode(G1,G2,vt,mark1,beg);
			}
			
			tmp2=tmp2->nextLink;
		}

		tmp2=G1->clist[tmp1->headVex].firstLink;
		if(tmp2 && tmp2->headVex!=v)
		{
			
			--G1->clist[tmp2->headVex].inDegree;
			G1->clist[tmp1->headVex].firstLink=tmp2->nextLink;
			if(G1->clist[tmp2->headVex].inDegree==1 && G1->clist[tmp2->headVex].key)
			{
				cutNeighborInD(G1,G2,tmp2->headVex,mark1,beg);
			}
			else if(G1->clist[tmp2->headVex].inDegree==0 && tmp2->headVex!=beg)
			{
				removeInZeroNode(G1,G2,tmp2->headVex,mark1,beg);
			}
			
		}
		G1->clist[tmp1->headVex].outDegree=1;

		//如果不是必经则递归删除，因为是必经的话会在删除的范围的，这里避免重复操作。
		//v=tmp1->headVex;//记录其前驱节点的节点ID,方便递归删除,当然这里删除是因为它不是必经节点，只是因为必经的前驱只有它，所以它和必经其实是等价的
		//由于这里的tmp1->headVex剪枝不影响必经节点的入度，而必经节点最终都要遍历的
		//考虑到减支对其它节点的影响，这里是不是还要补充一点，如果减去的是另一个必经的入度，那么就要调整该点的入度，查看是否为1然后减枝。
		if(!G1->clist[tmp1->headVex].key)
		{cutNeighborInD(G1,G2,tmp1->headVex,mark1,beg);}
	}	
}
void cutNeighborOutD(OrGraph *G1,OrGraph *G2,int v,bool *mark2,int end)
{
	
	if(G1->clist[v].outDegree==1&&!mark2[v])
	{
		ArcNode *tmp1,*tmp2,*tmp3;
		mark2[v]=true;
		//printf("%d_%d\n", v,G1->clist[v].outDegree);
		tmp1=G1->clist[v].firstLink;//后继

		tmp2=G2->clist[tmp1->headVex].firstLink;//后继的逆邻接表

		while(tmp2)//后继的逆邻接表减枝，因为unvoid[i]为必经所以它的唯一后继必须经过它，在逆邻接表中就是它的前驱必经过它，所以把逆邻接表中的其它出去的分支去掉
		{
			if(tmp2->headVex!=v)//这里要除去为必经点的那个头节点
			{
				tmp3=G1->clist[tmp2->headVex].firstLink;//取得指向出度为1的后继的其它前驱节点
				
				//先while把边中的指向出度为1的后继的其它前驱去掉，再看看第一条边，即与头节点相连的那条边，如果是的话，把后面的边连接到头节点
				//剪了之后导致其它节点的出度可能为1，但是这不重要，因为其它节点可能不是必经节点。如果是必经节点的话，那那还是要继续剪，有没有意义呢，可能有的，不防试试
				while(tmp3)
				{
					if(tmp3->nextLink && tmp3->nextLink->headVex==tmp1->headVex)
					{
						tmp3->nextLink=tmp3->nextLink->nextLink;
						//break;如果有多条一个点同时指向该唯一后继的前驱节点的话就要继续找而不用break;
						--G1->clist[tmp2->headVex].outDegree;
					}
					tmp3=tmp3->nextLink;
				}

				//再取一次，检查与头节点相连的第一条边
				tmp3=G1->clist[tmp2->headVex].firstLink;//取得指向出度为1的后继的其它前驱节点
				if(tmp3 && tmp3->headVex==tmp1->headVex)//把必经点唯一后继的其它前驱指向该节点边给去掉,为什么要单独拿出来，因为有一个点的头节点，如查第一条边就是那么就得把下一条边连接到头节点
				{
					G1->clist[tmp2->headVex].firstLink=tmp3->nextLink;
					--G1->clist[tmp2->headVex].outDegree;
				}
				//如果为必经，若剪了之后出度变为1则继续剪
				if(G1->clist[tmp2->headVex].outDegree==1 && G1->clist[tmp2->headVex].key)
				{
					cutNeighborOutD(G1,G2,tmp2->headVex,mark2,end);
				}
				else if(G1->clist[tmp2->headVex].outDegree==0 && tmp2->headVex!=end)
				{
					removeOutZeroNode(G1,G2,tmp2->headVex,mark2,end);
				}
				
			}
			tmp2=tmp2->nextLink;
			
		}

		//剪v的后继节点的入度，前不影响其后继节点的出度，所以这里不用递归，如果其出度为1的话会在剪的范围内
		//但是如果不是必经则在剪的范围，因为当必经节点的唯一后继节点的出度为1时，此时其和必经是一样的，相当于必经只有一条出度
		if(!G1->clist[tmp1->headVex].key)
		{cutNeighborOutD(G1,G2,tmp1->headVex,mark2,end);}
	}	
}
//去掉度为零的点，除了终点和起点
void removeZero(OrGraph *G1,OrGraph *G2, int start_end[],bool *mark1,bool *mark2)
{
	//去除每个出度为零的点
	for(int i=0;i < G1->vexNum;i++)
	{
		//其实由于减枝先后其实是相互影响的，管他的影响不大
		//removeOutZeroNode(RG,G,i,start_end[1]);
		removeInZeroNode(G1,G2,i,mark1,start_end[0]);
		removeOutZeroNode(G1,G2,i,mark2,start_end[1]);
	}
}
void removeOutZeroNode(OrGraph *G1,OrGraph *G2,int v,bool *mark2,int end)
{
	
	//除去开始与结束节点的出度为零的点来减小网络，删去无效的点，使其它点的无效边减少，方便减枝
	//为什么要除去起终点呢，当然要去除终点
	if(G1->clist[v].outDegree==0 && v!=end)
	{
		ArcNode *tmp1,*tmp2;

		tmp1=G2->clist[v].firstLink;

		while(tmp1)
		{	
			tmp2=G1->clist[tmp1->headVex].firstLink;
			while(tmp2)
			{
				if(tmp2->nextLink && tmp2->nextLink->headVex==v)
				{
					tmp2->nextLink=tmp2->nextLink->nextLink;
					--G1->clist[tmp1->headVex].outDegree;
				}
				tmp2=tmp2->nextLink;
			}
			tmp2=G1->clist[tmp1->headVex].firstLink;

			if(tmp2 && tmp2->headVex==v)
			{

				G1->clist[tmp1->headVex].firstLink=tmp2->nextLink;
				--G1->clist[tmp1->headVex].outDegree;
			}
			//出错的原因可能是在邻接表中删除了但在逆邻接表中还有这个，但是有判断啊怎么错了
			//最终出错原因是用的静态局部变量不能用在递归

			if(G1->clist[tmp1->headVex].outDegree==0)
			{
				removeOutZeroNode(G1,G2,tmp1->headVex,mark2,end);
			}
			else if(G1->clist[tmp1->headVex].outDegree==1 && G1->clist[tmp1->headVex].key)
			{
				cutNeighborOutD(G1,G2,tmp1->headVex,mark2,end);
			}
			tmp1=tmp1->nextLink;
		}
		G1->clist[v].inDegree=0;//出度为零的点入度清为零
	}
}
//把入度为零的点去掉，没有意义因为在邻接表中只是连接出度
//想了想还是有意义的，因为影响了别的点的度，而度的大小是影响选择的
void removeInZeroNode(OrGraph *G1,OrGraph *G2,int v,bool *mark1,int beg)
{
	
	if(G1->clist[v].inDegree==0 && v!=beg)//排除是起点的点
	{
		//removeEndNode(G1,i);//入度为零的就去除其出度，和终点去除其出度是一样的道理。
		ArcNode *tmp1;
		int vt;//这里用于临时存放顶点
		tmp1=G1->clist[v].firstLink;//获得终点节点的后继

		while(tmp1&&tmp1->nextLink)
		{
			--G1->clist[tmp1->nextLink->headVex].inDegree;
			vt=tmp1->nextLink->headVex;
			tmp1->nextLink=tmp1->nextLink->nextLink;
			
			if(G1->clist[vt].inDegree==0)
			{
				removeInZeroNode(G1,G2,vt,mark1,beg);
			}
			else if(G1->clist[vt].inDegree==1 && G1->clist[vt].key)
			{
				cutNeighborInD(G1,G2,vt,mark1,beg);
			}
			tmp1=tmp1->nextLink;
		}
		tmp1=G1->clist[v].firstLink;//获得终点节点的后继
		if(tmp1)
		{
			--G1->clist[tmp1->headVex].inDegree;
			G1->clist[v].firstLink=tmp1->nextLink;

			if(G1->clist[tmp1->headVex].inDegree==0)
			{
				removeInZeroNode(G1,G2,tmp1->headVex,mark1,beg);
			}
			else if(G1->clist[tmp1->headVex].inDegree==1 && G1->clist[tmp1->headVex].key)
			{
				cutNeighborInD(G1,G2,tmp1->headVex,mark1,beg);
			}
			
		}
		G1->clist[v].outDegree=0;//入度为零的点，出度清为零
	}		
}
//再看看两个邻接表依赖问题
//去除开始节点的前驱，使之前驱的度减一在之后的去度可能有帮助
void removeBeginNode(OrGraph *G1, OrGraph *G2,int begin)
{
	//操作的是G1，G2是作为参考的
	ArcNode *tmp1,*tmp2;
	tmp1=G2->clist[begin].firstLink;//开始节点的邻接边
	while(tmp1)//寻找开始节点前驱
	{
		tmp2=G1->clist[tmp1->headVex].firstLink;//寻找开始节点前驱的后继
		while(tmp2&&tmp2->nextLink)
		{
			if(tmp2->nextLink->headVex==begin)//后继等于开始节点的则去除,这里这样子是因为有个头节点，其链域不一样，下面if专家处理头节点下的第一个节点相同情况下
			{
				tmp2->nextLink=tmp2->nextLink->nextLink;
				--G1->clist[tmp1->headVex].outDegree;
			}
			tmp2=tmp2->nextLink;
		}
		tmp2=G1->clist[tmp1->headVex].firstLink;//寻找开始节点前驱的后继
		if(tmp2 && tmp2->headVex==begin)
		{
			G1->clist[tmp1->headVex].firstLink=tmp2->nextLink;
			--G1->clist[tmp1->headVex].outDegree;
		}
		
		tmp1=tmp1->nextLink;
	}
	G1->clist[begin].inDegree=0;//起点，入度清为零
}
void removeEndNode(OrGraph *G1,int end)
{
	ArcNode *tmp1;
	tmp1=G1->clist[end].firstLink;//获得终点节点的后继

	while(tmp1&&tmp1->nextLink)
	{
		--G1->clist[tmp1->nextLink->headVex].inDegree;
		tmp1->nextLink=tmp1->nextLink->nextLink;
		tmp1=tmp1->nextLink;
	}
	tmp1=G1->clist[end].firstLink;//获得终点节点的后继
	if(tmp1)
	{
		--G1->clist[tmp1->headVex].inDegree;
		G1->clist[end].firstLink=tmp1->nextLink;
	}
	G1->clist[end].outDegree=0;//终点，出度清为零
}
void removeBENode(OrGraph *G1,OrGraph *G2,int start_end[])//去除开始点的前驱和结束点的后继并调整度
{
	removeBeginNode(G1,G2,start_end[0]);
	removeEndNode(G1,start_end[1]);
}
//找到弧权值
int findArcCost(OrGraph *G,int pnode,int arcId)
{
	
	//int arcn=-1;
	if(pnode<0){return 0;}
	int cost=MYINF;
	ArcNode *p= G->clist[pnode].firstLink;
	while(p!=NULL)
	{
		if(p->arcId==arcId)
		{

			cost=p->cost;
			return cost;//有弧id是确定的
		}
		p=p->nextLink;
	}
	return cost;
}
//计算权值
float calCost(OrGraph *G,int end,int path[],int arc[])
{
	int nn;
	float cost=0;
	int myArcId;
	nn=end;
	while(nn!=path[nn]&&nn>=0)
	{

		myArcId=arc[nn];//临时记录
		nn=path[nn];
		//由于记录弧是从出发点后面一个节点开始的所以，还有由于记录弧时是从邻接表中找，所以要从上一个节点找
		cost=cost+findArcCost(G,nn,myArcId);
	}
	return cost;
}
float calNodeInDeAverage(OrGraph *G)
{
	float totalInDe=0;
	for(int i=0;i<G->vexNum;i++)
	{
		totalInDe=totalInDe+G->clist[i].inDegree;
	}
	return totalInDe/G->vexNum;
}
float calNodeInDeStdDe(OrGraph *G,float ave)
{

	float tSqu=0;
	for(int i=0;i<G->vexNum;i++)
	{
		tSqu=tSqu + pow(G->clist[i].inDegree-ave,2);
	}
	return sqrt(tSqu/G->vexNum);
}
float calNodeOutDeAverage(OrGraph *G)
{
	float totalOutDe=0;
	for(int i=0;i<G->vexNum;i++)
	{
		totalOutDe=totalOutDe+G->clist[i].outDegree;
	}
	return totalOutDe/G->vexNum;
}
float calNodeOutDeStdDe(OrGraph *G,float aveO)
{
	//float ave=calNodeInDeAverage(G);
	float tSquO=0;
	for(int i=0;i<G->vexNum;i++)
	{
		tSquO=tSquO + pow(G->clist[i].outDegree-aveO,2);
	}
	return sqrt(tSquO/G->vexNum);
}
void updateNCost(OrGraph *G, int iAlgorithm)
{
	ArcNode *p;
	float ave=calNodeInDeAverage(G);
	float dev=calNodeInDeStdDe(G,ave);

	float aveO=calNodeOutDeAverage(G);
	float devO=calNodeOutDeStdDe(G,aveO);

	float inDe,outDe;
	float inDeStd,outDeStd;
	float inDeStd1,outDeStd1;

	for(int i=0;i<G->vexNum;i++)
	{
		p=G->clist[i].firstLink;
		while(p)
		{

			inDe=G->clist[p->headVex].inDegree<16?G->clist[p->headVex].inDegree:16;
			outDe=G->clist[p->headVex].outDegree;
			//标准化0
			inDeStd=(inDe-ave)/dev;
			outDeStd=(outDe-aveO)/devO;
			//标准化1
			inDeStd1=(16+inDe-2*ave)/dev;
			outDeStd1=G->clist[p->headVex].outDegree/16.0;
			//p->Ncost=outDeStd * (inDestd+(20+p->cost-21)/5.5);标准化算耗费，入度越大耗费越大，
			switch(iAlgorithm)
			{
				case 1: //A
				{
					p->Ncost=outDeStd1 +(inDeStd1+(p->cost-1)/5.5);
					break;
				}
				case 2: //C
				{
					p->Ncost=0.11*outDeStd + 3*inDeStd + 1.54*(2+(p->cost-10.5)/5.5);
					break;
				}
				case 3: //H
				{
					p->Ncost = 0.*inDe - 0.1*outDe + 1.68*log(p->cost)/log(3.2) + 1.08*abc;
					break;
					
				}
				case 4: //F
				{
					abc = (rand()/(RAND_MAX+1.0));
					p->Ncost=0.168*outDeStd + 0.628*inDeStd + 0.*(p->cost - 10.5)/5.5 + 5*abc;
					break;
				}
				case 5: //G
				{
					p->Ncost = inDe - outDe + p->cost - p->cost*sin(50*p->cost*3.14159/180);
					break;
				}
				case 6: //6
				{
					abc = (rand()/(RAND_MAX+1.0));
					p->Ncost=0.1*outDeStd + 0.3*inDeStd + 0.1*(p->cost - 10.5)/5.5 + abc;
					break;
				}
				case 7: //7
				{
					p->Ncost=0.314111-0.5*abc*outDeStd + 0.3812*inDeStd + 0.5644*(p->cost - 10.5)/5.5 + 1.592*abc;
					break;
				}
				case 8: //8
				{
					p->Ncost=0.314111-0.5*abc*outDeStd + 0.3812*inDeStd + 0.5644*(p->cost - 10.5)/5.5 + 1.592*abc;
					break;
				}
				case 9: //9
				{
					p->Ncost=0.284111+0.432*abc*outDeStd + abc*abc*0.3412*inDeStd + 0.7104*(p->cost - 10.5)/5.5 + 4.582*abc;
					break;
				}
				case 10: //10
				{
					p->Ncost = 0.*inDe - 0.1*outDe + 1.68*log(p->cost)/log(3.2) + 1.08*abc;
					break;
				}
				case 11: //11
				{
					abc = (rand()/(RAND_MAX+1.0));
					p->Ncost = 0.01*inDe + 0.01* outDe + 0.01*p->cost + p->cost+0.8*abc; //
					break;
				}
				case 12: //12
				{
					abc = (rand()/(RAND_MAX+1.0));
					p->Ncost = 0.*inDe + 0.* outDe + 1.18*log(p->cost)/log(3.2)+1.08*abc; //
					break;
				}
				case 13: //13
				{
					abc = (rand()/(RAND_MAX+1.0));
					p->Ncost = 0.3*outDeStd + 0.1* inDeStd + log(p->cost)/log(3.2)+0.5*pow(sin(abc),2); //
					break;
				}
				case 14: //14
				{
					abc = (rand()/(RAND_MAX+1.0));
					p->Ncost = 0.*inDe + 0.* outDe + p->cost + 1.628*abc; //
					break;
				}
				case 15: //15
				{
					abc = (rand()/(RAND_MAX+1.0));
					p->Ncost=1.1284+0.2*outDeStd + 0.322*inDeStd + (p->cost - 10.5)/5.5 + 1.8555*abc; //
					break;
				}
			}
			p=p->nextLink;
		}
	}
}
void updateNCostJoin(OrGraph *G, float join[], int iAlgorithm)
{
	ArcNode *p;
	float ave=calNodeInDeAverage(G);
	float dev=calNodeInDeStdDe(G,ave);

	float aveO=calNodeOutDeAverage(G);
	float devO=calNodeOutDeStdDe(G,aveO);

	float inDeStd,outDeStd;
	float inDe,outDe;
	for(int i=0;i<G->vexNum;i++)
	{
		p=G->clist[i].firstLink;
		while(p)
		{
			
			inDe=G->clist[p->headVex].inDegree<16?G->clist[p->headVex].inDegree:16;
			outDe=G->clist[p->headVex].outDegree;
			inDeStd=(inDe-ave)/dev;
			outDeStd=(outDe-aveO)/devO;
			switch(iAlgorithm)
			{
				case 1: //B
				{
					p->Ncost=5+join[0]*outDeStd + join[1]*inDeStd + join[2]*(p->cost-10.5)/5.5;
					break;
				}
				case 2: //D
				{
					p->Ncost=10+join[0]*outDeStd + join[1]*inDeStd + join[2]*(p->cost-10.5)/5.5;
					break;
				}				
			}
			p=p->nextLink;
		}
	}
}
void solve(OrGraph *G, int start_end[], OrGraph *RG , int Rstart_end[], float dist[] ,int arc[] ,bool flag[], int path[],int path_min[],int arc_min[],int unvoid[], bool *isR)
{
	int cost1;
	long int len=G->vexNum*sizeof(int);
	if(minPathNcost(RG,Rstart_end,dist,arc,flag,path))
	{
		kmark=true;
		cost1=calCost(RG,Rstart_end[1],path,arc);
		if(cost1<mcost)
		{
			mcost=cost1;
			memcpy(path_min,path,len);
			memcpy(arc_min,arc,len);
			*isR=true;
		}
	}
	
	if(minPathNcost(G,start_end,dist,arc,flag,path))
	{
		kmark=true;
		cost1=calCost(G,start_end[1],path,arc);
		if(cost1<mcost)
		{
			mcost=cost1;
			memcpy(path_min,path,len);
			memcpy(arc_min,arc,len);
			*isR=false;
		}
	}
	
	for(int i=0;i<start_end[2];i++)
	{
		if(minPathMidstNcost(G,RG,start_end,unvoid[i],dist,arc,flag,path,unvoid))
		{
			kmark=true;
			cost1=calCost(G,start_end[1],path,arc);
			if(cost1<mcost)
			{
				mcost=cost1;
				memcpy(path_min,path,len);
				memcpy(arc_min,arc,len);
				*isR=false;
			}
		}
	}
}
