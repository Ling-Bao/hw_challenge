//
// Created by lw_co on 2017/4/8.
//

#ifndef CDN_BEST_H
#define CDN_BEST_H

//
//struct capCostOrder{
//    int id;
//    double total;
////    bool operator < (const capCostOrder &d) const{
////        return total>d.total;//默认为从小到大，所以这里改了
////    }
//    capCostOrder(int i, double v):id{i},total{v} {}
//
//};
//static bool capCostSort(capCostOrder x, capCostOrder y) {
//    return (x.total > y.total);//从大到小
//}
struct Best {
    std::vector<int> getBest(std::vector<int> &server){
        int flag[MAXNODE]{0};
        for (int j = 0; j < server.size() ; ++j) {
            flag[server[j]]=1;
        }
        //最后一次的cost，应该还是可以
//        for(int i=0;i<Graph::nodeCount;++i){
//            if(WeFastMCMF::mCost[i]>Graph::serverFee && WeFastMCMF::mCost[server[i] ]<INF){
//                if(flag[i]!=1){
//                    server.push_back(i);
//                }
//            }
//        }
        //对每个点需求评价
        int costTotal[MAXNODE];
        //std::vector<capCostOrder> cap;
        for(int i=0;i<Graph::nodeCount;++i){
            Arc *p=Graph::netNode[i]->arc;
            int total;
            while(p!= nullptr){
                total += (p->capacity-p->mCapacity)*p->cost;
                p=p->next;
            }
            costTotal[i]=total;
//            capCostOrder t(i,total);
//            cap.push_back(t);
        }
        //std::sort(cap.begin(),cap.back(),capCostSort);
        for (int k = 0; k < MAXNODE ; ++k) {
            if((costTotal[k]*2)>Graph::serverFee&&flag[k]!=1){
                server.push_back(k);
            }
        }
        return server;
    }
};


#endif //CDN_BEST_H
