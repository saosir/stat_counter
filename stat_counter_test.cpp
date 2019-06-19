#include <assert.h>
#include <iostream>
#include <windows.h>
#include "stat_counter.h"

int time_average_event_test()
{
    // 窗口数2， 窗口大小2秒
    TimeBaseAverageEventCounter tbaec(2, 2);
    int seconds = 0;
    // 6秒时间每秒发生一次事件，每次值为2，由于窗口配置为2X2，有效秒数为4秒
    while (seconds < 6) {
        tbaec.event(2);
        Sleep(1000);
        seconds++;
    }
    assert(tbaec.sum() == 8);
    assert(tbaec.avg() == 2); // 适合用于计算平均udp包大小
    return 0;
}

int time_average_window_test()
{
    TimeBaseAverageWindowCounter tbawc(2, 2);
    int seconds = 0;
    while (seconds < 6) {
        tbawc.event(2);
        Sleep(1000);
        seconds++;
    }
    assert(tbawc.sum() == 8);
    assert(tbawc.avg() == 2); // 适合用于计算tcp流量速率

    return 0;
}

int main(int argc, char const *argv[])
{
    TotalCounter tc;
    assert(tc.empty());
    tc.event(10);
    tc.event(2);
    assert(tc.sum() == tc.avg());
    assert(tc.sum() == 12);
    assert(!tc.empty());

    AverageWindowCounter awc;
    assert(awc.empty());
    awc.event(10);
    awc.event(2);
    assert(awc.sum() == 12);
    assert(awc.avg() == 6);
    assert(!awc.empty());
    awc.event(18);
    assert(awc.sum() == 30);
    assert(awc.avg() == 10);

    TimeBaseAverageEventCounter tbaec;
    tbaec.event(10);
    tbaec.event(10);
    assert(tbaec.avg() == 10);
    assert(tbaec.sum() == 20);
    assert(!tbaec.empty());

    TimeBaseAverageWindowCounter tbawc(1, 10);
    tbawc.event(10);
    tbawc.event(10);
    assert(tbawc.avg() == 2);
    assert(tbawc.sum() == 20);
    assert(!tbawc.empty());

    time_average_event_test();
    time_average_window_test();
    return 0;
}
