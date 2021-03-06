/*
 * Copyright 2017
 *
 * mc_zk.h
 *
 * mcmf_zk的共同结构
 *
 * Date: March 26, 2017
 * Author: Wei Liu, Ling Bao, Shangyue Zhou
 *
 * $Id: mc_zk.h,v 0.1 2017/03/26 00:30:30 zsy Exp $
 *
 */
#ifndef GLOBAL_H_INCLUDED
#define GLOBAL_H_INCLUDED

struct Link{
    int u;
    int v;
    int next;
    double cap;
    double cost;
    Link():u{-1}, v{-1},next{-1} ,cap{0.0}, cost{INF}  {}
};//记录边

struct PRE{
    int n;
    int e;
    PRE(){
        n=-1;
        e=-1;
    };
};//记录前驱点以及和前驱点构成的边号

struct Path{
    double cost;
    double flow;
    std::deque<int> nodes;
    Path()
    {
        cost=0;
        flow=0;
        nodes.clear();
    }
    void Print()
    {
        std::cout<<"Flow: "<<flow<<" Cost: "<<cost<< std::endl;
        for(int p:nodes)
        {
            std::cout<<p<<"-";
        }
        std::cout<<std::endl;
    }
};

int source_p;//超源点
int sink_p;//超汇点
int node_num;//网络结点数量
int link_num;//边数量
double all_demand;//所有消费结点的总需求

#endif // GLOBAL_H_INCLUDED