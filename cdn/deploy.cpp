/*
 * Copyright 2017
 *
 * deploy.cpp
 *
 * 功能入口
 *
 * Date: March 26, 2017
 * Author: Wei Liu, Ling Bao, Shangyue Zhou
 *
 * $Id: deploy.h,v 0.1 2017/03/26 00:22:35 bl Exp $
 *
 */
#include "deploy.h"
#include "common.h"

//#define Mc
#define Zk

//你要完成的功能总入口
void deploy_server(char * topo[MAX_EDGE_NUM], int line_num,char * filename)
{
    char* result;
    double minCost;


    int t0=clock();

    // 图
    Graph G;
    G.init(topo);

    // 线性规划
    //printf(splitLine);
    ChooseServer::lpChoose();//线性规划选择服务器

    // 遗传算法
    //printf(splitLine);
//    if (ChooseServer::serverID.size() + ChooseServer::serverCandidate.size() + ChooseServer::serverPossible.size() > 300) {
//        OurGA ourGA = OurGA(filename);
//        if (ourGA.bSolve) {
//            ourGA.GaAlgorithmServer();
//        }
//    }
//    else {
//        OurGA1 ourGA = OurGA1(filename);
//        if (ourGA.bSolve) {
//            ourGA.GaAlgorithmServer();
//        }
//    }

//    OurGA1 ourGA = OurGA1(filename);
//    if (ourGA.bSolve) {
//        ourGA.GaAlgorithmServer();
//    }
    OurGA ourGA = OurGA(filename);
    if (ourGA.bSolve) {
        ourGA.GaAlgorithmServer();
    }
//
//    printf(splitLine);
//    std::cout<<clock() - t0<<std::endl;

    /**--------------------测试------------------------------*/
    //std::vector<int> serverTmp{ChooseServer::serverID};
//    serverTmp.insert(serverTmp.end(),ChooseServer::serverCandidate.begin(),ChooseServer::serverCandidate.end());
//    serverTmp.insert(serverTmp.end(),ChooseServer::serverPossible.begin(),ChooseServer::serverPossible.end());
    //std::vector<int> sv;
    //int sr[]={17,26,32,35,43,50,59,74,82,89,96,101,111,120,124,126,138,147,161,166,167,175,178,186,187,194,203,205,218,227,234,238,242,252,254,263,267,270,271,275,278,281,288,308,321,326,328,331,334,336,338,346,349,363,370,375,383,387,397,402,409,415,416,423,426,438,459,463,464,474,475,482,491,500,503,505,507,520,525,529,542,548,552,557,570,576,584,592,594,617,632,634,641,643,646,660,663,671,675,679,686,711,718,719,724,731,739,742,745,751,756,757,760,767,770,774,778,784,791,795};
    //sv.assign(&sr[0],&sr[sizeof(sr)/sizeof(int)]);
//    std::string case0_best{"111111111111111011111111111111111111111110111101011111101111010111000110110011111111111011011010111000010011001101011101101010110100011100010000010001100010100001110001000100100000101000100100001000010001000000001000000000000000000000000000000001000000000000000000000010000000000000000010000000000000000000000000000000000000000000000000000000000000000000000000"};
//    printf("\n");
//    for (int i = 0; i <case0_best.size() ; ++i) {
//        if(case0_best.at(i)=='1'){
//            sv.push_back(serverTmp[i]);
//            printf("%d ",serverTmp[i]);
//        }
//    }
//    printf("\n");
    //WeFastMCMF m;
//    std::vector<int> delV;
//    //delV.push_back(9);
//    delV.push_back(14);
//    delV.push_back(7);
//    WeFastMCMF m;
//    m.run(0,0,sv);
//    ChooseServer::printServerInfo();
    //m.getRoute();
    //m.deleteServer(delV);
    //m.addServer(delV);
    //m.clearData(1000);
    //printf("result:%s\n minCost:%.f\n",m.result,m.minCost);
    /**----------------------------------------------------*/

}





