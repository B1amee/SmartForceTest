FROM gcc:latest as build

ADD ./src src
ADD ./build build
ADD ./logs logs

RUN gcc src/main.c -o build/main

FROM ubuntu:latest

# RUN groupadd -r sample && useradd -r -g sample sample
# USER sample

COPY --from=build /build/main .
# COPY --from=build /logs .

# ENTRYPOINT [ "./main" ]
