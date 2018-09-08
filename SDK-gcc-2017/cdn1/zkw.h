/*
 * Copyright 2017
 *
 * z.h
 *
 * zkw算法
 *
 * Date: March 26, 2017
 * Author: Wei Liu, Ling Bao, Shangyue Zhou
 *
 * $Id: zkw.h,v 0.1 2017/03/26 00:30:30 zsy Exp $
 *
 */
#ifndef ZKW_H_INCLUDED
#define ZKW_H_INCLUDED


struct ZKW {
public:
    double minicost;
    char s[MAX_OUT_CHAR_NUM];
private :
    std::vector<Link> edge;
    std::vector<int> head;
    std::vector<PRE> pre;
    std::vector<int> dist;
    std::deque<int> pat;
    std::vector<Path> path;
    std::bitset<MAX_NODE_NUM> mark;
    int po;
    double max_flow;
    double cost;

    int path_num;//生成流的路径数量

    void Init(int num_n,int num_e,std::vector<int> &serverLocation)
    {
        po=0;
        max_flow=0;
        minicost=0.0;
        cost=0.0;
        path_num=0;
        node_num=num_n+2;
        link_num=num_e;
        int edgesize=link_num*2+serverLocation.size()+Graph::consumerCount;
        source_p =node_num-2;
        sink_p=node_num-1;
        all_demand=0;
        head.clear();
        edge.clear();
        path.clear();
        pre.clear();
        dist.clear();
        pat.clear();
        mark.reset();
        head.resize(node_num,-1);
        edge.resize(edgesize*2+1,Link());


        for(int i : serverLocation)
        {
            add_edge(source_p,i,INF,0);//构造超源点和部署服务器节点的链路
        }
        for(int i=0;i<node_num-2;i++)
        {
            Arc *a=Graph::netNode[i]->arc;
            while(a!=nullptr)
            {
                if(a->node0==i)
                    add_edge(a->node0,a->node1,a->capacity,a->cost);
                a=a->next;
            }
        }//构造网络链路
        for(int i=0;i<Graph::consumerCount;i++)
        {
            int net=Graph::consumerNode[i];
            if(Graph::netNode[net]->isWithConsumer){
                all_demand+=Graph::netNode[net]->require;
                add_edge(net,sink_p,Graph::netNode[net]->require,0);
            }

        }//构造连接消费节点的网络节点与超汇点之间的链路；
    }
    //Init负责构造网络图，加入节点，是与外部的接口；
    void add_edge(int u,int v, double cap, double cost)
    {

        edge[po].u=u;
        edge[po].v=v;
        edge[po].cap=cap;
        edge[po].cost=cost;
        edge[po].next=head[u];
        head[u]=po++;

        edge[po].u=v;
        edge[po].v=u;
        edge[po].cap=0;
        edge[po].cost=-cost;
        edge[po].next=head[v];
        head[v]=po++;
    }

    bool spfa(int s, int t)
    {
        std::deque<int> q;
        int cur,cur_v;
        dist.clear();
        dist.clear();
        dist.resize(node_num,INF);
        pre.resize(node_num,PRE());
        //mark.reset();

        q.push_back(t);
        //mark.set(t);
        dist[t]=0;
        //std::cout<<"NBbu!"<<std::endl;

        while(!q.empty())
        {
            cur=q.front();
            //mark.reset(cur);
            q.pop_front();
            for(int i=head[cur];i!=-1;i=edge[i].next)
            {
                cur_v=edge[i].v;
                if(edge[i^1].cap>0&&dist[cur_v]>dist[cur]-edge[i].cost)
                {
                    dist[cur_v]=dist[cur]-edge[i].cost;
                    pre[cur_v].n=cur;
                    pre[cur_v].e=i;

                    if(q.empty())
                    {
                        if(dist[cur_v] <= dist[s])
                            q.push_front(cur_v);
                        else
                            q.push_back(cur_v);
                    }
                    else
                    {
                        if(dist[cur_v]<=dist[q.front()])
                            q.push_front(cur_v);
                        else
                            q.push_back(cur_v);
                    }

                }
            }

        }


        for (int i = 0; i < node_num; ++i)
        {
            for(int j = head[i]; j != -1; j = edge[j].next)
            {
                edge[j].cost+= dist[edge[j].v] - dist[i];
            }
        }

        cost+=dist[s];
        //std::cout<<"Cost:"<<cost<<std::endl;
        return dist[s]<INF;
    }

    double aug(int u, double f, std::deque<int> pat)
    {
        if(u == sink_p){
            minicost += cost * f;
            max_flow += f;

            Path temp;
            temp.nodes = pat;
            temp.flow = f;
            temp.cost = cost;
            path.push_back(temp);
            path_num++;
            return f;
        }
        double tmp = f;
        mark.set(u);
        pat.push_back(u);

        for(int i = head[u]; i != -1;i = edge[i].next){
            int v = edge[i].v;
            if(edge[i].cap && !edge[i].cost && !mark.test(v)){
                double tmp_f = tmp < edge[i].cap ? tmp : edge[i].cap;
                double cap = aug(v, tmp_f, pat);
                edge[i].cap -= cap;
                edge[i^1].cap += cap;
                tmp -= cap;
                if(!tmp) return f;
            }
        }

        return f-tmp;
    }

public :
    double run(int num1,int num2,std::vector<int> &serverLocation)
    {
        Init(num1,num2,serverLocation);

        while(spfa(source_p,sink_p))
        {

            do
            {
                mark.reset();
                pat.clear();
            } while(aug(source_p, INF, pat));
        }
        if(max_flow<all_demand)
        {
            sprintf(s,"NA\n");
            //std::cout<<"NA"<<std::endl;
            minicost=INF;
        } else{
            minicost+=serverLocation.size()*Graph::serverFee;
        }

        //std::cout<<"Cost:"<<minicost<<std::endl;
        return minicost;
    }
    char* getRoute(){

        int offset{0};
        offset+=sprintf(s+offset,"%d\n\n",path_num);
        //sprintf(s,"%d\n\n",path_num);
        for(Path pa : path)
        {
            //pa.Print();
            pa.nodes.pop_front();
            //pa.nodes.pop_back();
            int t=pa.nodes.back();
            pa.nodes.push_back(Graph::netNode[t]->consumerId);
            //char s1[1000];
            for(int n : pa.nodes)
            {
                offset+=sprintf(s+offset,"%d ",n);
                // sprintf(s1,"%d ",n);
                //strcat(s,s1);
            }
            offset+=sprintf(s+offset,"%.f\n",pa.flow);

        }
        s[offset-1]='\0';
        //std::cout<<s<<std::endl;
        return s;

    }

};

#endif // ZKW_H_INCLUDED