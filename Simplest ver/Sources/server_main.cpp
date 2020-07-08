#include "http_server.h"
#include "my_algorithm.h"
#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/stringbuffer.h"
using namespace rapidjson;
mg_serve_http_opts HttpServer::s_server_option;
string HttpServer::s_web_dir = "./web";
unordered_map<string, ReqHandler> HttpServer::s_handler_map;

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
                int srv_size = srv.value.GetInt();
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
    rsp_callback(c, "ok");
    return true;
}
bool p1_start(string url, string body, mg_connection *c, OnRspCallback rsp_callback)
{
    //read data
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
    ifstream in("./input/data.json");
    ostringstream buf;
    buf << in.rdbuf();
    string ss = buf.str();
    doc.Parse(ss.c_str());
    read_data(doc);
    //cal result
    alloc_apps();
    rsp_callback(c, get_result().c_str());
    cout << "score:" << cal_score() << endl;
    return true;
}
bool p2_start(string url, string body, mg_connection *c, OnRspCallback rsp_callback)
{
    //read data
    Document doc;
    doc.Parse(body.c_str());
    read_data(doc);
    //cal result
    alloc_apps();
    rsp_callback(c, get_result().c_str());
    cout << "score:" << cal_score() << endl;
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