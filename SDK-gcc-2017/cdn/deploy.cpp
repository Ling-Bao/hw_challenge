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
    Graph G;
    G.init(topo);
    ChooseServer::lpChoose();//线性规划选择服务器
    ChooseServer::printServerInfo();//打印所选服务器信息
    ChooseServer::minCost=mc.run(Graph::nodeCount,Graph::arcCount,ChooseServer::serverID);
    timer.Begin();//计时开始
//    WeGa wega(filename);
//    wega.chooseServer();


    timer.End();//计时结束
//    std::cout<<"time: "<<timer.ms()<<std::endl;
//    result=mc.getRoute();
//    minCost=mc.minicost;
//    printf("we get minCost : %.f\n" ,minCost);
//    printf("%sminCost is : %.f\n\n%s\n",splitLine,minCost,result);
//    printf(splitLine);
//    write_result(result,filename);


    //write_result(mcmf.s,filename);

    //OurGA ourGA = OurGA();
    //ourGA.GaAlgorithmServer(ChooseServer::serverID, ChooseServer::serverCandidate, Graph::nodeCount, Graph::arcCount, filename);
    //int tmp[]={0,9,23,42,61,77,86,88,105,125,143,156,176,193,198,201,204,221,233,253,256,267,281,290,304,324,336,356,375,388,391,394,414,416,429,449,454,458,469,477,482,491,507,512,525,537,557,573,590,601,613,628,630,631,642,659,660,671,691,694,700,702,716,731,749,759,771,791};
    //int tmp[]={11, 13, 17, 22, 34, 49, 50, 54, 62, 63, 68, 75, 77, 81, 89, 95, 100, 102, 119, 121, 122, 127, 133, 135, 137, 141, 147, 148, 149, 150, 151, 167, 173, 183, 185, 190, 196, 204, 205, 218, 219, 221, 238, 245, 246, 250, 260, 263, 270, 271, 283, 284, 289, 290, 291, 297, 304, 311, 313, 319, 335, 348, 352, 360, 372, 378, 384, 385, 393, 394, 396, 406, 410, 417, 424, 444, 450, 465, 473, 477, 478, 482, 503, 506, 530, 537, 541, 571, 579, 581, 582, 585, 587, 590, 595, 597, 600, 607, 608, 615, 624, 628, 638, 645, 656, 668, 672, 682, 686, 687, 689, 691, 698, 700, 711, 721, 724, 729, 730, 732, 735, 736, 743, 763, 781};
    //int tmp[]={0,9,23,42,61,77,86,88,105,125,143,156,176,193,198,201,204,221,233,253,256,267,281,290,304,324,336,356,375,388,391,394,414,416,429,449,454,458,469,477,482,491,507,512,525,537,557,573,590,601,613,628,630,631,642,659,660,671,691,694,700,702,716,731,749,759,771,791};
    //int tmp[]={11,17,23,43,63,82,90,96,104,113,120,129,134,145,152,163,166,176,187,200,206,207,222,237,244,246,266,282,300,314,316,319,320,326,340,342,343,346,362,376,383,403,422,434,451,461,471,489,507,527,544,549,561,568,577,582,602,609,616,633,649,665,677,686,706,726,742,747,748,754,767,771,772,774,781,782};
    //ChooseServer::serverID.resize(sizeof(tmp)/ sizeof(int));
    //ChooseServer::serverID.assign(&tmp[0], &tmp[sizeof(tmp)/ sizeof(int)]);
    //memcpy(&ChooseServer::serverID[0],tmp, sizeof(tmp));

//    ZKW zkw;
//    zkw.run(Graph::nodeCount, Graph::arcCount, ChooseServer::serverID);
//    zkw.minicost;

    OurGA ourGA = OurGA(filename);
    if (ourGA.bSolve) {
        ourGA.GaAlgorithmServer();
    }
    printf(splitLine);
    //int t0=clock();

}





