/*
 * Copyright 2017
 *
 * 计时器
 *
 * 功能入口
 *
 * Date: March 26, 2017
 * Author: Wei Liu, Ling Bao, Shangyue Zhou
 *
 * $Id: TimeCalculator.h,v 0.1 2017/03/26 00:22:35 zs Exp $
 *
 */
#ifndef DATABASE_TIMECALCULATOR_H_
#define DATABASE_TIMECALCULATOR_H_

#include <sys/time.h>
#include <iostream>
using namespace std;

class TimeCalculator {
    public:
        TimeCalculator();
        explicit TimeCalculator(unsigned long long ms);
        ~TimeCalculator();

        void Begin();
        void End();
        unsigned long long us();
        double ms();
        double s();

        unsigned long long Now();
        bool TimeOut();
        void set_exit_time(unsigned long long ms) {
            exit_time_ = ms;
        }
    private:
        timeval begin_;
        timeval end_;

        //How long the Timer exit(Unit: ms)
        unsigned long long exit_time_;
};
TimeCalculator::TimeCalculator() {
}

TimeCalculator::TimeCalculator(unsigned long long ms) : exit_time_(ms) {
}

TimeCalculator::~TimeCalculator() {
}

void TimeCalculator::Begin() {
    gettimeofday(&begin_, NULL);
    //Avoid end_ - begin_ < 0
    gettimeofday(&end_, NULL);
}

void TimeCalculator::End() {
    gettimeofday(&end_, NULL);
}

unsigned long long TimeCalculator::us() {
    unsigned long long process_time =
            (end_.tv_sec - begin_.tv_sec) * 1000000 + (end_.tv_usec - begin_.tv_usec);
    return process_time;
}

double TimeCalculator::ms() {
    unsigned long long process_time =
            (end_.tv_sec - begin_.tv_sec) * 1000000 + (end_.tv_usec - begin_.tv_usec);
    double result = process_time / 1000.0;
    return result;
}

double TimeCalculator::s() {
    unsigned long long process_time =
            (end_.tv_sec - begin_.tv_sec) * 1000000 + (end_.tv_usec - begin_.tv_usec);
    double result = process_time / 1000000.0;
    return result;
}

unsigned long long TimeCalculator::Now() {
    timeval now_time;
    gettimeofday(&now_time, NULL);
    return (now_time.tv_sec * 1000000 + now_time.tv_usec);
}

bool TimeCalculator::TimeOut() {
    this->End();
    if (this->ms() >= exit_time_)
        return true;
    else
        return false;
}

#endif
