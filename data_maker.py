import random
import json

dic = {}
dic["dependencies"] = {}

dic["apps"] = {}
appnum = random.randint(1000, 1200)
srvnum = random.randint(80000, 90000)
srv_size = {}
id = 0
for j in range(srvnum):
    srv = "testsvc-" + str(j)
    p = random.randint(1, 10)
    if p < 2:
        srv_size[srv] = random.randint(5000, 50000)
    elif p < 5:
        srv_size[srv] = random.randint(50, 5000)
    else:
        srv_size[srv] = random.randint(1, 50)

for i in range(appnum):
    app = "testapp-" + str(id)
    id += 1
    dic["dependencies"][app] = {}
    p = random.randint(1, 10)
    if p < 2:
        dic["apps"][app] = random.randint(3000, 30000)
    elif p < 5:
        dic["apps"][app] = random.randint(30, 3000)
    else:
        dic["apps"][app] = random.randint(1, 30)
    num = random.randint(1, 500)
    st = set()
    for j in range(num):
        srv = "testsvc-" + str(random.randint(0, srvnum - 1))
        if srv not in st:
            st.add(srv)
            dic["dependencies"][app][srv] = srv_size[srv]

with open("./data/data.json", "w") as f:
    tmp = json.dumps(dic)
    f.write(tmp)
for t in range(8):
    appnum = random.randint(5, 20)
    dic.clear()
    dic["dependencies"] = {}
    dic["apps"] = {}
    for i in range(appnum):
        app = "testapp-" + str(id)
        id += 1
        dic["dependencies"][app] = {}
        p = random.randint(1, 10)
        if p < 2:
            dic["apps"][app] = random.randint(300, 30000)
        elif p < 5:
            dic["apps"][app] = random.randint(30, 300)
        else:
            dic["apps"][app] = random.randint(1, 30)
        num = random.randint(1, 500)
        st = set()
        for j in range(num):
            srv = "testsvc-" + str(random.randint(0, srvnum - 1))
            if srv not in st:
                st.add(srv)
                dic["dependencies"][app][srv] = srv_size[srv]
    with open("./data/00{}.json".format(t+1), "w") as f:
        tmp = json.dumps(dic)
        f.write(tmp)
dic.clear()
dic["pilots"] = []
for i in range(100):
    dic["pilots"].append("p{}".format(i))
with open("./data/pilots.json", "w") as f:
    tmp = json.dumps(dic)
    f.write(tmp)
