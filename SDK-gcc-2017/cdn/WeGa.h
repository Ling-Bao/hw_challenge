/*
 * Copyright 2017
 *
 * ga.h
 *
 * 遗传算法优化初始解
 *
 * 算法思路：
 * 每组服务器为一个个体
 * 第一阶段：增大个体差异
 * 第二阶段：选择优秀基因传承，好个体关联分析
 * 第三阶段：个体变异进化
 *
 * 初始服务器：ServerID,ServerCandidate,ServerPossible
 *
 * 少一个有服务器情景，少两个服务器情景，结果最好的前n个，抽取共同基因片段，做传承基因
 * 如果，少了任何一个服务器价格高于LP价，则尝试增加服务器个数，ln(allServer)
 *
 * Date: March 26, 2017
 * Author: Wei Liu, Ling Bao, Shangyue Zhou
 *
 * $Id: ga.h,v 0.1 2017/03/26 00:30:30 bl Exp $
 *
 */

#ifndef CDN_WEGA_H
#define CDN_WEGA_H


class WeGa {
private:
    char * thefilename;
    std::vector<int> allServer;
    std::bitset<MAXCONSUMER> BESTServer;//个体基因，标记所选择的服务器
    std::bitset<MAXCONSUMER> LPBestServer;//保存最好的基因，即选择服务器的最好结果
    std::bitset<MAXCONSUMER> BestGene;//最好的基因片段，即选出来的最好的需要传承的基因
    std::vector<std::bitset<MAXCONSUMER>> gaPopulation;//种群，从种群中选择好的基因
//    int initPopulationSize;
//    int populationSize;//种群大小
//    int maxIterate;
    int serverCountRange;//计算服务器要在ServerID上变动的个数，为ln(allserver)
    double minCost;
    int calcDirect;//计算方向，由预处理决定
public:
    std::vector<int> GServerID;//存放ga选择的服务器节点,经解码过后
private:
    void preDeal();//预处理，判断进行路线
    void reduceOneServer();//在LPserverID中减少一个服务器
    void addOneServer();//增加一个服务器

//    void stageTwoGeneP();//生成种群第二阶段，增加服务器数

//    void chooseBestGene();//精英群体
//    void generatePopulation();//生成种群
//    void decode();//基因解码，即对应真实服务器ID
    void initial();//初始化种群服务器
    double calcCost();//计算适应度
    void calcFitness();//计算适应度
//    void mate();//个体交配，基因片段交换
//    void swap();//交换父子群体
//    void mutate();//突变操作
public:
    WeGa(char * filename):minCost{INF}{
        allServer.insert(allServer.end(),ChooseServer::serverID.begin(),ChooseServer::serverID.end());
        allServer.insert(allServer.end(),ChooseServer::serverCandidate.begin(),ChooseServer::serverCandidate.end());
        allServer.insert(allServer.end(),ChooseServer::serverPossible.begin(),ChooseServer::serverPossible.end());
        serverCountRange=(int)log(allServer.size());
        thefilename=filename;

    }
    void chooseServer();//ga选择服务器

};


void WeGa::chooseServer() {
    preDeal();

    for (int j = 0; j < ChooseServer::serverID.size()+ChooseServer::serverCandidate.size(); ++j) {
        if(BESTServer[j]){
            GServerID.push_back(allServer[j]);
        }
    }
    if(minCost<ChooseServer::minCost){
        mc.run(Graph::nodeCount,Graph::arcCount,GServerID);
    }
    else{
        mc.run(Graph::nodeCount,Graph::arcCount,ChooseServer::serverID);
    }

    //std::cout<<"mincost: "<<minCost<<std::endl;
    //write_result(mc.s,thefilename);
}

void WeGa::preDeal() {
    double reduceOneCost,addOneCost;
    initial();
    reduceOneServer();
    reduceOneCost=calcCost();
    printf(splitLine);
    if(reduceOneCost<ChooseServer::minCost){
        std::cout<<"we can reduce the server\n";
    }

    addOneServer();
    addOneCost=calcCost();
    if(addOneCost<ChooseServer::minCost && addOneCost<reduceOneCost){
        std::cout<<"we can add the server\n";
    }
    if(reduceOneCost>addOneCost){
        std::cout<<"we should add the server\n";
    } else{
        std::cout<<"we should reduce the server\n";
    }
    printf("lpCost: %.f \nreduceOneCost: %.f \naddOneCost: %.f\n",ChooseServer::minCost,reduceOneCost,addOneCost);
}
void WeGa::addOneServer() {
    for (int j = ChooseServer::serverID.size(); j < ChooseServer::serverID.size()+ChooseServer::serverCandidate.size(); ++j) {
        std::bitset<MAXCONSUMER> b{LPBestServer};
        //printf("j: %d",j);
        b.set(j);
        //std::cout <<b<<std::endl;
        gaPopulation.push_back(b);
    }
    //printf(splitLine);
    std::cout<<"addOneServer Popu Size : "<<gaPopulation.size()<<std::endl;
}
void WeGa::reduceOneServer() {
    //close a server from ServerID
    for(int i=0;i<ChooseServer::serverID.size();++i){
        std::bitset<MAXCONSUMER> b{LPBestServer};
        b.reset(i);
        gaPopulation.push_back(b);
        //std::cout<<b<<std::endl;
    }
    //printf(splitLine);
    std::cout<<"reduceOneServer Popu Size : "<<gaPopulation.size()<<std::endl;
    //open a server from ServerCandidate
}
void WeGa::initial() {
    for (int i = 0; i <ChooseServer::serverID.size() ; ++i) {
        LPBestServer.set(i);
    }
}

double WeGa::calcCost() {

    for (int i = 0; i <gaPopulation.size()  ; ++i) {
        std::vector<int> serverLoc;
        for (int j = 0; j < ChooseServer::serverID.size()+ChooseServer::serverCandidate.size(); ++j) {
            if(gaPopulation[i][j]){
                serverLoc.push_back(allServer[j]);
            }
        }

        mc.run(Graph::nodeCount,Graph::arcCount,serverLoc);
        if(minCost>mc.minicost){
            minCost=mc.minicost;
            BESTServer=gaPopulation[i];
        }
        //serverLoc.clear();
    }
    gaPopulation.clear();
    return minCost;
//    for (int j = 0; j < ChooseServer::serverID.size()+ChooseServer::serverCandidate.size(); ++j) {
//        if(BESTServer[j]){
//            GServerID.push_back(allServer[j]);
//        }
//    }
//    std::cout<<"mincost: "<<minCost<<std::endl;
//    write_result(m.s,thefilename);
}

//void WeGa::stageTwoGeneP() {
//    for(int i=0;i<ChooseServer::serverID.size();++i){
//        std::bitset<MAXCONSUMER> b;
//        b.bitset();
//        b.reset(i);
//        for (int j= i+1; j < ChooseServer::serverID.size(); ++j) {
//            b.reset(j);
//        }
//        gaPopulation.push_back(b);
//    }
//}




#endif //CDN_WEGA_H
