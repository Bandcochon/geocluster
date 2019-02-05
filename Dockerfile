FROM debian:stretch

WORKDIR /app

COPY src src
COPY CMakeLists.txt .
COPY config.ini .

RUN apt-get update \
 && apt-get install build-essential cmake -y --no-install-recommends\
        libjansson4 \
        libjansson-dev \
        default-libmysqlclient-dev \
        libmariadbclient18 \
        pkgconf \
        libevent-2.0-5 \
        libevent-dev \
 && cmake . \
 && make \
 && rm -r src *.txt Makefile *.cmake \
 && apt-get autoremove -y \
    build-essential \
    cmake \
    libjansson-dev \
    default-libmysqlclient-dev \
    pkgconf \
    libevent-dev \
 && apt-get clean \
 && rm -r /var/lib/apt
   
    
ENV DEBUG=0

EXPOSE 5000

CMD [ "./geocluster", "-c", "config.ini" ]

