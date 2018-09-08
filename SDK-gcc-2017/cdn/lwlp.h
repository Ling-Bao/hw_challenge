/*
 * Copyright 2017
 *
 * lwlp.h
 *
 * 线性规划
 *
 * Date: March 26, 2017
 * Author: Wei Liu, Ling Bao, Shangyue Zhou
 *
 * $Id: lwlp.h,v 0.1 2017/03/26 00:30:35 lw Exp $
 *
 */

#ifndef CDN_LWLP_H
#define CDN_LWLP_H


struct LinearRe {
    std::vector<double> y_;//单纯形算子
    std::vector<std::vector<double>> B_;//B逆，初始为单位矩阵，中间变换矩阵
    std::vector<double> Z_N;//用于存放检验数
    std::vector<double> DetaN;//中间算子
    //std::vector<double> numB;//基向量相对位置对应编号
    //std::vector<double> numN;//非基向量对应编号
    std::vector<int> numVar;//基变量与非基变量，对应实际的位置编号，开始时前h个是基变量
    std::vector<double> inVarFactor;//换入变量的系数计算
    std::vector<std::vector<double >> *mc;
    std::vector<double> *co;
    int h,l,s;
    int min_in_loc{-1};
    std::vector<int> Z_N_To_RLoc;
    double minAlpha1{0.0};
    double minAlpha{0.0};
    double object{0};
    //小入基大出基,对于min标准型
    int inNum{-1};//换入列变量编号（在非基中的相对编号）
    int outNum{-1};//换出行变量编号（在基中位置，相对编号），可以通过numB也转成列变量编号

    void run();
    void y_Cal();
    bool z_NCal();
    void updateFactor();
    void chooseOut();
    void updateResource();
    void updateB_();
    void init();
    void colDetaN();
    void updateZ_N();
    bool chooseBaseCol();
    LinearRe(std::vector<std::vector<double >> *matrixC,std::vector<double> *vectorO){

        mc=matrixC;
        co=vectorO;
        h=(signed)matrixC->size();
        l=(signed)(*matrixC)[0].size();
        s=(signed)vectorO->size();
        //printf("h=%d,l=%d,s=%d\n",h,l,s);
        numVar.resize(l);
        //最后一列为资源向量
        if(l!=s+1){
            std::cout<<"l="<<l<<",s="<<s<<", input error!"<<std::endl;
            exit(10);
        }
        y_.resize(h);
        B_.resize(h);
        Z_N.resize(h);
        Z_N_To_RLoc.resize(h);
        inVarFactor.resize(h);
        for(int i=0;i<h;++i){
            B_[i].resize(h);
            B_[i][i]=1;
            Z_N_To_RLoc[i]=-1;
        }
        for(int i=0;i< l;++i){
            numVar[i]=i;
        }

    }

};

void LinearRe::init() {
    //计算单纯形算子
    y_Cal();
    //计算初始检验数
    z_NCal();
}
void LinearRe::run(){
    while(1){
        //确定主元列标
        if(chooseBaseCol()==true){
            break;
        }

        //更新系数
        updateFactor();
        //换出变量
        chooseOut();
        //更新资源向量，计算目标值
        updateResource();
        //计算DetaN
        colDetaN();
        //更新检验数
        updateZ_N();
        //更新B_
        updateB_();
    }


}

//0、单纯形算子
// B_T:B 逆的转置
//CB:基变量对价值系统
//用B_T的每行乘以CB的每列
//y_=B_T*CB
void LinearRe::y_Cal(){
    for(int j=0;j<h;++j){
        double s=0;
        for(int i=0;i<h;++i){
            s=s+B_[i][j]*(*co)[numVar[i] ];
        }
        y_[j]=s;
    }
}

//0、检验数，选入基变量
// Z_N=CN-NTY_，Z_N为检验数
//C0N表示原始的输入目标向量,其从k之后开始才是CN
//NT是我们需要的，输入N要用其转置，注意标号
//计算检验数，并选出要换入的标号，若检验数都大于等于零则停止（获得最优解）
bool LinearRe::z_NCal(){
    double checkNum{0.0};
    //int inNumtmp{-1};
    for(int j=h;j<s;++j){
        double t=0;
        for(int i=0;i<h;++i){
            t=t+(*mc)[i][numVar[j]]*y_[i];
        }
        //选择检验数最小的，记录其位置
        checkNum=(*co)[numVar[j] ]-t;
        Z_N.push_back(checkNum);
        Z_N_To_RLoc.push_back(numVar[j]);

    }
    return false;

}
//1、在非基变量选择换入基的变量编号
bool LinearRe::chooseBaseCol() {
    double min=1.0e6;
    int loc{-1};
    for(int i=0;i<h;++i){
        if(Z_N[i]<min){
            min=Z_N[i];
            min_in_loc=i;
            loc=Z_N_To_RLoc[i];
        }
    }
    if(min>=0){
        printf("lp succceed to gain the selected server!\n");
#ifdef WEDEBUG
        for(int i=0; i<h;++i){
            printf("val_%d =%.2f\n",numVar[i],(*mc)[i][l-1]);
        }
#endif
        return true;
    }
    minAlpha=min;
    inNum=loc;
    return false;
}
//2、
// 计算变换后的矩阵要换入基的那一列的系数计算
void LinearRe::updateFactor(){
    if(inNum==-1){
        std::cout <<"updateFactor error!"<<std::endl;
        exit(2);
    }
    bool geZeroflag{false};
    std::vector<double> newCol;
    for(int i=0;i<h;++i){
        double s=0;
        for(int j=0;j<h;++j){
            s=s+B_[i][j]* (*mc)[j][numVar[inNum] ];
        }
        if(s>0){geZeroflag=true;}
        newCol.push_back(s);
    }
    if(!geZeroflag){
        std::cout<<"no bound question!"<<std::endl;
        exit(3);
    }
    //将计算得到的值赋给inVarFactor
    for(int i=0;i<h;++i){
        //用于后面计算资源向量
        inVarFactor[i]=newCol[i];
    }
}
//3、确定换出基
void LinearRe::chooseOut(){
    double min1{1.0e6};
    double tmp{0};
    int outNumTmp=-1;
    for(int i=0;i<h;++i){
        if(inVarFactor[i]>0){
            tmp=(*mc)[i][l-1]/inVarFactor[i];

            if(tmp<min1){
                min1=tmp;
                outNumTmp=i;
            }
        }
    }
    if(outNumTmp==-1){
        std::cout<<"Find out var error!"<<std::endl;
        exit(20);
    }
    outNum=outNumTmp;
    minAlpha1=min1;
}
//4、更新资源向量,计算目标值，更换基变量位置并记录
void LinearRe::updateResource(){
    for(int i=0;i<h;++i){
        (*mc)[i][l-1]=(*mc)[i][l-1]-minAlpha1*(*mc)[i][numVar[inNum] ];
         //std::cout<<"mc: "<<(*mc)[i][l-1]<<std::endl;
    }
    //(*mc)[numVar[outNum]][l-1]=minAlpha1;
    (*mc)[outNum][l-1]=minAlpha1;
    object=object+minAlpha1*minAlpha;
    int tmp;
    tmp=numVar[outNum];
    numVar[outNum]=numVar[inNum];
    numVar[inNum]=tmp;
}
//5、计算DetaN
void LinearRe::colDetaN() {
    std::vector<double> v;
    for(int i=0;i<h;++i){
        v.push_back(B_[outNum][i]);
    }
    for(int i=h;i<s;++i){
        double tmp{0.0};
        for(int j=0;j<h;++j){
            tmp=tmp+(*mc)[j][ numVar[i] ]*v[j];
        }
        DetaN.push_back(tmp);
    }
}
//6、更新检验数
void LinearRe::updateZ_N() {
    double Beta{0.0};
    Beta=-(Z_N[min_in_loc]/inVarFactor[min_in_loc]);
    for(int i=0;i<h;++i){
        //Beta.push_back(-(Z_N[i]/inVarFactor[i]));
        Z_N[i]=Z_N[i]+Beta*DetaN[i];
    }
}
//7、更新B_,矩阵初等行变换，由换出位置知道哪个是主元，由换进位置知道对哪列进行单位化
void LinearRe::updateB_(){
    double k;
    k=1.0/inVarFactor[outNum];
    if(k!=1){
        for(int i=0;i<h;++i)
        {
            inVarFactor[i]=-k*inVarFactor[i];
        }
    }
    //上面全部转化负的，现在转换过来
    inVarFactor[outNum]=-inVarFactor[outNum];

    //左乘以B_进行行变换
    for(int i=0;i<h;++i){
        if(i!=outNum){
            if(inVarFactor[i]!=0){
                //把invarFactor[i]倍的第outNum行加到第i行
                for(int j=0;j<h;++j){
                    B_[i][j]=B_[i][j]+inVarFactor[i]*B_[outNum][j];
                }
            }
        }
    }
    if(k!=1){
        for(int j=0;j<h;++j){
            B_[outNum][j]=k*B_[outNum][j];
        }

    }
    //清空以免影响后面
    inVarFactor.clear();

}


#endif //CDN_LWLP_H


