FROM ubuntu:16.04
VOLUME /root/input

COPY ./somServ /root/
COPY ./run.sh /root/
WORKDIR /root
ENTRYPOINT ["sh","/root/run.sh"]