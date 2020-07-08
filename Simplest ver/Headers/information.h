#include <bits/stdc++.h>
#include "robin_hood.h"
using namespace std;

struct Srv
{
    int used;
    int size;
};
struct App
{
    vector<pair<string, list<Srv>::iterator>> srvs;
    int conn;
    double size;
};
struct Pilot
{
    robin_hood::unordered_set<string> srvs;
    robin_hood::unordered_set<string> apps;
    long long size;
    long long conn;
};

/*数据存放*/
robin_hood::unordered_map<string, list<Srv>::iterator> srv_mp;
list<Srv> srv_list;
robin_hood::unordered_map<string, App> app_mp;
robin_hood::unordered_map<string, Pilot> pilot_mp;
vector<string> app_list, pilot_list;
int pilot_cnt = 0;
//所有pilot的一次方和
long long pilot_sum_size = 0;
long long pilot_sum_conn = 0;
//二次方和
long long pilot_sum_size2 = 0;
long long pilot_sum_conn2 = 0;
//通过维护多项式来做到O(1)维护序列标准差

double cal_size_s2()
{
    double n = pilot_cnt;
    double avg = pilot_sum_size / n;
    return sqrt((pilot_sum_size2 + n * avg * avg - 2 * avg * pilot_sum_size) / n);
}
double cal_conn_s2()
{
    double n = pilot_cnt;
    double avg = pilot_sum_conn / n;
    return sqrt((pilot_sum_conn2 + n * avg * avg - 2 * avg * pilot_sum_conn) / n);
}
double cal_score() //计算分数 忽视比率 只考虑方差
{
    double D1 = cal_size_s2();
    double D2 = cal_conn_s2();
    return (D1 / 100 + D2);
}