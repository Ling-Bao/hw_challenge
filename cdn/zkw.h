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
public :
    int minicost;
private :
    std::vector<Link> edge;//图中边总数
    std::vector<int> head;//每个点出发的边的链表首节点
    std::vector<PRE> pre;
    std::vector<int> dist;
    std::deque<int> pat;
    std::vector<Path> path;//记录所有的路径
    std::bitset<MAX_NODE_NUM+2> mark;

    int po;
    int max_flow;
    int cost;

    int path_num;//生成流的路径数量

    void Init(int num_n,int num_e,std::vector<int> &serverLocation)
    {
        po=0;
        max_flow=0;
        minicost=0;
        cost=0;
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
        path.clear();
        for(int i=0;i<node_num;i++)
            for(int j=0;j<node_num;j++)
            {
                search_edge[i][j]=-1;
                search_cost[i][j]=0;
                gr[i][j]=0;
            }

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
    void add_edge(int u,int v, int cap, int cost)
    {

        edge[po].u=u;
        edge[po].v=v;
        edge[po].cap=cap;
        edge[po].cost=cost;
        edge[po].next=head[u];
        search_edge[u][v]=po;
        search_cost[u][v]=cost;
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

        double sum_Q=0.0;
        std::deque<int> q;
        int cur,cur_v;
        dist.clear();
        pre.clear();
        dist.resize(node_num,INF);
        pre.resize(node_num,PRE());

        q.push_back(t);
        dist[t]=0;
        sum_Q+=dist[t];

        while(!q.empty())
        {
            cur=q.front();
            q.pop_front();
            sum_Q-=dist[cur];
            for(int i=head[cur];i!=-1;i=edge[i].next)
            {
                cur_v=edge[i].v;
                if(edge[i^1].cap>0&&edge[i].cost<=0&&dist[cur_v]>dist[cur]-edge[i].cost)
                {
                    dist[cur_v]=dist[cur]-edge[i].cost;
                    pre[cur_v].n=cur;
                    pre[cur_v].e=i;
                    sum_Q+=dist[cur_v];
                    if(q.empty())
                    {
                        if(dist[cur_v] <= dist[s])
                            q.push_front(cur_v);
                        else
                            q.push_back(cur_v);
                    }
                    else
                    {
                        if(dist[cur_v]<=dist[q.front()]&&dist[cur_v]<=sum_Q/q.size())
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


    bool search_path(int s,double f,double c,deque<int> no)
    {
        no.push_back(s);
        if(s!=sink_p)
        {
            for(int i=head[s];i!=-1;i=edge[i].next)
            {
                int v=edge[i].v;
                if(gr[s][v]>0)
                {
                    int tmp_f=f<gr[s][v] ? f:gr[s][v];
                    int tmp_c=c+search_cost[s][v];
                    if(search_path(v,tmp_f,tmp_c,no))
                        return true;
                }
            }
            return false;
        }
        else if(s==sink_p)
        {
            Path temp;
            temp.nodes=no;
            temp.flow=f;
            temp.cost=c*f;
            //temp.Print();
            path.push_back(temp);
            for(int i=0;i<temp.nodes.size()-1;i++)
            {
                int k1=temp.nodes[i];
                int k2=temp.nodes[i+1];
                gr[k1][k2]-=f;
            }
            return true;
        }
    }

    void c_result()
    {
        /***S需要初始化***/
        while(spfa(source_p,sink_p))
        {
            do
            {
                path.clear();
                mark.reset();
            } while(aug(source_p, INF ,pat));
        }
    }
public :
    char s[999999];
    double a=0.0;
    double run(int num1,int num2,std::vector<int> &serverLocation)
    {

        Init(num1,num2,serverLocation);
        c_result();
        if(max_flow>=all_demand)
        {
            minicost+=serverLocation.size()*Graph::serverFee;
//            std::cout<<"Cost:"<<minicost<<std::endl;
//                if(minicost<=455){
//                    printf("max_flow: %.f, all_demand= %.f \n",max_flow,all_demand );
//                    exit(-1);
//                }
            return minicost;
        }
        return INF;
    }
    char* getRoute(){
        if(max_flow<all_demand)
        {
            sprintf(s,"NA\n");
            //std::cout<<"NA\n"<<std::endl;
            minicost=INF;
        }
        else{

            for(int i=0;i<node_num;i++)
            {
                for(int j=head[i];j!=-1;j=edge[j].next)
                {
                    if(j%2==0&&edge[j^1].cap>0)
                        gr[i][edge[j].v]=edge[j^1].cap;
                }
            }
            path.clear();
            deque<int> tmp;
            double cal=0;
            while(search_path(source_p,INF,0,tmp))
            {
                tmp.clear();
            }
            for(Path pa:path)
            {
                cal+=pa.cost;
            }
            cout<<minicost<<":"<<cal<<endl;

            int offset{0};
            offset+=sprintf(s+offset,"%d\n\n",path_num);
            //sprintf(s,"%d\n\n",path_num);
            for(Path pa : path)
            {
                //pa.Print();
                if(pa.nodes.front()==source_p)
                    pa.nodes.pop_front();
                if(pa.nodes.back()==sink_p)
                    pa.nodes.pop_back();
                int t=pa.nodes.back();
                pa.nodes.push_back(Graph::netNode[t]->consumerId);
                //char s1[1000];
                for(int n : pa.nodes)
                {
                    offset+=sprintf(s+offset,"%d ",n);
                    // sprintf(s1,"%d ",n);
                    //strcat(s,s1);
                }
                offset+=sprintf(s+offset,"%.lf\n",pa.flow);
            }
            s[offset-1]='\0';
            //std::cout<<s<<std::endl;
        }


        return s;

    }
};

#endif // ZKW_H_INCLUDED