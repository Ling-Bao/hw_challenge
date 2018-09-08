/*
 * Copyright 2017
 *
 * chooseServer.h
 *
 * 初始服务器选择
 *
 * Date: March 26, 2017
 * Author: Wei Liu, Ling Bao, Shangyue Zhou
 *
 * $Id: chooseServer.h,v 0.1 2017/03/26 00:22:35 lw Exp $
 *
 */

#ifndef CDN_CHOOSESERVER_H
#define CDN_CHOOSESERVER_H

struct serverOrder{
    int id;
    double val;
    bool operator < (const serverOrder &d) const{
        return val<d.val;//默认为大顶堆
    }
    serverOrder(int i, double v):id{i},val{v} {}
};

class ChooseServer {
private:

public:
    static double minCost;
    static double minCostCan;
    static double minCostPos;
    static std::vector<int> serverID;
    static std::vector<int> serverCandidate;
    static std::vector<int> serverPossible;
    static std::priority_queue<serverOrder> pq;

    static void lpChoose();
    static void testlwlp();
    static void joinArr(double c[], int sc,double a[],int sa, double b[], int sb);
    static void printArr(double a[],int m);
    static void printVector(std::vector<int> v);
    static void printServerInfo();
};


std::priority_queue<serverOrder> ChooseServer::pq;
double ChooseServer::minCost{0};
double ChooseServer::minCostCan{0};
double ChooseServer::minCostPos{0};
std::vector<int> ChooseServer::serverID;
std::vector<int> ChooseServer::serverCandidate;
std::vector<int> ChooseServer::serverPossible;

void ChooseServer::printServerInfo() {
    printf(splitLine);
    printf("serverID :");
    ChooseServer::printVector(ChooseServer::serverID);//打印服务器节点
    printf("candidate serverID :");
    ChooseServer::printVector(ChooseServer::serverCandidate);//打印备选服务器节点
    printf("Possible serverID :");
    ChooseServer::printVector(ChooseServer::serverPossible);//打印可能服务器节点
    printf(splitLine);
}
void ChooseServer::testlwlp() {
    std::vector<std::vector<double >> matrix;
    //case1
//    std::vector<double> m1={1,0,0,1,2,8};
//    std::vector<double> m2={0,1,0,4,0,16};
//    std::vector<double> m3={0,0,1,0,4,12};
//
//    matrix.push_back(m1);
//    matrix.push_back(m2);
//    matrix.push_back(m3);
//    double myoj[]={0,0,0,-2,-3};
//    std::vector<double> oj(&myoj[0],&myoj[5]);

    //case2
    std::vector<double> m1={1,0,0,0,2,1,3,15};
    std::vector<double> m2={0,1,0,0,1,1,1,12};
    std::vector<double> m3={0,0,1,0,0,1,-3,3};
    std::vector<double> m4={0,0,0,1,2,1,0,9};


    matrix.push_back(m1);
    matrix.push_back(m2);
    matrix.push_back(m3);
    matrix.push_back(m4);
    double myoj[]={0,0,0,0,-4,-3,-5};
    std::vector<double> oj(&myoj[0],&myoj[7]);

    LinearRe re=LinearRe(&matrix,&oj);
    re.run();
}
void ChooseServer::lpChoose() {
    std::vector<std::vector<double> > matrix;
    std::vector<double> row;
    int varN=Graph::nodeCount+2*Graph::arcCount;
    /**约束条件开始**********/
    /**网络节点流量差为0约束开始***/
    double b[varN+1];
    //结点i连接网络节点,注意超源节点会流向它
    for(int i=0;i<Graph::nodeCount;++i){
        memset(b,0,sizeof(b));
        //边编号从0开始
        //注意这里没用j=i因为是要对每个节点的流入流出分析，而提供约束，由于上三角和下三角指的结点是一样的，所以都有n
        for(int j=0;j<Graph::nodeCount;++j){
            if(Graph::gNet[i][j]!= nullptr){
                //访问时索引不用加1
                b[Graph::nodeCount+Graph::gNet[i][j]->id ]=1.0/MAX_VALUE_SUPER_OUT;//正向->反向
                b[Graph::nodeCount+Graph::arcCount+Graph::gNet[i][j]->id ]=-1.0/MAX_VALUE_SUPER_OUT;//反向->正向
            }
        }
        b[i]=1.0;
        b[varN]=(-1.0)*Graph::netNode[i]->require/MAX_VALUE_SUPER_OUT;
        std::vector<double> cons(&b[0],&b[varN+1]);
        matrix.push_back(cons);
        //连接消费节点,由于节点索引从0开始，所以+1
        //printArr(b,varN);
        //printf("netNode[1][i]=%d\n", (-1)*netNode[1][i]);
    }
    //printf("netword flow 0 constraint set ok\n");
    /**网络节点流量差为0约束结束***/

    /**目标函数开始************/
    double myoj[varN];
    memset(myoj,0,sizeof(myoj));
    int i,j,k;
    //vx顶点变量系数矩阵，ex边变量系数矩阵
    double vx[Graph::nodeCount], ex[Graph::arcCount];
    //vx
    for(i=0; i<=Graph::nodeCount; ++i){
        vx[i]=Graph::serverFee;
    }
    //ex
    for(i=0,k=0; i<Graph::nodeCount; ++i){
        for(j=i; j<Graph::nodeCount; ++j){
            if(Graph::gNet[i][j]!=NULL){
                ex[k]=Graph::gNet[i][j]->cost;
                ++k;
            }
        }
    }
    //拼接数组
    joinArr(myoj,varN,vx,Graph::nodeCount,ex,Graph::arcCount);
    //printArr(myoj,varN);
    std::vector<double> oj(&myoj[0],&myoj[varN]);
    //printf("object function set ok \n");
    /**目标函数结束**********/
    LinearRe re=LinearRe(&matrix,&oj);
    re.run();
    //LinearProgrammingResult result=linearPSimplexM(matrix,oj);
    //result.print();
    int sn{0},scn{0},spn{0};
    for(int i=0;i<Graph::nodeCount;++i){
        //浮点!=0判断，如果大于MIN_VALUE则变量值不为0，记为服务器位置，记录服务器编号
        if( -matrix[i][varN]>= MIN_VALUE ){
            ++sn;
            //serverID.push_back(i);//网络节点从零开始编号，在线性规划中变量从1开始，而前info[0]个为网络节点
        }
        else if(-matrix[i][varN] >= MIN_VALUE_ZERO ){
            ++scn;
            //serverCandidate.push_back(i);//网络节点从零开始编号，在线性规划中变量从1开始，而前info[0]个为网络节点
        }
        else if(-matrix[i][varN] >= MIN_VALUE_ZERO_DOWN){
            ++spn;
            //serverPossible.push_back(i);
        }
        if(-matrix[i][varN] >= MIN_VALUE_ZERO_DOWN){
            serverOrder so{i,-matrix[i][varN]};
            pq.push(so);
        }

    }

    for (int i = 0; i < sn; ++i) {
        serverID.push_back(pq.top().id);
        //std::cout<<pq.top().val<<std::endl;
        pq.pop();
    }
    for(int i=0;i<scn;++i){
        serverCandidate.push_back(pq.top().id);
        pq.pop();
    }
    for(int i=0;i<spn;++i){
        serverPossible.push_back(pq.top().id);
        pq.pop();
    }
}
void ChooseServer::joinArr(double c[], int sc,double a[5],int sa, double b[], int sb){
    int i,j;
    //形参不能计算数组大小，因为是指针
    for(i=0,j=0;i<sa;++i,++j){
        c[j]=a[i];
    }
    for(i=0;i<sb;++i,++j){
        c[j]=b[i];
    }
    for(i=0;i<sb;++i,++j){
        c[j]=b[i];
    }
    /*for(i=0;i<sc;++i){
        printf("%.0f, ",c[i]);
    }
    printf("看看sc=%d,sa=%d,sb=%d\n",sc,sa,sb);
    */
}
void ChooseServer::printArr(double a[],int m){
    int i;
    for(i=0;i<m;++i){
        printf("%.5f ",a[i] );
    }
    printf("\n");
}

void ChooseServer::printVector(std::vector<int> v){
    printf("count %d, ID as follows\n",(signed)v.size());
    for(int i=0;i<(signed)v.size();++i){
        printf("%d, ",v[i]);
    }
    printf("\n");
}

#endif //CDN_CHOOSESERVER_H