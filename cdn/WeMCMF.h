/*
 * Copyright 2017
 *
 * WeMCMF.h
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
 * $Id: WeMCMF.h,v 0.1 2017/03/26 00:30:35 lw Exp $
 *
 */

#ifndef CDN_WEMCMF_H
#define CDN_WEMCMF_H


struct WeMCMF {
    //兼容
    double minicost;

    double mCost[MAXNODE];
    int pre[MAXNODE];

    std::bitset<MAXNODE> isInQue;
    std::deque<Arc*> que;
    std::deque<int> sRoute;//在寻找时运和
    std::deque<int> fRoute;
    double minCost;
    double sumFlow;//use for check
    int s;//super source
    int t;//super sink
    int sa;//super add source
    char result[MAX_OUT_CHAR_NUM];

    void addSource(int superId,std::vector<int> &server);
    void addSink();
    bool spfa();

    double run(int num1,int num2,std::vector<int> &serverLocation);
    void addServer(std::vector<int> &server);
    bool addSpfa();
    void delServer(std::vector<int> &server);
    void delSpfa();
    void freeSa();
    char* getRoute();
    void deepSearch();
    bool DFS(Arc *p);
    void printVector(std::vector<int> &serverLocation);
    //每次要重置一下数据
    void clearData();

    WeMCMF():minCost{0},sumFlow{0},s{1000},t{1001},sa{1002} {

        addSink();
    }
};
//void WeMCMF::freeSa() {
//    if(Graph::netNode[sa]){
//        Arc *p=Graph::netNode[sa]->arc;
//        Arc *q;
//        while(p!= nullptr){
//            q=p->next;
//            free(p);
//            p=q;
//        }
//    }
//    free(Graph::netNode[sa]);
//}
void WeMCMF::printVector(std::vector<int> &serverLocation) {
    printf(splitLine);
    for(int i=0;i<serverLocation.size();++i){
        printf("%d ," ,serverLocation[i]);
    }
    printf("\n%s",splitLine);
}
void WeMCMF::clearData() {
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
char * WeMCMF::getRoute() {
    //双向队列模拟一个栈，输出则从头即可
    Arc *p=Graph::netNode[s]->arc;
    //sRoute.push_back(s);
    int routeArr[MAXNODE];
    int offset{0};
    int pathNum=0;
    char re[MAX_OUT_CHAR_NUM];
    while(p!= nullptr&&DFS(p)){
        //printf(splitLine);
        //printf("times: %d\n",++i);
        sRoute.clear();
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
        for(int i=0;i<n-1;++i){
            Graph::gNet[routeArr[i+1] ][routeArr[i] ]->rCapacity=Graph::gNet[routeArr[i+1] ][routeArr[i] ]->rCapacity - minFTmp;
        }
        //倒数第二位上记录消费节点
        routeArr[n]=Graph::netNode[routeArr[n-1]]->consumerId;
        //最后一位上记录流量
        if(minFTmp>Graph::netNode[routeArr[n-1]]->require){
            //printf("consuemer: %.f\n",Graph::netNode[routeArr[n-1]]->require);
            minFTmp=Graph::netNode[routeArr[n-1]]->require;
        }
        routeArr[n+1]=minFTmp;

        //printf(splitLine);
        //printf("route:\n");
        for(int i=0;i<n+2;++i){
            offset+=sprintf(re+offset,"%d ",routeArr[i]);
            //printf("%d , ",routeArr[i]);
        }
        re[offset-1]='\n';
        ++pathNum;
        //offset+=sprintf(result+offset,"\n");
        //printf("\n");
        fRoute.clear();
        p=p->next;
        //printf("p->next:%d, rcap:%.f\n",p->next->node1,p->rCapacity);
    }
    char rNumLine[10];
    sprintf(rNumLine,"%d\n\n",pathNum);
    sprintf(result,"%s%s",rNumLine,re);
    //printf("%s",result);
    //printf("minCost: %.f\n",minCost);
    return result;

}
bool WeMCMF::DFS(Arc *p) {
    //printf("0-enter DFS \n");
    bool flag{false};
    sRoute.push_back(p->node1);
    if(Graph::gNet[p->node1][p->node0]->rCapacity>0 && Graph::netNode[p->node1]->isWithConsumer){
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

void WeMCMF::addServer(std::vector<int> &server) {
    //freeSa();
    //第一次建sa
    addSource(sa,server);
    //Arc *p=Graph::netNode[sa]->arc;
    isInQue.set(sa);
    que.push_back(Graph::netNode[sa]->arc);
    while(spfa()){

    }


    //建立超源，在原来超源的基础上增加连向服务器节点的边即可
//    for (int i = 0; i < server.size(); ++i) {
//        Arc *p=(Arc *)calloc(1,sizeof(Arc));
//        p->node0=s;
//        p->node1=server[i];
//        p->capacity=INF;
//        p->mCapacity=INF;
//        p->cost=0;
//        q->next=p;
//        q=q->next;
//        Graph::gNet[p->node0][p->node1]=p;
//
//        que.push_back(Graph::netNode[server[i] ]->arc);
//        isInQue.set(server[i] );
//    }
    //更新mCost和pre,把mCost和pre置为

}
bool WeMCMF::addSpfa() {
    Arc *p,*q;
    int dir=1;
    //int sum=mCost[s];
    while(!que.empty()){
        p=que.front();
        que.pop_front();

        //printf("-------------------------------enter spfa while pullptr %d->%d :%.f,  %.f,  %.f, %d\n",p->node0,p->node1,p->cost,mCost[p->node0],mCost[p->node1], dir);

        int node0=p->node0;
        while(p!= nullptr){
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
    return pre[t]!=-1;
}

double WeMCMF::run(int num1,int num2,std::vector<int> &serverLocation) {
    //printVector(serverLocation);
    clearData();
    addSource(s,serverLocation);
    memset(mCost,0x70, sizeof(mCost));
    memset(pre,-1,sizeof(pre));
    double minFlow;
    int minFlowLoc;
    //int u=s;
    mCost[s]=0;
    pre[s]=s;
    isInQue.set(s);
    que.push_back(Graph::netNode[s]->arc);
    //int tt;
    while(spfa()){
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
    //printf("minCost: %.f\n",minCost);
    return minCost;
}
bool WeMCMF::spfa() {
//    memset(mCost,0x70, sizeof(mCost));
//    memset(pre,-1,sizeof(pre));
//    mCost[u]=0;
//    pre[u]=s;
//    isInQue.set(u);
//    que.push_back(Graph::netNode[u]->arc);
    Arc *p,*q;
    int dir=1;
    //int sum=mCost[s];
    int tt;
    while(!que.empty()){
        //printf("spfaCycle: %d\n",++tt);
        p=que.front();
        que.pop_front();
//        if(mCost[p->node0]*que.size()>sum){
//            que.push_back(p);
//            continue;
//        }
//        sum=sum-mCost[p->node0];
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
    return pre[t]!=-1;
}
void WeMCMF::addSource(int superId,std::vector<int> &server) {
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
        p->rCapacity=INF;
        p->cost=0;
        q->next=p;
        q=q->next;
        Graph::gNet[p->node0][p->node1]=p;
        Graph::gNet[p->node1][p->node0]=p;
    }
    Graph::netNode[superId]->arc=tmp.next;

}
void WeMCMF::addSink() {
    Graph::netNode[t]=(Node *)calloc(1, sizeof(Node));//superSink,outDegree=0;
//    Arc *q;
//    Arc tmp;
//    q=&tmp;

//    Graph::netNode[t]->arc=tmp.next;
    for(int i=0;i<Graph::consumerCount;++i){
        Arc *p=(Arc *)calloc(1,sizeof(Arc));
        p->node0=Graph::consumerNode[i];
        p->node1=t;
        p->capacity=Graph::netNode[Graph::consumerNode[i]]->require;
        p->mCapacity=Graph::netNode[Graph::consumerNode[i]]->require;

        //p->rCapacity=Graph::netNode[Graph::consumerNode[i]]->require;
        p->cost=0;

        p->next=Graph::netNode[Graph::consumerNode[i]]->arc;
        Graph::netNode[Graph::consumerNode[i]]->arc=p;
        Graph::gNet[p->node0][p->node1]=p;
        Graph::gNet[p->node1][p->node0]=p;
    }
}


#endif //CDN_WEMCMF_H
