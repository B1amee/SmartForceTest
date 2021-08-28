FROM gcc:latest as build

ADD ./src src
ADD ./build build
ADD ./logs logs

RUN gcc src/main.c -o build/main

FROM ubuntu:latest

RUN mkdir /data
COPY --from=build /build/main .

ENTRYPOINT [ "./main" ]
