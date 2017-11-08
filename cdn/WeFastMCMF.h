/*
 * Copyright 2017
 *
 * WeFastMCMF.h
 *
 * 最小费用，最大流
 *
 * Date: March 26, 2017
 * Author: Wei Liu, Ling Bao, Shangyue Zhou
 *
 * 思想：
 *
 * 在计算最小费用最大流时，要注意设置反向边，否则，费用会偏大
 *
 * 这里用两条有向边，包含两个容量，一个正向一个逆向
 *
 * c(a,b)=f(a,b)+f(b,a)
 *
 * 注意其中细节
 *
 * 优化，SLF,LLL
 *
 * 每次spfa在以前基础上计算
 *
 * 删除节点所在服务器，也即节点由供给变成需要了，在残余图中补充他所需要的就行
 *
 * 增加节点的服务器，也就是服务器流量流向服务器问题
 * 若服务器距离为负则花费减少加上其服务器费用为负则花费减少，否则花费增加
 * 建立另一超源，由新服务器组，流向原来的服务器
 *
 * 在增加时，只能利用残余图信息利用不了距离信息
 *
 * 流算法，分为两部分
 * 一个是残余图
 * 一个为最短路
 *
 * 超源：1000，1001
 * 增加服务器超源，1002
 * 删除服务器超源，1003
 *
 * 操作：
 * 增加服务器：建节点，建边
 * 删除服务器：建节点，转换边
 *
 * 由于用单向表示双向，在运算中只用到cost和capacity
 * 所以统一下
 * node0表示出发点
 * node1表示到达点
 *
 * $Id: WeFastMCMF.h,v 0.1 2017/03/26 00:30:35 lw Exp $
 *
 */

#ifndef CDN_WEFASTMCMF_H
#define CDN_WEFASTMCMF_H


struct WeFastMCMF {
    //兼容
    double minicost;

    static double mCost[MAXNODE];
    int pre[MAXNODE];

    std::bitset<MAXNODE> isInQue;
    std::deque<Arc*> que;
    std::deque<int> sRoute;//在寻找时运和
    std::deque<int> fRoute;
    std::vector<int> *lserver;
    std::vector<int> nowServer;
    double minCost;
    double sumFlow;//use for check
    int s;//super source
    int t;//super sink
    int sa;//super add source
    int td;//super delete sink
    char result[MAX_OUT_CHAR_NUM];
    //增加超源：指定超源，和服务器，增加超源到服务器的节点的连接，建点、建边
    void addSource(int superId,std::vector<int> &server);
    //增加删除的超汇：指定超汇，和服务器，增加服务器到超汇的连接，建点，转换边
    void addDeleteSink(std::vector<int> &server);
    //转换超源到超汇
    void SourceToSink();
    //增加服务器运算
    double addServer(std::vector<int> &server);
    double addServer1(std::vector<int> &server);
    double deleteServer(std::vector<int> &server);
    //增加服务器spfa(),有点不一样，就是用的是rcapacity,而不是mcapacity.好像是一样的
    bool addSpfa(int end);
    //新的超源合并到原有超源；
    void combineSource();
    void bestRe();

    void addSink();
    bool spfa(int end);

    double run(int num1,int num2,std::vector<int> &serverLocation);

    char* getRoute();
    void deepSearch();
    bool DFS(Arc *p);
    void printVector(std::vector<int> &serverLocation);
    //每次要重置一下数据
    void clearData();

    WeFastMCMF():minCost{0},sumFlow{0},s{1000},t{1001},sa{1002},td{1003} {

        addSink();
    }
};
double WeFastMCMF::mCost[MAXNODE];
void WeFastMCMF::bestRe() {
    Best best;
    nowServer=best.getBest(nowServer);
    run(0,0,nowServer);
}
double WeFastMCMF::deleteServer(std::vector<int> &server) {
    memset(mCost,0x70, sizeof(mCost));
    memset(pre,-1,sizeof(pre));
    isInQue.reset();
    addDeleteSink(server);
    que.push_back(Graph::netNode[s]->arc);
    mCost[s]=0;
    pre[s]=s;
    isInQue.set(s);
    int minFlowLoc{-1};
    double minFlow;
    int delCost{0};
    //printf("pre[td]=%d\n",pre[td]);
    while(spfa(td)){
        //printf("pre[td]=%d\n",pre[td]);
        minFlow=INF;
        for(int i=td;i!=s;i=pre[i]){
            if(Graph::gNet[pre[i] ][i]->rCapacity > 0){
                if(minFlow >= Graph::gNet[pre[i] ][i]->rCapacity){
                    minFlow = Graph::gNet[pre[i] ][i]->rCapacity;
                    minFlowLoc=pre[i];
                }
            }
            else{
                if(minFlow >= Graph::gNet[pre[i] ][i]->mCapacity ){
                    minFlow = Graph::gNet[pre[i] ][i]->mCapacity;
                    minFlowLoc=pre[i];
                }
            }
        }
        for(int i=td;i!=s;i=pre[i]){
            //printf("minFlow= %.f, capacity= %.f\n",minFlow,Graph::gNet[pre[i] ][i]->rCapacity);
            if(Graph::gNet[pre[i] ][i]->rCapacity > 0){
                Graph::gNet[pre[i] ][i]->rCapacity=Graph::gNet[pre[i] ][i]->rCapacity - minFlow;
                delCost=delCost - minFlow*Graph::gNet[pre[i] ][i]->cost;

                if(i!=td&&pre[i]!=s){
                    Graph::gNet[i ][pre[i]]->mCapacity=Graph::gNet[i ][pre[i]]->capacity - Graph::gNet[pre[i] ][i]->rCapacity;

                }
                //printf("-1 %d -> %d ,minFlow= %.f, capacity= %.f\n",pre[i],i,minFlow,Graph::gNet[pre[i] ][i]->rCapacity);
            }
            else{
                Graph::gNet[pre[i] ][i]->mCapacity = Graph::gNet[pre[i] ][i]->mCapacity -minFlow;
                delCost=delCost + minFlow*Graph::gNet[pre[i] ][i]->cost;
                if(i!=td && pre[i]!=s){
                    Graph::gNet[i ][pre[i]]->rCapacity=Graph::gNet[i ][pre[i]]->capacity - Graph::gNet[pre[i] ][i]->mCapacity;
                }
                //printf("+1 %d -> %d ,minFlow= %.f, capacity= %.f\n",pre[i],i,minFlow,Graph::gNet[pre[i] ][i]->mCapacity);
            }
        }

        double CTmp=INF;
        int CLoc=-1;
        //当以前的服务器没有形成有效流量时，minFlowLoc会为-1;
        //一般不会出现此情况

        std::queue<int> vQue;
        vQue.push(minFlowLoc);
        int vt;
        while(!vQue.empty()){
            vt=vQue.front();
            vQue.pop();
            Arc *p=Graph::netNode[vt]->arc;
            while(p!= nullptr){
                if(pre[p->node1] == vt){
                    pre[p->node1] = -1;
                    mCost[p->node1] = INF;
                    vQue.push(p->node1);
                    //++cc;
                    //把充满的节点，的后继节点的相邻的节点，为不INF的推入队列
                    Arc *q=Graph::netNode[p->node1]->arc;
                    while(q!= nullptr){
                        if(pre[q->node1]!=-1 && !isInQue[q->node1]){
                            if(!que.empty()){
                                if(mCost[q->node1]<mCost[que.front()->node0]){
                                    que.push_front(Graph::netNode[q->node1]->arc);
                                } else{
                                    que.push_back(Graph::netNode[q->node1]->arc);
                                }
                            } else{
                                que.push_back(Graph::netNode[q->node1]->arc);
                            }

                            //que.push_back(Graph::netNode[q->node1]->arc);
                            isInQue.set(q->node1);

                            //printf("q->node1: %d\n",q->node1);
                        }
                        q=q->next;
                    }

                }
                p=p->next;
            }
        }
        //推入一个离消费点最近的有值节点
        CTmp=INF;
        CLoc=-1;
        for(int i=0;i<server.size();++i){
            Arc *p=Graph::netNode[server[i]]->arc;
            if(p->mCapacity>0 && !isInQue[p->node0]){
                if(CTmp>mCost[p->node0]+p->cost){
                    CTmp=mCost[p->node0]+p->cost;
                    CLoc=p->node0;
                }
            }
        }
        if(CLoc!=-1){
            que.push_back(Graph::netNode[CLoc]->arc);
            isInQue.set(CLoc);
            CLoc=-1;
            CTmp=INF;
        }
    }

    delCost=delCost-Graph::serverFee*server.size();
    minCost=minCost+delCost;
    minicost=minCost;
    printf("minCost: %.f\n",minCost);
    return minCost;
}
void WeFastMCMF::SourceToSink() {
    //超源转为汇点
    // 方向node0,node1要变
    // 反向流量要更正，因为超源没有更新反向边
    //超汇的mcapcity为0;
    int sourceId=s;
    Arc* p=Graph::netNode[sourceId]->arc;
    Graph::netNode[sourceId]->arc= nullptr;
    Arc* q;
    while(p!= nullptr){
        //头插法
        int t;
        q=p->next;

        t=p->node0;//交换头和尾
        p->node0=p->node1;
        p->node1=t;

        p->mCapacity=p->capacity - p->mCapacity;
        p->rCapacity=0;

        p->next=Graph::netNode[p->node0]->arc;
        //由于是无向图所以p->next必定不为空,但是这里是p->node1,即可能连接到超源超汇，所以要判断
        if(p->next!= nullptr){
            p->next->pre=p;
        }
        p->pre=(Arc *)1;
        Graph::netNode[p->node0]->arc=p;
        printf("%d ",p->node0);

        p=q;
    }
    printf("\n");
    Arc *x=Graph::netNode[14]->arc;
    while(x!= nullptr){
        printf("%d -> %d\n",x->node0,x->node1);
        x=x->next;
    }

}
void WeFastMCMF::addDeleteSink(std::vector<int> &server) {
    //变更邻接表和邻接数组的内容
    //1、取消原来超源与其的链接
    //2、将其链到sinkId上，更改node0,node1,为serverid和sinkId
    //3、更改rcap=cap-map
    //4、mcap=0
    int sinkId=td;
    Graph::netNode[sinkId]=(Node *)calloc(1, sizeof(Node));//superSink,outDegree=0;
    Node* d=Graph::netNode[s];
    Arc fresh;
    Arc* tmp=&fresh;
    Arc* p,q;

    for(int i=0;i<server.size();++i){
        //取消超源

        p=Graph::gNet[s][server[i] ];
        if((long)p->pre==1){
            d->arc=p->next;
            p->next->pre=(Arc *)1;
        }
        else if(p->next!= nullptr){
            p->pre->next=p->next;
            p->next->pre=p->pre;
        }
        else{
            p->pre->next=p->next;
        }

        //连接为超汇,即给删除服务器增加出边
        //p=Graph::gNet[s][server[i]];

        Graph::gNet[sinkId][server[i]]= p;//在输出时需要
        Graph::gNet[server[i]][sinkId]=p;
        p->node0=server[i];
        p->node1=sinkId;
        p->mCapacity=p->capacity - p->mCapacity;
        p->rCapacity=0;

        //头插法
        p->next=Graph::netNode[server[i]]->arc;
        //网络节点都是有边的，不像超源没有入边，超汇没有出边
        p->next->pre=p;
        p->pre=(Arc *)1;
        Graph::netNode[server[i]]->arc=p;
    }
}
void WeFastMCMF::printVector(std::vector<int> &serverLocation) {
    printf(splitLine);
    for(int i=0;i<serverLocation.size();++i){
        printf("%d ," ,serverLocation[i]);
    }
    printf("\n%s",splitLine);
}
void WeFastMCMF::clearData() {
    //1、图重置
    for(int i=0;i<Graph::nodeCount;++i){
        Arc *p=Graph::netNode[i]->arc;
        while(p!= nullptr){
            p->mCapacity=p->capacity;
            p->rCapacity=0;
            p=p->next;
        }
    }
    //2、mCost,pre
    memset(mCost,0x70, sizeof(mCost));
    memset(pre,-1,sizeof(pre));
    isInQue.reset();
    que.clear();
    sRoute.clear();
    fRoute.clear();
    minCost=0;
    minicost=0;
    sumFlow=0;
    //3、释放超源
    if(Graph::netNode[s]!= nullptr){
        Arc *q=Graph::netNode[s]->arc;
        Arc *f;
        while(q!= nullptr){
            Graph::gNet[s][q->node1]= nullptr;
            Graph::gNet[q->node1][s]= nullptr;
            f=q->next;
            free(q);
            q=f;
        }
        free(Graph::netNode[s]);
    }
    Graph::netNode[s]= nullptr;
}
char* WeFastMCMF::getRoute() {
    //双向队列模拟一个栈，输出则从头即可
    Arc *p=Graph::netNode[s]->arc;
    sRoute.push_back(s);
    int routeArr[MAXNODE];
    int offset{0};
    int pathNum=0;
    char re[MAX_OUT_CHAR_NUM];
    //重置反向边
    Arc *se=Graph::netNode[s]->arc;
    while(se!= nullptr){
        se->rCapacity=se->capacity-se->mCapacity;
        se=se->next;
    }
    //重置正向边
    for (int i = 0; i < Graph::consumerCount; ++i) {

        Graph::gNet[Graph::consumerNode[i]][t]->rCapacity=Graph::gNet[Graph::consumerNode[i]][t]->capacity - Graph::gNet[Graph::consumerNode[i]][t]->mCapacity;

    }
    while(p!= nullptr&&DFS(p)){

//        if(!DFS(p)){
//            p=p->next;
//        }
        //printf(splitLine);
        //printf("times: %d\n",++i);
        sRoute.clear();
        sRoute.push_back(s);
        int n=fRoute.size();
        if(n==0){
            p=p->next;
            continue;
        }
        memset(routeArr,-1,sizeof(routeArr));
        for(int i=0;i<n;++i){
            routeArr[i]=fRoute.front();
            //printf("%d ,",fRoute.front() );
            fRoute.pop_front();
        }
        double minFTmp=INF;
        //查看通过反向边最小流
        for(int i=0;i<n-1;++i){
            if(minFTmp> Graph::gNet[routeArr[i+1] ][routeArr[i] ]->rCapacity){
                minFTmp=Graph::gNet[routeArr[i+1] ][routeArr[i] ]->rCapacity;
            }
        }
        //减去
        //printf("pmCap: %.f , %.f , %.f\n",p->mCapacity,p->rCapacity,Graph::gNet[p->node1][p->node0]->rCapacity);
        for(int i=0;i<n-1;++i){
            Graph::gNet[routeArr[i+1] ][routeArr[i] ]->rCapacity=Graph::gNet[routeArr[i+1] ][routeArr[i] ]->rCapacity - minFTmp;
        }
        //倒数第二位上记录消费节点
        routeArr[n-1]=Graph::netNode[routeArr[n-2]]->consumerId;
        //最后一位上记录流量
        if(minFTmp>Graph::netNode[routeArr[n-2]]->require){
            //printf("consuemer: %.f\n",Graph::netNode[routeArr[n-1]]->require);
            minFTmp=Graph::netNode[routeArr[n-2]]->require;
        }
        routeArr[n]=minFTmp;
        //printf(splitLine);
        //printf("route:\n");
        for(int i=1;i<n+1;++i){
            offset+=sprintf(re+offset,"%d ",routeArr[i]);
            //printf("%d , ",routeArr[i]);
        }
        re[offset-1]='\n';
        ++pathNum;
        //offset+=sprintf(result+offset,"\n");
        //printf("\n");
        fRoute.clear();
        //如果流量没用完则继续找，否则换下一边
        //p->mCapacity=p->mCapacity+minFTmp;
        //printf("pmCap: %.f , %.f , %.f\n",p->mCapacity,p->rCapacity,Graph::gNet[p->node1][p->node0]->rCapacity);
        //printf("cap: %.f , %.f\n",p->rCapacity,p->mCapacity);
        if(p->rCapacity<=0){
            p=p->next;
        }
        //printf("p->next:%d, rcap:%.f\n",p->next->node1,p->rCapacity);
    }
    for (int i = 0; i < Graph::consumerCount; ++i) {

        Graph::gNet[Graph::consumerNode[i]][t]->mCapacity=Graph::gNet[Graph::consumerNode[i]][t]->capacity;

    }
    re[offset-1]='\0';
    char rNumLine[10];
    sprintf(rNumLine,"%d\n\n",pathNum);
    sprintf(result,"%s%s",rNumLine,re);
    //printf("%s",result);
    //printf("minCost: %.f\n",minCost);
    return result;
}
bool WeFastMCMF::DFS(Arc *p) {
    //printf("0-enter DFS \n");
    bool flag{false};
    sRoute.push_back(p->node1);
    //这里需要用到超汇和超源的反向边
    if(Graph::gNet[p->node1][p->node0]->rCapacity>0 && p->node1==t){
        //printf("1----enter DFS \n");
        fRoute.assign(sRoute.begin(),sRoute.end());
        //printf("2----enter DFS \n");
        return true;
    } else{
        //printf("3----enter DFS \n");
        Arc *q=Graph::netNode[p->node1]->arc;
        while(q!= nullptr){
            // printf("4----enter DFS, %.f \n",q->rCapacity);
            if(Graph::gNet[q->node1][q->node0]->rCapacity>0){
                flag=DFS(q);
                sRoute.pop_back();
            }
            q=q->next;
        }
        return flag;
    }
}
double WeFastMCMF::addServer(std::vector<int> &server) {
    memset(mCost,0x70, sizeof(mCost));
    memset(pre,-1,sizeof(pre));
    isInQue.reset();
    //第一次建sa
    addSource(sa,server);
    //源点转为汇点
    SourceToSink();
    //推入汇点
    que.push_back(Graph::netNode[sa]->arc);
    mCost[sa]=0;

    pre[sa]=sa;
    isInQue.set(sa);
    int minFlowLoc{-1};
    double minFlow;
    int addCost{0};

    while(addSpfa(s)){
        //printf("pre[td]=%d\n",pre[td]);
        minFlow=INF;
        for(int i=s;i!=sa;i=pre[i]){
            if(Graph::gNet[pre[i] ][i]->rCapacity > 0){
                if(minFlow >= Graph::gNet[pre[i] ][i]->rCapacity){
                    minFlow = Graph::gNet[pre[i] ][i]->rCapacity;
                    minFlowLoc=pre[i];
                }
            }
            else{
                if(minFlow >= Graph::gNet[pre[i] ][i]->mCapacity ){
                    minFlow = Graph::gNet[pre[i] ][i]->mCapacity;
                    minFlowLoc=pre[i];
                }
            }
        }
        for(int i=s;i!=sa;i=pre[i]){
            //printf("minFlow= %.f, capacity= %.f\n",minFlow,Graph::gNet[pre[i] ][i]->rCapacity);
            if(Graph::gNet[pre[i] ][i]->rCapacity > 0){
                Graph::gNet[pre[i] ][i]->rCapacity=Graph::gNet[pre[i] ][i]->rCapacity - minFlow;
                addCost=addCost - minFlow*Graph::gNet[pre[i] ][i]->cost;

                if(i!=s&&pre[i]!=sa){
                    Graph::gNet[i ][pre[i]]->mCapacity=Graph::gNet[i ][pre[i]]->capacity - Graph::gNet[pre[i] ][i]->rCapacity;

                }
                //printf("-1 %d -> %d ,minFlow= %.f, capacity= %.f\n",pre[i],i,minFlow,Graph::gNet[pre[i] ][i]->rCapacity);
            }
            else{
                Graph::gNet[pre[i] ][i]->mCapacity = Graph::gNet[pre[i] ][i]->mCapacity -minFlow;
                addCost=addCost + minFlow*Graph::gNet[pre[i] ][i]->cost;
                if(i!=s && pre[i]!=sa){
                    Graph::gNet[i ][pre[i]]->rCapacity=Graph::gNet[i ][pre[i]]->capacity - Graph::gNet[pre[i] ][i]->mCapacity;
                }
                //printf("+1 %d -> %d ,minFlow= %.f, capacity= %.f\n",pre[i],i,minFlow,Graph::gNet[pre[i] ][i]->mCapacity);
            }
        }

        double CTmp=INF;
        int CLoc=-1;
        //当以前的服务器没有形成有效流量时，minFlowLoc会为-1;
        //一般不会出现此情况

        std::queue<int> vQue;
        vQue.push(minFlowLoc);
        int vt;
        while(!vQue.empty()){
            vt=vQue.front();
            vQue.pop();
            Arc *p=Graph::netNode[vt]->arc;
            while(p!= nullptr){
                if(pre[p->node1] == vt){
                    pre[p->node1] = -1;
                    mCost[p->node1] = INF;
                    vQue.push(p->node1);
                    //++cc;
                    //把充满的节点，的后继节点的相邻的节点，为不INF的推入队列
                    Arc *q=Graph::netNode[p->node1]->arc;
                    while(q!= nullptr){
                        if(pre[q->node1]!=-1 && !isInQue[q->node1]){
                            if(!que.empty()){
                                if(mCost[q->node1]<mCost[que.front()->node0]){
                                    que.push_front(Graph::netNode[q->node1]->arc);
                                } else{
                                    que.push_back(Graph::netNode[q->node1]->arc);
                                }
                            } else{
                                que.push_back(Graph::netNode[q->node1]->arc);
                            }

                            //que.push_back(Graph::netNode[q->node1]->arc);
                            isInQue.set(q->node1);

                            //printf("q->node1: %d\n",q->node1);
                        }
                        q=q->next;
                    }

                }
                p=p->next;
            }
        }
        //推入一个离消费点最近的有值节点
        CTmp=INF;
        CLoc=-1;
        for(int i=0;i<nowServer.size();++i){
            Arc *p=Graph::netNode[nowServer[i]]->arc;
            if(p->mCapacity>0 && !isInQue[p->node0]){
                if(CTmp>mCost[p->node0]+p->cost){
                    CTmp=mCost[p->node0]+p->cost;
                    CLoc=p->node0;
                }
            }
        }
        if(CLoc!=-1){
            que.push_back(Graph::netNode[CLoc]->arc);
            isInQue.set(CLoc);
            CLoc=-1;
            CTmp=INF;
        }
    }

    addCost=addCost+Graph::serverFee*server.size();
    minCost=minCost+addCost;
    minicost=minCost;
    printf("minCost: %.f\n",minCost);
    return minCost;
}
double WeFastMCMF::run(int num1,int num2,std::vector<int> &serverLocation) {
    nowServer.assign(serverLocation.begin(),serverLocation.end());
    lserver=&serverLocation;
    //printVector(serverLocation);
    clearData();
    addSource(s,serverLocation);
    memset(mCost,0x70, sizeof(mCost));
    memset(pre,-1,sizeof(pre));
    double minFlow;
    int minFlowLoc{-1};
    //int u=s;
    mCost[s]=0;
    pre[s]=s;
    isInQue.set(s);
    que.push_back(Graph::netNode[s]->arc);
    //int tt;
    while(spfa(t)){
        //printf("cycle: %d\n",++tt);
        minFlow=INF;
        for(int i=t;i!=s;i=pre[i]){
            //比较改为小于等于，方便找链路最前面的那个通过量最小的
            if(Graph::gNet[pre[i] ][i]->rCapacity > 0){
                if(minFlow >= Graph::gNet[pre[i] ][i]->rCapacity){
                    minFlow = Graph::gNet[pre[i] ][i]->rCapacity;
                    minFlowLoc=pre[i];
                }
            }
            else{
                if(minFlow >= Graph::gNet[pre[i] ][i]->mCapacity ){
                    minFlow = Graph::gNet[pre[i] ][i]->mCapacity;
                    minFlowLoc=pre[i];
                }
            }

        }
        sumFlow=sumFlow+minFlow;
        for(int i=t;i!=s;i=pre[i]){
            //printf("minFlow= %.f, capacity= %.f\n",minFlow,Graph::gNet[pre[i] ][i]->rCapacity);
            if(Graph::gNet[pre[i] ][i]->rCapacity > 0){
                Graph::gNet[pre[i] ][i]->rCapacity=Graph::gNet[pre[i] ][i]->rCapacity - minFlow;
                minCost=minCost - minFlow*Graph::gNet[pre[i] ][i]->cost;

                if(i!=t&&pre[i]!=s){
                    Graph::gNet[i ][pre[i]]->mCapacity=Graph::gNet[i ][pre[i]]->capacity - Graph::gNet[pre[i] ][i]->rCapacity;

                }
                //printf("-1 %d -> %d ,minFlow= %.f, capacity= %.f\n",pre[i],i,minFlow,Graph::gNet[pre[i] ][i]->rCapacity);
            }
            else{
                Graph::gNet[pre[i] ][i]->mCapacity = Graph::gNet[pre[i] ][i]->mCapacity -minFlow;
                minCost=minCost + minFlow*Graph::gNet[pre[i] ][i]->cost;
                if(i!=t && pre[i]!=s){
                    Graph::gNet[i ][pre[i]]->rCapacity=Graph::gNet[i ][pre[i]]->capacity - Graph::gNet[pre[i] ][i]->mCapacity;
                }
                //printf("+1 %d -> %d ,minFlow= %.f, capacity= %.f\n",pre[i],i,minFlow,Graph::gNet[pre[i] ][i]->mCapacity);
            }
        }


        double CTmp=INF;
        int CLoc=-1;

        std::queue<int> vQue;
        vQue.push(minFlowLoc);
        int vt;
        while(!vQue.empty()){
            vt=vQue.front();
            vQue.pop();
            Arc *p=Graph::netNode[vt]->arc;
            while(p!= nullptr){
                if(pre[p->node1] == vt){
                    pre[p->node1] = -1;
                    mCost[p->node1] = INF;
                    vQue.push(p->node1);
                    //++cc;
                    //把充满的节点，的后继节点的相邻的节点，为不INF的推入队列
                    Arc *q=Graph::netNode[p->node1]->arc;
                    while(q!= nullptr){
                        if(pre[q->node1]!=-1 && !isInQue[q->node1]){
                            if(!que.empty()){
                                if(mCost[q->node1]<mCost[que.front()->node0]){
                                    que.push_front(Graph::netNode[q->node1]->arc);
                                } else{
                                    que.push_back(Graph::netNode[q->node1]->arc);
                                }
                            } else{
                                que.push_back(Graph::netNode[q->node1]->arc);
                            }

                            //que.push_back(Graph::netNode[q->node1]->arc);
                            isInQue.set(q->node1);

                            //printf("q->node1: %d\n",q->node1);
                        }
                        q=q->next;
                    }

                }
                p=p->next;
            }
        }

        //推入一个离服务器最近的有值节点
        CTmp=INF;
        CLoc=-1;
        for(int i=0;i<Graph::consumerCount;++i){
            Arc *p=Graph::netNode[Graph::consumerNode[i] ]->arc;
            if(p->mCapacity>0 && !isInQue[p->node0]){
                if(CTmp>mCost[p->node0]+p->cost){
                    CTmp=mCost[p->node0]+p->cost;
                    CLoc=p->node0;
                }
            }
        }
        if(CLoc!=-1){
            que.push_back(Graph::netNode[CLoc]->arc);
            isInQue.set(CLoc);
            CLoc=-1;
            CTmp=INF;
        }

        //(4) best推入全部有值的服务器
//        for(int i=0;i<Graph::nodeCount;++i){
//
//            if(pre[i]!=-1 && !isInQue[i]){
//                que.push_back(Graph::netNode[i]->arc);
//                isInQue.set(i);
//                //++cc;
//            }
//        }

//        CTmp=INF;
//        CLoc=-1;
//        for(int i=0;i<Graph::consumerCount;++i){
//            Arc *p=Graph::netNode[Graph::consumerNode[i] ]->arc;
//            if(p->mCapacity>0 && !isInQue[p->node0]){
//                que.push_back(Graph::netNode[p->node0]->arc);
//                isInQue.set(p->node0);
//            }
//
//
//        }

    }
    //当流量充满时一些点不能经过，作如下处理，将该点后面受影响的值改为INF
    if(Graph::allDemand!=sumFlow){
        minCost=INF;
        minicost=INF;
        return minCost;
    }
    //然后设置一个点为起动点供spfa,该点为此次寻找链路的minFlowLoc的后继节点，的前
    minCost=minCost+Graph::serverFee*serverLocation.size();
    minicost=minCost;
    bestRe();
    //printf("minCost: %.f\n",minCost);
    return minCost;
}
bool WeFastMCMF::addSpfa(int end) {
    Arc *p,*q;
    int dir=1;
    //int sum=mCost[s];
    int tt;
    while(!que.empty()){
        //printf("spfaCycle: %d\n",++tt);
        p=que.front();
        que.pop_front();
        //printf("-------------------------------enter spfa while pullptr %d->%d :%.f,  %.f,  %.f, %d\n",p->node0,p->node1,p->cost,mCost[p->node0],mCost[p->node1], dir);
        if(p== nullptr){
            continue;
        }

        int node0=p->node0;
        while(p!= nullptr){
            //printf("--------------------------spfaCycle: %d , %d->%d, cap: %.f, mcap: %.f, rcap: %.f\n",
            //      ++tt,p->node0,p->node1,p->capacity,p->mCapacity,p->rCapacity);
            if(p->rCapacity>0){
                dir=-1;
            }else{
                dir=1;
                if(p->mCapacity<=0){
                    p=p->next;
                    continue;
                }
            }
            if(mCost[p->node1] > p->cost*dir+mCost[p->node0]){
                mCost[p->node1] = p->cost*dir+mCost[p->node0];
                if(!isInQue[p->node1]){
                    isInQue.set(p->node1);
                    if(Graph::netNode[p->node1]->arc!= nullptr){
                        if(!que.empty()){
                            //stl
                            if(mCost[p->node1]<mCost[que.front()->node0]){
                                que.push_front(Graph::netNode[p->node1]->arc);
                            }
                            else{
                                que.push_back(Graph::netNode[p->node1]->arc);
                            }
                        }
                        else{
                            que.push_back(Graph::netNode[p->node1]->arc);
                        }
                    }
                }
                //sum=sum+mCost[p->node1];
                pre[p->node1]=p->node0;
            }
            p=p->next;
        }
        isInQue.reset(node0);
    }
    return mCost[end]<0;
}
bool WeFastMCMF::spfa(int end) {
    Arc *p,*q;
    int dir=1;
    //int sum=mCost[s];
    int tt;
    while(!que.empty()){
        //printf("spfaCycle: %d\n",++tt);
        p=que.front();
        que.pop_front();
        //printf("-------------------------------enter spfa while pullptr %d->%d :%.f,  %.f,  %.f, %d\n",p->node0,p->node1,p->cost,mCost[p->node0],mCost[p->node1], dir);
        if(p== nullptr){
            continue;
        }

        int node0=p->node0;
        while(p!= nullptr){
            //printf("--------------------------spfaCycle: %d , %d->%d, cap: %.f, mcap: %.f, rcap: %.f\n",
            //      ++tt,p->node0,p->node1,p->capacity,p->mCapacity,p->rCapacity);
            if(p->rCapacity>0){
                dir=-1;
            }else{
                dir=1;
                if(p->mCapacity<=0){
                    p=p->next;
                    continue;
                }
            }
            if(mCost[p->node1] > p->cost*dir+mCost[p->node0]){
                //printf("enter spfa while pullptr %d->%d :%.f,  %.f,  %.f, %d\n",p->node0,p->node1,p->cost,mCost[p->node0],mCost[p->node1], dir);
                //if(dir==-1){
                // printf("..........................................spfaCycle: %d , %d->%d, cap: %.f, mcap: %.f, rcap: %.f,-----,%.f = %.f * %d + %.f\n",
                //        ++tt,p->node0,p->node1,p->capacity,p->mCapacity,p->rCapacity, mCost[p->node1],p->cost,dir,mCost[p->node0]);

                //}
                mCost[p->node1] = p->cost*dir+mCost[p->node0];
                if(!isInQue[p->node1]){
                    isInQue.set(p->node1);
                    if(Graph::netNode[p->node1]->arc!= nullptr){
                        if(!que.empty()){
                            //stl
                            if(mCost[p->node1]<mCost[que.front()->node0]){
                                que.push_front(Graph::netNode[p->node1]->arc);
                            }
                            else{
                                que.push_back(Graph::netNode[p->node1]->arc);
                            }
                        }
                        else{
                            que.push_back(Graph::netNode[p->node1]->arc);
                        }
                    }
                }
                //sum=sum+mCost[p->node1];
                pre[p->node1]=p->node0;
            }
            p=p->next;
        }
        isInQue.reset(node0);
    }
    return pre[end]!=-1;
}
void WeFastMCMF::addSource(int superId,std::vector<int> &server) {
    Graph::netNode[superId]=(Node *)calloc(1, sizeof(Node));//superSource,outDegree=server.size();
    Arc *q;
    Arc tmp;
    q=&tmp;
    for (int i = 0; i < server.size(); ++i) {
        Arc *p=(Arc *)calloc(1,sizeof(Arc));
        p->node0=superId;
        p->node1=server[i];
        p->capacity=INF;
        p->mCapacity=INF;
        p->rCapacity=0;
        p->cost=0;
        //尾插法
        p->pre=q;
        q->next=p;
        q=q->next;
        Graph::gNet[p->node0][p->node1]=p;
        Graph::gNet[p->node1][p->node0]=p;
    }
    Graph::netNode[superId]->arc=tmp.next;
    Graph::netNode[superId]->arc->pre=(Arc *)1;

}
void WeFastMCMF::addSink() {
    Graph::netNode[t]=(Node *)calloc(1, sizeof(Node));//superSink,outDegree=0;
    for(int i=0;i<Graph::consumerCount;++i){
        Arc *p=(Arc *)calloc(1,sizeof(Arc));
        p->node0=Graph::consumerNode[i];
        p->node1=t;
        p->capacity=Graph::netNode[Graph::consumerNode[i]]->require;
        p->mCapacity=Graph::netNode[Graph::consumerNode[i]]->require;
        p->rCapacity=0;
        p->cost=0;
        //头插法
        p->next=Graph::netNode[Graph::consumerNode[i]]->arc;
        if(p->next!= nullptr){
            p->next->pre=p;
        }
        p->pre=(Arc *)1;
        Graph::netNode[Graph::consumerNode[i]]->arc=p;
        Graph::gNet[p->node0][p->node1]=p;
        Graph::gNet[p->node1][p->node0]=p;
        //printf("p.rcap:%.f   ",Graph::gNet[p->node0][p->node1]->rCapacity);
    }
}
#endif //CDN_WEFASTMCMF_H