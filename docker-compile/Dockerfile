# micro-helix powered container image.
# @author Fabio Greotti <f.greotti@redkoibox.it>
FROM ubuntu:16.04
LABEL maintainer="Fabio Greotti <f.greotti@redkoibox.it>"
RUN apt-get update && apt-get install -y curl bzip2 build-essential
RUN curl -O -L https://dl.bintray.com/boostorg/release/1.67.0/source/boost_1_67_0.tar.bz2 && tar xjvf boost_1_67_0.tar.bz2
RUN cd boost_1_67_0 && ./bootstrap.sh && ./b2 install
RUN apt-get install -y libssl-dev liblua5.3-dev cmake libsasl2-dev pkg-config
RUN curl -O -L https://github.com/mongodb/mongo-c-driver/releases/download/1.9.4/mongo-c-driver-1.9.4.tar.gz && tar xzvf mongo-c-driver-1.9.4.tar.gz && cd mongo-c-driver-1.9.4/ && ./configure --disable-automatic-init-and-cleanup && make && make install
RUN mkdir -p /tmp/microhelix && mkdir -p /tmp/builds/microhelix
WORKDIR /tmp/microhelix
ENTRYPOINT [ "/bin/bash" ]