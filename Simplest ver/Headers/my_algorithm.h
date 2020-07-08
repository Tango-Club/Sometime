#include "information.h"
void re_alloc() //重新分配所有app
{
    pilot_sum_conn = 0;
    pilot_sum_conn2 = 0;
    for (auto &it : pilot_mp)
    {
        auto &pilot = it.second;
        pilot.conn = 0;
        pilot.apps.clear();
    }
}
double add_app_fake(App &app, Pilot &pilot, string app_name) //只计算分配后的分数
{
    long long o_pilot_sum_conn = pilot_sum_conn;
    long long o_pilot_sum_conn2 = pilot_sum_conn2;
    long long o_pilot_conn_pilot = pilot.conn;
    long long o_pilot_sum_size = pilot_sum_size;
    long long o_pilot_sum_size2 = pilot_sum_size2;
    long long o_pilot_size_pilot = pilot.size;

    int delta_pilot_size = 0;
    for (auto &srv : app.srvs)
    {
        if (!pilot.srvs.count(srv.first))
            delta_pilot_size += srv.second->size;
    }
    pilot_sum_conn -= pilot.conn;
    pilot_sum_conn2 -= pilot.conn * pilot.conn;
    pilot.conn += app.conn;
    pilot_sum_conn += pilot.conn;
    pilot_sum_conn2 += pilot.conn * pilot.conn;

    pilot_sum_size -= pilot.size;
    pilot_sum_size2 -= pilot.size * pilot.size;
    pilot.size += delta_pilot_size;
    pilot_sum_size += pilot.size;
    pilot_sum_size2 += pilot.size * pilot.size;
    double point = cal_score();

    pilot_sum_conn = o_pilot_sum_conn;
    pilot_sum_conn2 = o_pilot_sum_conn2;
    pilot.conn = o_pilot_conn_pilot;
    pilot_sum_size = o_pilot_sum_size;
    pilot_sum_size2 = o_pilot_sum_size2;
    pilot.size = o_pilot_size_pilot;

    return point;
}
void add_app(App &app, Pilot &pilot, string app_name) //真正分配app到pilot
{
    long long delta_pilot_size = 0;
    for (auto &srv : app.srvs)
    {
        if (!pilot.srvs.count(srv.first))
        {
            delta_pilot_size += srv.second->size;
            srv.second->used++;
            pilot.srvs.insert(srv.first);
        }
    }

    pilot_sum_conn -= pilot.conn;
    pilot_sum_conn2 -= pilot.conn * pilot.conn;
    pilot.conn += app.conn;
    pilot_sum_conn += pilot.conn;
    pilot_sum_conn2 += pilot.conn * pilot.conn;

    pilot_sum_size -= pilot.size;
    pilot_sum_size2 -= pilot.size * pilot.size;
    pilot.size += delta_pilot_size;
    pilot_sum_size += pilot.size;
    pilot_sum_size2 += pilot.size * pilot.size;

    pilot.apps.insert(app_name);
}

void alloc_app(string app) //枚举该app分配所有pilot 取分数最小的进行实际分配
{
    App &app_inf = app_mp[app];
    string best_pilot;
    double best_point = 1e18;
    for (auto &pilot : pilot_list)
    {
        Pilot &pilot_inf = pilot_mp[pilot];
        double point = add_app_fake(app_inf, pilot_inf, app);
        if (point < best_point)
        {
            best_point = point;
            best_pilot = pilot;
        }
    }
    add_app(app_inf, pilot_mp[best_pilot], app);
}

bool app_cmp(string x, string y)
{
    return app_mp[x].size > app_mp[y].size;
}
bool cmp_pair(pair<string, list<Srv>::iterator> x, pair<string, list<Srv>::iterator> y)
{
    return x.second->used < y.second->used;
}
void temp_alloc_apps()
{
    int block = app_list.size() / 10;
    for (size_t i = 0; i < app_list.size(); i++)
    {
        if (i % block == 0) //定期进行size更新
        {
            for (size_t j = i; j < app_list.size(); j++)
            {
                string app = app_list[j];
                sort(app_mp[app].srvs.begin(), app_mp[app].srvs.end(), cmp_pair);
                while (!app_mp[app].srvs.empty() && app_mp[app].srvs.back().second->used == pilot_cnt) //去除已经被所有pilot负载过的srv
                    app_mp[app].srvs.pop_back();
                app_mp[app].size = 0;
                for (auto &srv : app_mp[app].srvs)
                    app_mp[app].size += srv.second->size;
                app_mp[app].size = (app_mp[app].size + 100 * app_mp[app].conn);
            }
            sort(app_list.begin() + i, app_list.end(), app_cmp);
        }
        alloc_app(app_list[i]);
    }
}
void alloc_apps()
{
    re_alloc(); //每个阶段重新分配所有app
    temp_alloc_apps();
}