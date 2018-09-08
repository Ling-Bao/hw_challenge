/*
 * Copyright 2017
 *
 * DirectOut.h
 *
 * 直接输出，测试用例
 *
 * Date: March 26, 2017
 * Author: Wei Liu, Ling Bao, Shangyue Zhou
 *
 * $Id: DirectOut.h,v 0.1 2017/03/26 00:22:35 lw Exp $
 *
 */

#ifndef CDN_DIRECTOUT_H
#define CDN_DIRECTOUT_H


struct DirectOUT {
    static char re[MAX_OUT_CHAR_NUM];
    static void outResult();
};
char DirectOUT::re[MAX_OUT_CHAR_NUM]{0};
void DirectOUT::outResult() {
    int offset{0};
    offset+=sprintf(re+offset,"%d\n\n",Graph::consumerCount);  // 格式化的数据写入字符串
    for(int i = 0;i<Graph::consumerCount;++i)
    {
        offset+=sprintf(re+offset,"%d %d %.f\n",Graph::consumerNode[i],i,Graph::netNode[ Graph::consumerNode[i] ]->require);  // 格式化的数据写入字符串

    }
    re[offset-1]='\0';
    std::cout<<re<<std::endl;
}


#endif //CDN_DIRECTOUT_H
