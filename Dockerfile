FROM ubuntu:16.04

USER syncaide

COPY build/Release/bin /usr/local

RUN syncaide --help && syncaided --help