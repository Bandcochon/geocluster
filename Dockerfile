FROM debian:stretch

WORKDIR /app

COPY src src
COPY CMakeLists.txt .
COPY config.ini .

RUN apt-get update && \
    apt-get install build-essential cmake -y libjansson-dev libjansson4 default-libmysqlclient-dev pkgconf libevent-dev --no-install-recommends && \
    cmake . && make && rm -r src && apt-get autoremove build-essential cmake -y

ENV DEBUG=1

EXPOSE 5000

CMD [ "./geocluster", "-c", "config.ini" ]

