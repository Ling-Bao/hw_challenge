/*
 * Copyright 2017
 *
 * ga.h
 *
 * 遗传算法优化初始解
 *
 * Date: March 26, 2017
 * Author: Wei Liu, Ling Bao, Shangyue Zhou
 *
 * $Id: ga.h,v 0.1 2017/03/26 00:30:30 bl Exp $
 *
 */

#ifndef CDN_GA_H
#define CDN_GA_H


#define MAX_COST 1e8
// 85000000
#define TIME_END 85000000


// 遗传算法实现函数
class OurGA {
private:
    // 默认初始化参数

    // 种群基本参数
    int ga_max_iterate = 3200;	// 最大迭代次数 16384
    int ga_size; // 种群大小 2048

    // 初始化优秀基因与劣等基因变异率
    double ga_init_good_rate = 0.10f;
    double ga_init_bad_rate = 0.10f;

    float ga_elitism_rate = 0.25f; // 精英比率 0.10f
    int decay_e_step = 2; // 多少步进行精英增加以及变异率增加 10 (2)
    double decay_e_rate;  // 不同等级基因大小的群体的衰减率
    int esize; // 精英在群体中的数量ga_size*ga_elitism_rate_now

    double decay_m_rate = 0.99;
    int mutate_step = 3; // 多少步后不同阶层个体突变情况的变化 10 (3)
    float ga_mutation_rate  = 0.25f; // 变异率 0.25f
    float ga_mutation; // 基因变异码

    // 突变与精英率每次衰减率值
    float decay_rate = 0.05;

    // 全局步骤
    int ga_step;

    // 等级演化初始分布
    // 正向递减high=30--->high_line=10;middle=70+++>middle_line=90.所以中间模糊基因段变异概率会慢慢变大
    int high = 30;
    int middle = 70;
    int high_line = 10;
    int middle_line = 90;
    // 正向递减high=10+++>high_line=35;middle=90--->middle_line=65.所以中间模糊基因段变异概率会慢慢变小
    bool hm_flag = false;
    int hm_line_high = 35;
    int hm_line_middle = 65;

    // 保存本次迭代最优基因及其适应度
    std::string ga_s;
    double ga_minicost = INF;

    // 写结果文件
    char *ga_filename;

    // 网络相关的动态初始化参数
    std::vector<int> ga_server = ChooseServer::serverID; // 工作原码
    int ga_target_size = ChooseServer::serverID.size(); // 工作基因长度

    // 基因片段不同等级划分
    // ServerID
    std::vector<int> ga_Id_server;
    int size_Id_server;
    // ServerCandidate
    std::vector<int> ga_Candidate_server;
    int size_Candidate_server;
    // ServerPossible
    std::vector<int> ga_possible_server;
    int size_Possible_server;

    // 求解最小费用流
    ZKW ga_run;
    //MCMF ga_run;

    // 遗传算法结构体，包括适应与最优解
    struct ga_struct {
        std::string str; // 个体基因编码串
        double fitness; // 个体适应度
    };
    typedef std::vector<ga_struct> ga_vector;// 合并写法

public:
    bool bSolve;

    // ----已测试----
    // 将下标对应到serverID
    std::vector<int> to_serverID(std::vector<int> tmp_serverID, int middle) {
        std::vector<int> done_serverID;
        int i = 0;

        for (auto tmp = tmp_serverID.begin(); i < middle; ++i, ++tmp) {
            done_serverID.push_back(*tmp);
        }

        return done_serverID;
    }

    // ----已测试----
    // 第一条线的二分搜索
    int binary_search_one(std::vector<int> bs_serverID, int low, int high, double *bs_minicost) {
        std::vector<int> find_line_bs;
        double minicost_bs = INF;
        int i = 0; // 用于提前终止二维搜索
        //如果无法找到解，那么默认为(high+low)/2
        int real_middle = -1;

        while(low<=high){
            //中间位置计算,low+最高位置减去最低位置,右移一位,相当于除2.也可以用(high+low)/2
            int middle=low + ((high-low)>>1);

            //计算此时的耗费
            find_line_bs = to_serverID(bs_serverID, middle);
            ga_run.run(Graph::nodeCount,Graph::arcCount, find_line_bs);
            minicost_bs = ga_run.minicost;

            // 二维搜索
            if(i==10) {
                // 最多二分八次
                *bs_minicost = minicost_bs;

                return real_middle;
            }

            if(minicost_bs < INF) {
                // 有解，向左搜索
                high=middle-1;
                real_middle = middle+1;
            }else {
                // 无解，向右搜索
                low=middle+1;
            }

            // 迭代次数统计
            i++;
        }

        // 迭代资源未用完，代表已精确定位
        if (i < 10) {
            // 精确搜索到
            *bs_minicost = minicost_bs;

            return real_middle;
        }
    }

    // ----已测试----
    // 第二条线的二分搜索
    // 此处需要做边界处理，当minicost_bs的值均为小于或大于one_minicost时取（candidate_size+possible_size）/2
    int binary_search_two(std::vector<int> bs_serverID, double one_minicost, int low, int high) {
        std::vector<int> find_line_bs;
        double minicost_bs = INF;
        int i = 0; // 用于提前终止二维搜索
        //如果无法找到解，那么默认为(high+low)/2
        int real_middle = -1;

        while(low<=high){
            //中间位置计算,low+最高位置减去最低位置,右移一位,相当于除2.也可以用(high+low)/2
            int middle=low + ((high-low)>>1);

            //计算此时的耗费
            find_line_bs = to_serverID(bs_serverID, middle);
            ga_run.run(Graph::nodeCount,Graph::arcCount, find_line_bs);
            minicost_bs = ga_run.minicost;

            // 二维搜索
            if(i==10) {
                // 最多二分八次
                return real_middle;
            }

            if(minicost_bs < one_minicost) {
                // 比one_minicost小，可能不包含最优解集，向右搜索
                low=middle+1;
            }else {
                // 比one_minicost大，可能包含太多最优解集，向左搜索
                high=middle-1;
                real_middle = high+1;
            }

            // 迭代次数统计
            i++;
        }

        // 迭代资源未用完，代表已精确定位
        if (i < 10) {
            // 精确搜索到
            return real_middle;
        }
    }

    // ----已测试----
    // 找分界线
    bool finding_boundary() {
        // 初始化临时变量
        // ServerID
        std::vector<int> ga_wk_tmp = ChooseServer::serverID;
        int size_Id_tmp = ChooseServer::serverID.size();
        // ServerCandidate
        ga_wk_tmp.insert(ga_wk_tmp.end(),
                         ChooseServer::serverCandidate.begin(), ChooseServer::serverCandidate.end());
        int size_Candidate_tmp = ChooseServer::serverCandidate.size();
        // ServerPossible
        ga_wk_tmp.insert(ga_wk_tmp.end(),
                         ChooseServer::serverPossible.begin(), ChooseServer::serverPossible.end());
        int size_Possible_tmp = ChooseServer::serverPossible.size();

        //*******************************第一分线****************************************//
        // 用于保存第一条线的serverID和数组索引值
        std::vector<int> find_line_one = ChooseServer::serverID;
        int size_find_line_one = find_line_one.size();

        // 计算初始serverID耗费
        double minicost_tmp_one = INF;
        ga_run.run(Graph::nodeCount, Graph::arcCount, find_line_one);
        minicost_tmp_one = ga_run.minicost;

        // 开始寻找第一条线
        if (minicost_tmp_one < INF) {
            // 在serverID于搜索
            size_find_line_one = binary_search_one(ChooseServer::serverID, 0, size_Id_tmp, &minicost_tmp_one);

            // 将第一条线的元素还原
            std::vector <int>().swap(ChooseServer::serverID);
            ChooseServer::serverID = to_serverID(ga_wk_tmp, size_find_line_one);
        }
        else {
            // tested, Ling Bao
            // 注意，此处假设serverID+candidate+possible一定有解，所以不做条件判断
            //

            // 在candidate和possible搜索，可将candidate与possible合并
            size_find_line_one = binary_search_one(ga_wk_tmp, size_Id_tmp, ga_wk_tmp.size(), &minicost_tmp_one);

            // 做边界处理
            if (size_find_line_one == -1) {
                std::cout<<"May be Original LP parameter is not suitable, I will use half of length!\n"<<std::endl;
                std::cout<<"Warning, This may be cause some problem, Please check it again!\n"<<std::endl;
                size_find_line_one = size_Id_tmp + ((ga_wk_tmp.size()-size_Id_tmp)>>1);
            }

            // 将第一条线的元素还原ChooseServer::serverID
            std::vector <int>().swap(ChooseServer::serverID);
            ChooseServer::serverID = to_serverID(ga_wk_tmp, size_find_line_one);

            // 将第一条线的元素还原find_line_one
            std::vector <int>().swap(find_line_one);
            find_line_one = to_serverID(ga_wk_tmp, size_find_line_one);
        }

        //*******************************第二分线****************************************//
        // 开始寻找第二条线
        //
        //注意：第一条线找到的serverID可能出现minicost=INF的情况（暂时不管）
        //
        double minicost_tmp_two;
        if (minicost_tmp_one < INF) {
            minicost_tmp_two = minicost_tmp_one;
        }
        else {
            std::cout<<"ServerID's minicost is INF!"<<std::endl;
        }

        // 用于保存第二条线的serverID和数组索引值
        std::vector<int> find_line_two = ga_wk_tmp;
        int size_find_line_two = ga_wk_tmp.size();

        // 开始第二条线的二分一维搜索
        size_find_line_two = binary_search_two(ga_wk_tmp, minicost_tmp_two, size_find_line_one, size_find_line_two);

        // 做边界处理
        if (size_find_line_two == size_find_line_one || size_find_line_two == ga_wk_tmp.size()
            || size_find_line_two == -1) {
            std::cout<<"Can't solve the second line, I will get half of the length!\n"<<std::endl;
            size_find_line_two = (size_find_line_one + ga_wk_tmp.size()) / 2;
        }

        // 将第二条线的元素还原serverPossible
        //serverCandidate
        std::vector <int>().swap(ChooseServer::serverCandidate);
        for (int i = size_find_line_one; i < size_find_line_two; ++i) {
            ChooseServer::serverCandidate.push_back(ga_wk_tmp[i]);
        }

        //serverPossible
        std::vector <int>().swap(ChooseServer::serverPossible);
        for (int j = size_find_line_two; j < ga_wk_tmp.size(); ++j) {
            ChooseServer::serverPossible.push_back(ga_wk_tmp[j]);
        }

        // 基因片段不同等级划分
        // ServerID
        ga_Id_server = ChooseServer::serverID;
        size_Id_server = ChooseServer::serverID.size();
        // ServerCandidate
        ga_Candidate_server = ChooseServer::serverCandidate;
        size_Candidate_server = ChooseServer::serverCandidate.size();
        // ServerPossible
        ga_possible_server = ChooseServer::serverPossible;
        size_Possible_server = ChooseServer::serverPossible.size();

        return true;
    }

    // ----已测试----
    // 初始化化参数
    OurGA(char *filename) {
        std::cout<<"使用GA默认初始参数！"<<std::endl;

        // 初始化种群大小
        if (ga_target_size < 100) {
            ga_size = 50;
        }
        else if ((ga_target_size >= 100) && (ga_target_size < 200)) {
            ga_size = 30;
        }
        else if ((ga_target_size >= 200) && (ga_target_size < 300)) {
            ga_size = 20;
        }
        else if ((ga_target_size >= 300) && (ga_target_size < 400)) {
            ga_size = 15;
        }
        else {
            ga_size = 10;
        }

        // 同等级基因大小的群体的衰减率
        if (ga_target_size < 100) {
            decay_e_rate = 0.70;
        }
        else if ((ga_target_size >= 100) && (ga_target_size < 200)) {
            decay_e_rate = 0.75;
        }
        else if ((ga_target_size >= 200) && (ga_target_size < 300)) {
            decay_e_rate = 0.80;
        }
        else if ((ga_target_size >= 300) && (ga_target_size < 400)) {
            decay_e_rate = 0.85;
        }
        else {
            decay_e_rate = 0.90;
        }

        // 精英在群体中的数量ga_size*ga_elitism_rate_now
        esize = ceil(ga_size * ga_elitism_rate);

        // 基因变异码
        ga_mutation = RAND_MAX * ga_mutation_rate;

        // 写结果文件
        ga_filename = filename;

        // 二分一维搜索重新确定LP的两条线
        finding_boundary();

        // 网络相关的动态初始化参数
        ChooseServer::serverID.insert(ChooseServer::serverID.end(),
                                      ga_Candidate_server.begin(), ga_Candidate_server.end());
        ChooseServer::serverID.insert(ChooseServer::serverID.end(),
                                      ga_possible_server.begin(), ga_possible_server.end());
        ga_target_size = ChooseServer::serverID.size();
        ga_server = ChooseServer::serverID;

        std::cout<<"GA length:"<<ga_target_size<<std::endl;

        //ZKW ga_run;
        ga_run.run(Graph::nodeCount,Graph::arcCount, ChooseServer::serverID);
        if (ga_run.minicost >= INF) {
            printf("No LP Solve!\n");

            bSolve = false;
        }
        else {
            write_result(ga_run.getRoute(), ga_filename);
            bSolve = true;
        }
    }

    // ----已测试----
    // 初始化函数
    // 备注，暂时不用
    OurGA(int size, int max_iterate, float elitism_rate, float mutation_rate) {
        ga_size = size;
        ga_max_iterate = max_iterate;
        ga_elitism_rate = elitism_rate;
        ga_mutation_rate = mutation_rate;
    }

    // ----已测试----
    // 初始化阶段，优秀基因段变异概率
    int init_good_design() {
        int init_good = RAND_MAX * ga_init_good_rate;
        if (rand() < init_good) {
            return 0;
        }
        else {
            return 1;
        }
    }

    // ----已测试----
    // 初始化阶段，劣等基因段变异概率
    int init_bad_design() {
        int init_bad = RAND_MAX * ga_init_bad_rate;
        if (rand() < init_bad) {
            return 1;
        }
        else {
            return 0;
        }
    }

    // ----已测试----
    // 初始化种群（服务器）
    void init_population_server(ga_vector &population, ga_vector &buffer ) {
        for (int i=0; i<ga_size; i++) {
            ga_struct citizen;
            citizen.fitness = INF;
            citizen.str.erase();

            // 初始化个体基因
            if (i == 0) {
                // 将最优个体基因加入种群
                for (int j=0; j<ga_target_size; j++) {
                    citizen.str += std::to_string(1);
                }
                ga_s.clear();
                ga_s = citizen.str;
            }
            else {
                // 初始化其余个体基因
                // 对基因序列进行分段初始化
                for (int j=0; j<ga_target_size; j++) {
                    if (j >= 0 && j < size_Id_server) {
                        // 第一段0～serverID.size(), 90%为1
                        citizen.str += std::to_string(init_good_design());
                    }
                    else if (j >= size_Id_server && j < (size_Id_server + size_Candidate_server)) {
                        // 第三段serverCandidate.seze()+1~serverPossible.size(), 90%为0
                        citizen.str += std::to_string(rand() % 2);
                    }
                    else {
                        // 第二段serverID.size()+1~serverCandidate.size(),0与1一半一半
                        citizen.str += std::to_string(init_bad_design());
                    }
                }
            }

            // 将个体放入种群
            population.push_back(citizen);
        }

        // 分配一个存储空间
        buffer.resize(ga_size);

        //PrintGA(population);
    }

    // ----已测试----
    // 计算适应度（服务器）
    void calc_fitness_server(ga_vector &population, int step) {
        // 查看种群个体基因序列
        //PrintGA(population);

        // 第一次计算适应度精英数未知
        int tmp_esize;
        if (step == 0) {
            tmp_esize = 0;
        }
        else {
            tmp_esize = esize;
        }

        for (int i=tmp_esize; i<ga_size; i++) {
            // 清空服务器节点放置缓存
            std::vector <int>().swap(ChooseServer::serverID);
            for (int j=0; j<ga_target_size; j++) {
                if (population[i].str[j] == '1') {
                    ChooseServer::serverID.push_back(ga_server[j]);
                }
            }

            // 查看解码后服务器所在位置
            //ChooseServer::printVector(ChooseServer::serverID);

            // 计算个体适应度
            if (!ChooseServer::serverID.empty()) {
                //ZKW ga_run;
                ga_run.run(Graph::nodeCount,Graph::arcCount,ChooseServer::serverID);
                population[i].fitness = ga_run.minicost;
            }
            else {
                // 没有服务节点的个体
                population[i].fitness = INF;
            }
        }
    }

    // ----已测试----
    // 根据适应度对个体进行排序，从小到大排序
    static bool fitness_sort(ga_struct x, ga_struct y) {
        return (x.fitness < y.fitness);
    }
    inline void sort_by_fitness(ga_vector &population) {
        std::sort(population.begin(), population.end(), fitness_sort);
    }

    // ----已测试----
    // 突变概率设计,不同阶层在不同时期的突破概率不同
    int mutate_design() {
        // ——————待测试——————
        int g = rand() % 100;

        if (g < high) {
            return 1;
        }
        else if (g >= high && g < middle) {
            return 2;
        }
        else {
            return 3;
        }
    }

    // ----已测试----
    // 突变操作（服务器）
    void mutate_server(ga_struct &member) {
        int choose = mutate_design();
        int ipos;
        if (choose == 1) {
            if (size_Id_server != 0) {
                ipos = rand() % size_Id_server;
            }
            else {
                ipos = 0;

                printf("May be something wrong!\n");
            }
        }
        else if (choose == 2) {
            if (size_Candidate_server != 0) {
                ipos = size_Id_server + rand() % size_Candidate_server;
            }
            else {
                ipos = rand() % size_Id_server;
            }
        }
        else {
            if (size_Possible_server != 0) {
                ipos = size_Id_server + size_Candidate_server + rand() % size_Possible_server;
            }
            else {
                ipos = rand() % (size_Id_server + size_Candidate_server);
            }
        }

        (member.str[ipos] == '1') ? (member.str[ipos] = '0') : (member.str[ipos] = '1');
    }

    // ----已测试----
    // 精英群体
    void elitism(ga_vector &population, ga_vector &buffer, int esize) {
        for (int i=0; i<esize; i++) {
            buffer[i].str = population[i].str;
            buffer[i].fitness = population[i].fitness;
        }
    }

    // ----已测试----
    // 交换操作（服务器）
    void mate_server(ga_vector &population, ga_vector &buffer) {
        int spos;
        int i1;
        int i2;

        // 精英群体（遗传）
        elitism(population, buffer, esize);

        // 与其它个体基因进行变异
        for (int i=esize; i<ga_size; i++) {
            i1 = rand() % ga_size;
            i2 = rand() % ga_size;
            spos = rand() % ga_target_size;

            // ----已测试----
            // 基因交换(拼接)
            buffer[i].str = population[i1].str.substr(0, spos) +
                            population[i2].str.substr(spos, ga_target_size - spos);
            buffer[i].fitness = INF;

            // 基因位变异
            if (rand() < ga_mutation) mutate_server(buffer[i]);
        }
    }

    // ----已测试----
    // 打印输出本次迭代最好的个体
    inline void print_best(ga_vector &gav) {
        std::cout << "Best: " << gav[0].str << " (" << gav[0].fitness << ")" << std::endl;
        ga_s.clear();
        ga_s = gav[0].str;
        ga_minicost = gav[0].fitness;
    }

    // ----已测试----
    // 基因解码
    void decode() {
        // 清空服务器节点放置缓存
        std::vector <int>().swap(ChooseServer::serverID);

        for (int i = 0; i < ga_s.size(); ++i) {
            if (ga_s[i] == '1') {
                ChooseServer::serverID.push_back(ga_server[i]);
            }
        }

        if (!ChooseServer::serverID.empty()) {
            //ZKW ga_run;
            ga_run.run(Graph::nodeCount,Graph::arcCount, ChooseServer::serverID);
            if (ga_run.minicost < INF) {
                write_result(ga_run.getRoute(),ga_filename);
            }
        }
        else {
            std::cout<<"Code is Error Error, Please check it!\n"<<std::endl;
        }
    }

    // ----已测试----
    // population等级概率渐变
    void population_change() {
        // 循环突变
        if (hm_flag) {
            if (ga_step % mutate_step == (mutate_step - 1)) {
                if (high < hm_line_high) {
                    high += 1;
                }
                if (middle > hm_line_middle) {
                    middle -= 1;
                }
            }

            if (high == hm_line_high && middle == hm_line_middle) {
                hm_flag = false;
            }
        }
        else {
            if (ga_step % mutate_step == (mutate_step - 1)) {
                if (high > high_line) {
                    high -= 1;
                }
                if (middle < middle_line) {
                    middle += 1;
                }
            }

            if (high == high_line && middle == middle_line) {
                hm_flag = true;
            }
        }
    }

    // ----已测试----
    // 精英率与变异率递增
    void decay(int step) {
        if ((step % decay_e_step) == (decay_e_step - 1)) {
            // 精英率递增
            if (ga_elitism_rate * (1 + decay_rate) < decay_e_rate) {
                ga_elitism_rate *= 1 + decay_rate;
            }
            // 将精英率转变为种群中精英个体数目
            esize = ceil(ga_size * ga_elitism_rate);

            // 变异率递增
            if (ga_mutation_rate * (1 + decay_rate) < decay_m_rate) {
                ga_mutation_rate *= 1 + decay_rate;
            }
            // 将变异率转变为real突变数
            ga_mutation = RAND_MAX * ga_mutation_rate;
        }
    }

    // ----已测试----
    // 交换父子群体
    void swap(ga_vector *&population, ga_vector *&buffer) {
        ga_vector *temp = population;
        population = buffer;
        buffer = temp;
    }

    // ----已测试----
    // 打印基因序列
    void PrintGA(ga_vector &population) {
        for (int i=0; i<ga_size; i++) {
            std::cout<<(population)[i].str<<"\t"<<(population)[i].fitness<<std::endl;
        }
    }

    bool GaAlgorithmServer() {
        // 用于产生伪随机数的时间种子
        srand(unsigned(time(NULL)));

        ga_vector pop_alpha, pop_beta;
        ga_vector *population, *buffer;

        init_population_server(pop_alpha, pop_beta);

        population = &pop_alpha;
        buffer = &pop_beta;

        int t0=clock();
        for (int i=0; i<ga_max_iterate; i++) {
            ga_step = i;

            // population等级概率渐变
            population_change();

            // 精英率与变异率衰减
            decay(ga_step);

            // 计算适应度
            calc_fitness_server(*population, i);

            // 对个体进行排序
            sort_by_fitness(*population);

            // 输出最好的个体
            print_best(*population);

            // ----已测试----
            if (((clock() - t0) > TIME_END) || (i == ga_max_iterate-1)) {
                decode();  // 基因解码
                std::cout<<"基因序列:"<<ga_s<<std::endl;
                std::cout<<"Ga_Mincost:"<<ga_minicost<<std::endl;

                return true;
            }

            // 个体基因交换
            mate_server(*population, *buffer);

            // 交换父子群体
            swap(population, buffer);
        }

        return 0;
    }
};



///**
// * 存储任意两点之间的距离和路径，Floyd算法
// * */
//int VertexCost[MAXNODE][MAXNODE];
//int VertexPath[MAXNODE][MAXNODE];
//std::vector<int> getPath;
//
//
//struct GA {
//    // Floyd计算任意两点之间的距离
//    int ComputeCost(Arc *gMatrix[][MAXNODE], int vexNum);
//
//    // 递归寻路
//    void  Prn_Pass(int j , int k);
//    // 获取两点之间的路径
//    bool GetPath(int i, int j);
//};
//
//
//int GA::ComputeCost(Arc *gMatrix[][MAXNODE], int vexNum) {
//    for (int i = 0;i < vexNum;i++) {
//        for (int j = 0;j < vexNum;j++) {
//            if (i == j) {
//                VertexCost[i][j] = 0;
//            }
//            else if(gMatrix[i][j] != nullptr) {
//                VertexCost[i][j] = (gMatrix[i][j]->capacity) * (gMatrix[i][j]->cost);
//            }
//            else {
//                VertexCost[i][j] = MAX_COST;
//            }
//            VertexPath[i][j] = -1;
//        }
//    }
//
//    //关键代码部分
//    for (int k = 0;k < vexNum;k++) {
//        for (int i = 0;i < vexNum;i++) {
//            for (int j = 0;j < vexNum;j++) {
//                if (VertexCost[i][k] + VertexCost[k][j] < VertexCost[i][j]) {
//                    VertexCost[i][j] = VertexCost[i][k] + VertexCost[k][j];
//                    VertexPath[i][j] = k;
//                }
//            }
//        }
//    }
//
//    return 0;
//}
//
//// 获取两点之间的路径
//void  GA::Prn_Pass(int j , int k) {
//    if (VertexPath[j][k]!=-1) {
//        Prn_Pass(j,VertexPath[j][k]);
//        std::cout<<"-->"<<VertexPath[j][k];
//        getPath.push_back(VertexPath[j][k]);
//        Prn_Pass(VertexPath[j][k],k);
//    }
//}
//
//bool GA::GetPath(int i, int j) {
//    std::cout<<i<<"到"<<j<<"的最短路径为:";
//    std::cout<<i;
//    Prn_Pass(i, j);
//    std::cout<<"-->"<<j<<std::endl;
//    std::cout<<"最短路径长度为:"<<VertexCost[i][j]<<std::endl;
//
//    if (!getPath.empty()) {
//        return false;
//    }
//
//    return true;
//}


#endif //CDN_GA_H