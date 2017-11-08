/*
 * Copyright 2017
 *
 * common.h
 *
 * 包含所有模块，宏定义，头文件
 *
 * 注意：包含顺序，否则可能出错
 *
 * Date: March 26, 2017
 * Author: Wei Liu, Ling Bao, Shangyue Zhou
 *
 * $Id: common.h,v 0.1 2017/03/26 00:22:35 lw Exp $
 *
 */

#ifndef CDN_COMMON_H
#define CDN_COMMON_H


#define MAXNODE 1004                //最大节点数
#define MAXCONSUMER 500             //最大消费节点数
#define MAXVALUE 1000.0             //设定的最大值
#define MIN_VALUE 0.0005              //用来界定选择服务器,serverID 0.0005
#define MIN_VALUE_ZERO 0.000001        //用来界定所有备选服务器,serverCandidate 0.000001
#define MIN_VALUE_ZERO_DOWN 0.000005     //界定线性规划包含的可能服务器,serverPossible 0.000005
#define MAX_VALUE_SUPER_OUT 2000.0  //用来设定超源节点与服务器位置关系，作为服务器最大输出系数
#define SERVER_VAL_PARA 1.0         //服务器价格参数，用来变换服务器价格

#define INF 1.0e8                    //极大值
//#define MAX_EDGE_NUM_G (2000*20+1500)
#define MAX_NODE_NUM 1010           //最大节点数
#define MAX_OUT_CHAR_NUM 200000     //用于设定输出时的字符数组长符

//#define WEDEBUG                     //用于打印基本信息与计算过程

//c库
#include <cstring>
#include <cstdlib>
#include <cmath>
#include <cassert>
#include <ctime>

//c++
#include <iostream>
#include <string>
#include <vector>
#include <bitset>
#include <list>
#include <stack>
#include <queue>
#include <unordered_map>
#include <algorithm>

//we
#include "TimeCalculator.h" //0、计时器

//全局变量
TimeCalculator timer;       //计时器
char splitLine[]="---------------------------------split line--------------------------------------\n";

#include "Graph.h"          //1、建图
#include "DirectOut.h"      //1.5、直接输出
#include "lwlp.h"           //2、建线性规划
#include "chooseServer.h"   //3、开始选择服务器

#include "mc_zk.h"          //4、mcmf和zkw共有头文件
#include "zkw.h"            //4.2、zkw
#include "best.h"
#include "WeMCMF.h"         //4.2 MCMF优化
#include "WEMCMF1.h"
#include "WeFastMCMF.h"
#include "ga.h"
#include "ga1.h"


#endif //CDN_COMMON_H
