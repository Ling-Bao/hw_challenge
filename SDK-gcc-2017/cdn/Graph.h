/*
 * Copyright 2017
 *
 * Graph.h
 *
 * 图信息
 *
 * Date: March 26, 2017
 * Author: Wei Liu, Ling Bao, Shangyue Zhou
 *
 * $Id: Graph.h,v 0.1 2017/03/26 00:30:30 lw Exp $
 *
 */

#ifndef CDN_GRAPH_H
#define CDN_GRAPH_H

/**
 * 边结点，横坐标为起始节点，纵坐标为结束节点，消费节点序号顺次增加。
 * 包含：
 * 开始节点，暂时不需要。
 * 结束节点，暂时不需要。
 * 容量/消费需求带宽
 * 单价
 * 消费节点编号，暂时不需要。
 * 未知数序号,为矩阵中从左至右，从上至下边的序号
 */
struct Arc{
    int node0;//begin
    int node1;//end
    double capacity;
    double cost;
    int id;
    Arc *next;
    Arc(): node0{-1},node1{-1},capacity(0.0),cost(MAXVALUE),id{-1},next{nullptr} {}
};
/**头节点
 * isWithConsumer  该节点是否连接消费节点
 * consumerNum     消费节点编号
 * require         消费需求
 */
struct Node{
    bool isWithConsumer;
    int consumerId;
    double require;
    Arc *arc;
    Node():isWithConsumer{false},consumerId{-1},require{0.0},arc{nullptr} {}
};
struct Graph {
    static int nodeCount;
    static int arcCount;
    static int consumerCount;
    static double serverFee;

    static Arc* gNet[MAXNODE][MAXNODE];
    /**邻接表头节点，可以查找服务节点对应消费节点*/
    static Node* netNode[MAXNODE];
    /**消费节点对应服务节点*/
    static int consumerNode[MAXCONSUMER];


    void init(char * topo[]);
    void gNetBuild(char * topo[]);
    void getBaseInfo(char * topo[]);
    void netNodeBuild(char *topo[]);
    void setGNetArcId();
    void printGNet();
    void printNetNode();
    void printConsumerNode();
};

int Graph::nodeCount(0);
int Graph::arcCount{0};
int Graph::consumerCount{0};
double Graph::serverFee{MAXVALUE};
Arc* Graph::gNet[MAXNODE][MAXNODE]{nullptr};
Node* Graph::netNode[MAXNODE]{nullptr};
int Graph::consumerNode[MAXCONSUMER]{0};

void Graph::init(char * topo[]) {
    getBaseInfo(topo);
    netNodeBuild(topo);
    gNetBuild(topo);
    setGNetArcId();
#ifdef WEDEBUG
    printGNet();
    printNetNode();
    printConsumerNode();
#endif
}
void Graph::getBaseInfo(char * topo[]){
    if(topo[0]==NULL){
        printf("%s\n","input error! please check!" );
        exit(1);
    }
    char *result;
    int info[3];
    result=strtok(topo[0]," ");
    int i=0;
    while(result!=NULL){
        info[i]=atoi(result);
        result=strtok(NULL," ");
        ++i;
    }
    nodeCount=info[0];
    arcCount=info[1];
    consumerCount=info[2];
    serverFee=atoi(topo[2]);
    //测试打印
    printf(splitLine);
    printf("basic info: network node count, Arc count, consumer count\n");
    printf("basic info: %d,%d,%d,%.2f\n",nodeCount,arcCount,consumerCount,serverFee);

}
void Graph::gNetBuild(char * topo[]) {
    int i;
    char *r;
    for(i=4; i<4+arcCount; ++i){
        int tmp[4]={0};
        int k=0;
        r=strtok(topo[i]," ");
        while(r!=NULL){
            tmp[k]=atoi(r);
            r=strtok(NULL," ");
            ++k;
        }
        Arc *p=(Arc *)calloc(1,sizeof(Arc));
        p->capacity=(double)tmp[2];
        p->cost=(double)tmp[3];
        gNet[tmp[0] ][tmp[1] ]=p;
        //赋值另一半
        gNet[tmp[1] ][tmp[0] ]=p;

        //build twice of edge,and use the matrix edge p
        /**adjoin table,head join*/
        p->node0=tmp[0];
        p->node1=tmp[1];
        p->next=netNode[tmp[0] ]->arc;
        netNode[tmp[0]]->arc=p;


        Arc *q=(Arc *)calloc(1,sizeof(Arc));
        memcpy(q,p,sizeof(*p));
        q->node1=tmp[0];
        q->node0=tmp[1];
        /**adjoin table,head join*/
        q->next = netNode[tmp[1]]->arc;
        netNode[tmp[1]]->arc=q;
    }
}
void Graph::netNodeBuild(char **topo) {
    for(int i=0;i<nodeCount;++i){
        netNode[i]=(Node *)calloc(1,sizeof(Node));
    }
    char *r;
    for(int i=5+arcCount; i<5+arcCount+consumerCount;++i){
        int tmp[3]={0};
        int k=0;
        r=strtok(topo[i]," ");
        while(r!=NULL){
            tmp[k]=atoi(r);
            r=strtok(NULL," ");
            ++k;
        }
        netNode[tmp[1]]->require=(double)tmp[2];
        netNode[tmp[1]]->isWithConsumer=true;
        netNode[tmp[1]]->consumerId=tmp[0];

        consumerNode[tmp[0]]=tmp[1];
    }
}
void Graph::printGNet(){
    printf("gNet printf:%s",splitLine);
    for(int i=0;i<nodeCount;++i){
        for(int j=i;j<nodeCount;++j){
            if(gNet[i][j]!= nullptr){
                printf("%d:%d->%d,%.2f,%.2f\n",
                       gNet[i][j]->id,
                       gNet[i][j]->node0,
                       gNet[i][j]->node1,
                       gNet[i][j]->capacity,
                       gNet[i][j]->cost);
            }
        }
    }
}
void Graph::printNetNode() {
    printf("printNetNode:%s",splitLine);
    for(int i=0;i<nodeCount;++i){
        Arc *p=netNode[i]->arc;
        printf("the Node %d connect the nodes is: ",i);
        while(p!= nullptr){
            printf("%d ",p->node1);
            p=p->next;
        }
        printf("\n");
    }
}
void Graph::printConsumerNode(){
    printf("printConsumerNode:%s",splitLine);
    for(int i=0;i<consumerCount;++i){
        printf("%d->%d,%.2f\n",i,consumerNode[i],netNode[consumerNode[i]]->require);
    }
}
void Graph::setGNetArcId(){
    for(int i=0,k=0; i<nodeCount; ++i){
        //注意由于给的无向图，所以一半就行
        for(int j=i; j<nodeCount; ++j){
            if(gNet[i][j]!=NULL){
                gNet[i][j]->id=k;
                ++k;
            }
        }
    }
}

#endif //CDN_GRAPH_H
