FROM conanio/gcc9 as builder

USER conan

COPY src src
COPY CMakeLists.txt .
COPY conanfile.txt .

RUN conan install .
RUN cmake .
RUN cmake --build .

FROM debian:stretch
WORKDIR /app
COPY --from=builder /home/conan/bin/geocluster /app/geocluster
COPY config.ini .

ENV DEBUG=0
EXPOSE 5000

CMD [ "./geocluster", "-c", "config.ini" ]

