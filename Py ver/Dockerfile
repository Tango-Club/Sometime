
FROM python:3.8.3

VOLUME /root/input

COPY ./test.py /root/
COPY ./inf.py /root/
COPY ./requirements.txt /root/
COPY ./run.sh /root/
WORKDIR /root
RUN pip3 install -r requirements.txt -i http://pypi.douban.com/simple/ --trusted-host pypi.douban.com

ENTRYPOINT ["sh","/root/run.sh"]


#pip freeze >requirements.txt
#docker build -t registry-vpc.cn-hangzhou.aliyuncs.com/somethings/sometime .
#docker push registry-vpc.cn-hangzhou.aliyuncs.com/somethings/sometime
#docker run -v /root/program/something/som-proj/input:/root/input registry-vpc.cn-hangzhou.aliyuncs.com/somethings/sometime 
#/root/program/something/som-proj/data/input
#docker kill $(sudo docker ps -a -q)
#docker rmi -f `docker images | grep  "<none>" | awk "{print $3}"`
#docker run --net=container:$(docker ps -q) -v /root/program/something/som-proj/input:/root/input -v /root/program/something/som-proj/data:/root/data -v /root/program/something/som-proj/output:/root/output registry-vpc.cn-hangzhou.aliyuncs.com/alicontest/grading