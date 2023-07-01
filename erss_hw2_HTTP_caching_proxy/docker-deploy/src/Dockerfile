FROM ubuntu:20.04
RUN apt-get update
RUN apt-get install -y g++ make
RUN mkdir /var/log/erss
RUN touch /var/log/erss/proxy.log
RUN chmod 777 /var/log/erss
RUN mkdir /code
ADD . /code/
WORKDIR /code
RUN chmod +x /code/run.sh
