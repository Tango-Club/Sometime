#include <bits/stdc++.h>
#include <ext/pb_ds/assoc_container.hpp>
#include <ext/pb_ds/hash_policy.hpp>
#include "http_server.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
#include "robin_hood.h"
using namespace rapidjson;
using namespace std;
using namespace __gnu_pbds;
// 初始化HttpServer静态类成员
mg_serve_http_opts HttpServer::s_server_option;
string HttpServer::s_web_dir = "./web";
unordered_map<string, ReqHandler> HttpServer::s_handler_map;
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
    robin_hood::unordered_map<string, int> srvs;
    robin_hood::unordered_set<string> apps;
    long long size;
    long long conn;
};
//数据存放
robin_hood::unordered_map<string, list<Srv>::iterator> srv_mp;
list<Srv> srv_list;
robin_hood::unordered_map<string, App> app_mp;
robin_hood::unordered_map<string, Pilot> pilot_mp;
vector<string> app_list, pilot_list;
//公共数据
long long pilot_sum_size = 0;
long long pilot_sum_size2 = 0;
long long pilot_sum_conn = 0;
long long pilot_sum_conn2 = 0;
long long sum_size = 0;
long long sum_conn = 0;
double rate = 100;
int state = 0;
int pilot_cnt = 0;
robin_hood::unordered_map<int, double> value_dict;
robin_hood::unordered_map<string, string> app_pilot;
default_random_engine e(time(0));
int rd(int l, int r)
{
    uniform_int_distribution<int> u(l, r);
    return u(e);
}
double rd(double l, double r)
{
    uniform_real_distribution<double> u(l, r);
    return u(e);
}
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
double cal_score(bool flag)
{
    double M1 = pilot_sum_size;
    double M2 = sum_size;
    double D1 = cal_size_s2();
    double D2 = cal_conn_s2();
    if (flag)
    {
        //cout << "ratio: " << M1 / M2 << " conn: " << D2 << " mem: " << D1 / 100 << endl;
        return M1 / M2 * (D1 / 100 + D2);
    }
    else
    {
        return (D1 + rate * D2);
    }
}

void re_alloc()
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
double add_app_fake(App &app, Pilot &pilot, string app_name)
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
        if (pilot.srvs[srv.first] == 0)
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
    double point = cal_score(false);

    pilot_sum_conn = o_pilot_sum_conn;
    pilot_sum_conn2 = o_pilot_sum_conn2;
    pilot.conn = o_pilot_conn_pilot;
    pilot_sum_size = o_pilot_sum_size;
    pilot_sum_size2 = o_pilot_sum_size2;
    pilot.size = o_pilot_size_pilot;

    return point;
}
void add_app(App &app, Pilot &pilot, string app_name)
{
    long long delta_pilot_size = 0;
    for (auto &srv : app.srvs)
    {
        if (pilot.srvs[srv.first] == 0)
        {
            delta_pilot_size += srv.second->size;
            srv.second->used++;
        }
        pilot.srvs[srv.first] += 1;
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
void sub_app(App &app, Pilot &pilot, string app_name)
{
    long long delta_pilot_size = 0;
    for (auto &srv : app.srvs)
    {
        pilot.srvs[srv.first] -= 1;
        if (pilot.srvs[srv.first] == 0)
        {
            delta_pilot_size += srv.second->size;
            srv.second->used--;
        }
    }

    pilot_sum_conn -= pilot.conn;
    pilot_sum_conn2 -= pilot.conn * pilot.conn;
    pilot.conn -= app.conn;
    pilot_sum_conn += pilot.conn;
    pilot_sum_conn2 += pilot.conn * pilot.conn;

    pilot_sum_size -= pilot.size;
    pilot_sum_size2 -= pilot.size * pilot.size;
    pilot.size -= delta_pilot_size;
    pilot_sum_size += pilot.size;
    pilot_sum_size2 += pilot.size * pilot.size;

    pilot.apps.erase(app_name);
}
void alloc_app(string app)
{
    App &app_inf = app_mp[app];
    string best_pilot = pilot_list[0];
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
    app_pilot[app] = best_pilot;
}
bool app_cmp(string x, string y)
{
    return app_mp[x].size > app_mp[y].size;
}
bool cmp_pair(pair<string, list<Srv>::iterator> x, pair<string, list<Srv>::iterator> y)
{
    return x.second->used < y.second->used;
}
void temp_alloc_apps(int rate_p, int block_p)
{
    rate = rate_p;
    int block = app_list.size() / block_p;
    for (size_t i = 0; i < app_list.size(); i++)
    {
        if (i % block == 0)
        {
            for (size_t j = i; j < app_list.size(); j++)
            {
                string app = app_list[j];
                app_mp[app].size = 0;
                for (auto &srv : app_mp[app].srvs)
                    app_mp[app].size += srv.second->size * value_dict[srv.second->used];
                app_mp[app].size = (app_mp[app].size + rate * app_mp[app].conn);
            }
            sort(app_list.begin() + i, app_list.end(), app_cmp);
        }
        alloc_app(app_list[i]);
    }
}
void alloc_apps(double time_limit)
{
    clock_t t = clock();
    double rd_rate, ld_rate, round_rate, temp;
    re_alloc();
    for (size_t j = 0; j < app_list.size(); j++)
    {
        string app = app_list[j];
        sort(app_mp[app].srvs.begin(), app_mp[app].srvs.end(), cmp_pair);
        while (!app_mp[app].srvs.empty() && app_mp[app].srvs.back().second->used == pilot_cnt)
            app_mp[app].srvs.pop_back();
    }
    if (state == 1)
    {
        temp_alloc_apps(200, 20);
        rd_rate = 0.0;
        ld_rate = 1.0;
        round_rate = 0.33;
        temp = 1;
    }
    if (state == 2)
    {
        temp_alloc_apps(120, 10);
        rd_rate = 0.7;
        ld_rate = 1.0;
        round_rate = 0.1;
        temp = 1;
    }

    double best_point = cal_score(true);
    cout << "start sa time: " << double(clock() - t) / CLOCKS_PER_SEC << endl;

    int p = 0;
    while (double(clock() - t) / CLOCKS_PER_SEC <= time_limit)
    {
        p++;
        if (state == 1)
        {
            if (p % 1000 == 0)
                temp *= 0.999;
        }
        if (state == 2)
        {
            if (p % 100 == 0)
                temp *= 0.999;
        }
        int x = rd(int(app_list.size() * rd_rate), int(app_list.size() * ld_rate) - 1);
        int y = rd(max(0, x - int(app_list.size() * round_rate)), min(x + int(app_list.size() * round_rate), int(app_list.size()) - 1));
        string app_name_x = app_list[x];
        string app_name_y = app_list[y];
        string pilot_name_x = app_pilot[app_name_x];
        string pilot_name_y = app_pilot[app_name_y];
        if (pilot_name_x == pilot_name_y)
            continue;
        sub_app(app_mp[app_name_x], pilot_mp[pilot_name_x], app_name_x);
        sub_app(app_mp[app_name_y], pilot_mp[pilot_name_y], app_name_y);
        add_app(app_mp[app_name_x], pilot_mp[pilot_name_y], app_name_x);
        add_app(app_mp[app_name_y], pilot_mp[pilot_name_x], app_name_y);
        double delta = best_point - cal_score(true);
        if (delta > 0 || rd(0.0, 1.0) < exp(delta / temp))
        {
            //cout << delta << " " << temp << " " << exp(delta / temp) << endl;
            swap(app_pilot[app_name_x], app_pilot[app_name_y]);
            best_point = cal_score(true);
            //cout << "time: " << double(clock() - t) / CLOCKS_PER_SEC << " score: " << cal_score(true) << endl;
        }
        else
        {
            sub_app(app_mp[app_name_x], pilot_mp[pilot_name_y], app_name_x);
            sub_app(app_mp[app_name_y], pilot_mp[pilot_name_x], app_name_y);
            add_app(app_mp[app_name_x], pilot_mp[pilot_name_x], app_name_x);
            add_app(app_mp[app_name_y], pilot_mp[pilot_name_y], app_name_y);
        }
    }
}
void read_data(Document &doc)
{
    Value &dep = doc["dependencies"];
    for (auto &app : dep.GetObject())
    {
        string app_name = app.name.GetString();
        app_mp[app_name].srvs.clear();
        for (auto &srv : app.value.GetObject())
        {
            string srv_name = srv.name.GetString();
            if (srv_mp.find(srv_name) == srv_mp.end())
            {
                for (auto &pilot : pilot_mp)
                    pilot.second.srvs[srv_name] = 0;
                int srv_size = srv.value.GetInt();
                sum_size += srv_size;
                srv_list.push_back({0, srv_size});
                srv_mp[srv_name] = --srv_list.end();
            }
            app_mp[app_name].srvs.push_back({srv_name, srv_mp[srv_name]});
        }
    }

    Value &apps = doc["apps"];
    for (auto &app : apps.GetObject())
    {
        string app_name = app.name.GetString();
        app_list.push_back(app_name);
        int app_conn = app.value.GetInt();
        sum_conn += app_conn;
        app_mp[app_name].conn = app_conn;
    }
    cout << "app: " << app_list.size() << endl;
}
string get_result()
{
    Document result(kObjectType);
    auto &allocator = result.GetAllocator();
    for (auto &pilot : pilot_list)
    {
        result.AddMember(Value().SetString(pilot.c_str(), pilot.length()), Value().SetArray(), allocator);
        Value &apps = result[pilot.c_str()];
        for (auto &app : pilot_mp[pilot].apps)
        {
            apps.PushBack(Value().SetString(app.c_str(), app.length()), allocator);
        }
    }
    StringBuffer buffer;
    Writer<StringBuffer> writer(buffer);
    result.Accept(writer);
    string res = buffer.GetString();
    return res;
}
bool ready(string url, string body, mg_connection *c, OnRspCallback rsp_callback)
{
    cout << "Phase Ready" << endl;
    rsp_callback(c, "ok");
    return true;
}
bool p1_start(string url, string body, mg_connection *c, OnRspCallback rsp_callback)
{
    cout << "Phase P1" << endl;
    state = 1;
    //read&init pilot
    Document doc;
    doc.Parse(body.c_str());
    Value &pilots = doc["pilots"];
    pilot_cnt = pilots.Size();
    cout << "pilot: " << pilot_cnt << endl;
    for (auto &pilot : pilots.GetArray())
    {
        string pilot_name = pilot.GetString();
        pilot_list.push_back(pilot_name);
    }
    for (int i = 0; i <= pilot_cnt; i++)
    {
        double x = 1 - 1.0 * i / pilot_cnt;
        value_dict[i] = x / 1.75;
    }
    //read json file
    ifstream in("./input/data.json");
    ostringstream buf;
    buf << in.rdbuf();
    string ss = buf.str();
    doc.Parse(ss.c_str());
    read_data(doc);
    //cal result
    alloc_apps(112);
    //build result
    rsp_callback(c, get_result().c_str());
    double M1 = pilot_sum_size;
    double M2 = sum_size;
    double D1 = cal_size_s2();
    double D2 = cal_conn_s2();
    cout << "ratio: " << M1 / M2 << " conn: " << D2 << " mem: " << D1 / 100 << endl;
    cout << "score " << cal_score(true) << endl;
    return true;
}
bool p2_start(string url, string body, mg_connection *c, OnRspCallback rsp_callback)
{
    cout << "Phase P2" << endl;
    state = 2;
    //read json
    Document doc;
    doc.Parse(body.c_str());
    read_data(doc);
    //cal result
    alloc_apps(12);
    //build result
    rsp_callback(c, get_result().c_str());
    double M1 = pilot_sum_size;
    double M2 = sum_size;
    double D1 = cal_size_s2();
    double D2 = cal_conn_s2();
    cout << "ratio: " << M1 / M2 << " conn: " << D2 << " mem: " << D1 / 100 << endl;
    cout << "score " << cal_score(true) << endl;
    return true;
}
int main(int argc, char *argv[])
{
    string port = "3355";
    auto http_server = shared_ptr<HttpServer>(new HttpServer);
    http_server->Init(port);
    http_server->AddHandler("/ready", ready);
    http_server->AddHandler("/p1_start", p1_start);
    http_server->AddHandler("/p2_start", p2_start);
    http_server->Start();
    return 0;
}