FROM conanio/gcc10 as builder

COPY src src
COPY CMakeLists.txt .
COPY conanfile.txt .

RUN conan install .
RUN cmake .
RUN cmake --build .

FROM ubuntu:22.04
WORKDIR /app
COPY --from=builder --chown=root:root /home/conan/bin/geocluster /app/geocluster
COPY config.ini .

ENV DEBUG=0
EXPOSE 5000

CMD [ "/app/geocluster", "-c", "/app/config.ini" ]

