# Sometime
首届云原生编程挑战赛3：服务网格控制面分治体系构建 参赛作品

思路简述：
用一些加权计算每个app的权重，从大到小进行分配，中间由于srv的分配会导致权重变化，每过一段时间进行重新排序。
每次分配app时，枚举所有pilot，选择一个分配后score最小的进行分配。
后面用cpp重写了代码，加入了模拟退火来随机交换两个app所属的pilot，这部分最后由于没有提交次数所以参数没有调整的比较好，可能效果不太明显。
