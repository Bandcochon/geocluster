FROM gcc:7

WORKDIR /app

RUN apt-get update && apt-get install git cmake -y

RUN git clone https://github.com/regisf/geocluster-c.git geocluster
RUN cd geocluster && cmake .
#RUN make
