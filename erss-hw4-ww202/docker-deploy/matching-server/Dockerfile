FROM ubuntu:20.04

ENV DEBIAN_FRONTEND noninteractive

RUN mkdir /code

# install dependencies
RUN apt-get update -y
RUN apt-get install g++ make -y
RUN apt-get install libpqxx-dev -y

WORKDIR /code
ADD . /code

RUN chmod +x runserver.sh
