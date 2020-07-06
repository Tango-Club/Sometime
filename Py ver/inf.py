import math
pilot_list = []  # pilot列表

pilot_srv = {}  # pilot服务列表
srv_size = {}  # srv的内存占用
srv_used = {}  # srv被安装的次数

app_inf = {}
result = {}  # 返回结果

sum_size = 0  # app的srv总占用

pilot_size = {}  # pilot内存
pilot_sum_size = 0  # 内存一次方和
pilot_sum_size2 = 0  # 内存二次方和

pilot_conn = {}  # pilot已连接数
pilot_sum_conn = 0  # 连接一次方和
pilot_sum_conn2 = 0  # 连接二次方和

value_dic = {}

state = 0
rate = 100


def re_aloc(flag=False):
    global pilot_conn, pilot_sum_conn, pilot_sum_conn2, result, pilot_size, pilot_sum_size, pilot_sum_size2
    pilot_sum_conn = 0
    pilot_sum_conn2 = 0
    for pilot in pilot_list:
        result[pilot] = []
        pilot_conn[pilot] = 0
    if flag:
        for pilot in pilot_list:
            pilot_srv[pilot].clear()
            pilot_conn[pilot] = 0
            pilot_size[pilot] = 0
            pilot_size[pilot] = 0
        pilot_sum_size = 0
        pilot_sum_size2 = 0
        for srv in srv_used:
            srv_used[srv] = 0


def cal_size_s2():
    '''
    内存方差计算
    '''
    n = len(pilot_list)
    avg = pilot_sum_size / n
    return math.sqrt((pilot_sum_size2+n*avg*avg-2*avg*pilot_sum_size)/n)


def cal_conn_s2():
    '''
    连接方差计算
    '''
    n = len(pilot_list)
    avg = pilot_sum_conn / n
    return math.sqrt((pilot_sum_conn2+n*avg*avg-2*avg*pilot_sum_conn)/n)


def cal_score(flag=False):
    '''
    计算得分，越低越好。
    '''
    M1 = pilot_sum_size  # M加载内存
    M2 = sum_size  # M服务内存
    D1 = cal_size_s2()  # 内存标准差
    D2 = cal_conn_s2()  # 连接标准差
    if flag:
        return M1/M2*(D1/100+D2)
    if M2 == 0:
        return 0
    elif state == 1:
        return (D1+rate*D2)
    else:
        return (D1+rate*D2)
