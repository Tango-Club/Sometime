
from fastapi import FastAPI
from pydantic import BaseModel
import random
import uvicorn
import json
import os
import inf
import math
import time
import copy
app = FastAPI()


class p1_Item(BaseModel):
    pilots: list


class p2_Item(BaseModel):
    dependencies: dict
    apps: dict


class App:
    def __init__(self, name, connections, size, srvs):
        self.name = name
        self.connections = connections
        self.size = size
        self.srvs = srvs


def add_app(app, pilot, flag):
    if not flag:
        o_pilot_sum_conn = inf.pilot_sum_conn
        o_pilot_sum_conn2 = inf.pilot_sum_conn2
        o_pilot_conn_pilot = inf.pilot_conn[pilot]
        o_pilot_sum_size = inf.pilot_sum_size
        o_pilot_sum_size2 = inf.pilot_sum_size2
        o_pilot_size_pilot = inf.pilot_size[pilot]

    inf.pilot_sum_conn -= inf.pilot_conn[pilot]
    inf.pilot_sum_conn2 -= inf.pilot_conn[pilot]**2
    inf.pilot_conn[pilot] += app.connections
    inf.pilot_sum_conn += inf.pilot_conn[pilot]
    inf.pilot_sum_conn2 += inf.pilot_conn[pilot]**2
    delta_pilot_size = 0
    for srv in app.srvs:
        if srv not in inf.pilot_srv[pilot]:
            delta_pilot_size += inf.srv_size[srv]
            if flag:
                inf.srv_used[srv] += 1
                inf.pilot_srv[pilot].add(srv)
    inf.pilot_sum_size -= inf.pilot_size[pilot]
    inf.pilot_sum_size2 -= inf.pilot_size[pilot]**2
    inf.pilot_size[pilot] += delta_pilot_size
    inf.pilot_sum_size += inf.pilot_size[pilot]
    inf.pilot_sum_size2 += inf.pilot_size[pilot]**2
    point = inf.cal_score()
    if flag:
        inf.result[pilot].append(app.name)
    else:
        inf.pilot_sum_conn = o_pilot_sum_conn
        inf.pilot_sum_conn2 = o_pilot_sum_conn2
        inf.pilot_conn[pilot] = o_pilot_conn_pilot
        inf.pilot_sum_size = o_pilot_sum_size
        inf.pilot_sum_size2 = o_pilot_sum_size2
        inf.pilot_size[pilot] = o_pilot_size_pilot
    return point


def alloc_app(app):
    best_pilot = inf.pilot_list[0]
    best_point = add_app(app, best_pilot, False)
    for pilot in inf.pilot_list:
        point = add_app(app, pilot, False)
        if point < best_point:
            best_point = point
            best_pilot = pilot
    add_app(app, best_pilot, True)


all_apps = []


def alloc_apps_p2(apps):
    global all_apps
    inf.re_aloc()
    all_apps += apps

    block = len(all_apps)/10
    for i in range(len(all_apps)):
        if i % block == 0:
            for j in range(i, len(all_apps)):
                all_apps[j].size = 0
                for srv in list(all_apps[j].srvs):
                    all_apps[j].size += inf.srv_size[srv] * \
                        inf.value_dic[inf.srv_used[srv]]
                    if inf.srv_used[srv] == len(inf.pilot_list):
                        all_apps[j].srvs.pop(srv)
            all_apps[i:len(all_apps)] = sorted(all_apps[i:len(all_apps)], key=lambda app:
                                               (inf.rate * app.connections + app.size)*random.uniform(1, 1+i/len(all_apps)), reverse=True)
        alloc_app(all_apps[i])
    print("p2", inf.cal_score(True))


def alloc_apps_p1(apps):
    t = time.time()
    global all_apps
    inf.re_aloc()
    all_apps += apps

    best_point = None
    best_result = None

    while time.time() - t < 100:
        #print(time.time() - t, best_point)
        inf.rate = random.randint(190, 210)
        temp_alloc_apps()
        print(inf.rate, inf.cal_score(True))
        if best_point is None or best_point > inf.cal_score(True):
            best_point = inf.cal_score(True)
            best_result = copy.deepcopy(inf.result)
    inf.re_aloc(True)
    for pilot in best_result:
        print(pilot, len(best_result[pilot]))
        for app in best_result[pilot]:
            add_app(inf.app_inf[app], pilot, True)
    print("p1", inf.cal_score(True))


def temp_alloc_apps():
    inf.re_aloc(True)
    block = len(all_apps)/10
    for i in range(len(all_apps)):
        if i % block == 0:
            for j in range(i, len(all_apps)):
                all_apps[j].size = 0
                for srv in list(all_apps[j].srvs):
                    all_apps[j].size += inf.srv_size[srv] * \
                        inf.value_dic[inf.srv_used[srv]]
            all_apps[i:len(all_apps)] = sorted(all_apps[i:len(all_apps)], key=lambda app:
                                               (inf.rate * app.connections + app.size) * random.uniform(1, 1 + i / len(all_apps)), reverse=True)
        alloc_app(all_apps[i])


def change_data(apps_inf):
    apps = []
    for app in apps_inf["apps"]:
        size = 0
        for srv in apps_inf["dependencies"][app]:
            if srv not in inf.srv_size:
                inf.sum_size += apps_inf["dependencies"][app][srv]
                inf.srv_used[srv] = 0
            inf.srv_size[srv] = apps_inf["dependencies"][app][srv]
            size += inf.srv_size[srv]
        inf.app_inf[app] = App(app, apps_inf["apps"][app],
                               size, apps_inf["dependencies"][app])
        apps.append(inf.app_inf[app])

    return apps


@app.head('/ready')
def ready():
    print("ready")


@app.post('/p1_start/')
def p1_start(item: p1_Item):
    inf.state = 1
    inf.rate = 200
    with open('./input/data.json') as f:
        jData = json.load(f)

    inf.pilot_list = item.pilots
    for pilot in inf.pilot_list:
        inf.result[pilot] = []
        inf.pilot_srv[pilot] = set()
        inf.pilot_conn[pilot] = 0
        inf.pilot_size[pilot] = 0
    for i in range(len(inf.pilot_list) + 1):
        x = 1-i / len(inf.pilot_list)
        inf.value_dic[i] = x/1.75
    apps_inf = {}
    for k, v in jData.items():
        apps_inf[k] = v

    apps = change_data(apps_inf)
    alloc_apps_p1(apps)

    return inf.result


@ app.post('/p2_start/')
def p2_start(item: p2_Item):
    inf.state = 2
    inf.rate = 120
    apps_inf = {}
    apps_inf["apps"] = item.apps
    apps_inf["dependencies"] = item.dependencies

    apps = change_data(apps_inf)
    alloc_apps_p2(apps)

    return inf.result


uvicorn.run(app=app, host="0.0.0.0", port=3355, workers=1)
