FROM ubuntu:16.04

ENV GOSU_VERSION=1.10
RUN set -x && \
    apt-get update && \
    apt-get install -y --no-install-recommends ca-certificates wget && \
    rm -rf /var/lib/apt/lists/* && \
    GOSU_DPKG_ARCH="$(dpkg --print-architecture | awk -F- '{ print $NF }')" && \
	wget -O /usr/local/bin/gosu "https://github.com/tianon/gosu/releases/download/${GOSU_VERSION}/gosu-${GOSU_DPKG_ARCH}" && \
	wget -O /usr/local/bin/gosu.asc "https://github.com/tianon/gosu/releases/download/${GOSU_VERSION}/gosu-${GOSU_DPKG_ARCH}.asc" && \
	export GNUPGHOME="$(mktemp -d)" && \
	gpg --keyserver ha.pool.sks-keyservers.net --recv-keys B42F6819007F00F88E364FD4036A9C25BF357DD4 && \
	gpg --batch --verify /usr/local/bin/gosu.asc /usr/local/bin/gosu && \
	rm -r "${GNUPGHOME}" /usr/local/bin/gosu.asc && \
	chmod +x /usr/local/bin/gosu && \
	gosu nobody true && \
	apt-get purge -y --auto-remove ca-certificates wget

ENV CMAKE_VERSION=3.10 \
    CMAKE_BUILD=1
RUN apt-get update && \
    apt-get install -y curl && \
    curl -sL https://deb.nodesource.com/setup_8.x | bash - && \
    apt-get install -y \
        nodejs \
        wget \
        openssl \
        libssl-dev \
        build-essential && \
    cd /tmp && \
    wget https://cmake.org/files/v${CMAKE_VERSION}/cmake-${CMAKE_VERSION}.${CMAKE_BUILD}.tar.gz && \
    tar -xzvf cmake-${CMAKE_VERSION}.${CMAKE_BUILD}.tar.gz && \
    cd cmake-${CMAKE_VERSION}.${CMAKE_BUILD} && \
    ./bootstrap -- -DCMAKE_USE_OPENSSL=ON && \
    make -j4 && \
    make install && \
    rm -rf /tmp/cmake-${CMAKE_VERSION}.${CMAKE_BUILD} && \
    rm -rf /var/lib/apt/lists/*

RUN apt-get update && \
    apt-get install -y \
        wget \
        sudo \
        software-properties-common \
        python-software-properties && \
    cd /tmp && \
    wget https://bootstrap.pypa.io/get-pip.py && \
    python3.5 /tmp/get-pip.py && \
    python2.7 /tmp/get-pip.py && \
    pip3 install -I conan && \
    conan remote add conan-community https://api.bintray.com/conan/conan-community/conan && \
    conan remote add inexorgame https://api.bintray.com/conan/inexorgame/inexor-conan && \
    conan remote add bincrafters https://api.bintray.com/conan/bincrafters/public-conan && \
    rm -rf /var/lib/apt/lists/*

RUN apt-get update && \
    apt-get install -y \
        software-properties-common \
        python-software-properties && \
    add-apt-repository -y ppa:jonathonf/gcc-7.1 && \
    apt-get update && \
    apt-get install -y gcc-7 g++-7 && \
    rm /usr/bin/gcc /usr/bin/g++ && \
    ln -s /usr/bin/gcc-7 /usr/bin/gcc && \
    ln -s /usr/bin/g++-7 /usr/bin/g++ && \
    rm -rf /var/lib/apt/lists/*

ARG ssh_prv_key
ARG ssh_pub_key
RUN apt-get update && \
    apt-get install -y \
        git \
        openssh-server \
        libmysqlclient-dev && \
    mkdir -p /root/.ssh && \
    chmod 0700 /root/.ssh && \
    ssh-keyscan github.com > /root/.ssh/known_hosts && \
    echo "$ssh_prv_key" > /root/.ssh/id_rsa && \
    echo "$ssh_pub_key" > /root/.ssh/id_rsa.pub && \
    chmod 600 /root/.ssh/id_rsa && \
    chmod 600 /root/.ssh/id_rsa.pub && \
    rm -rf /var/lib/apt/lists/*

ADD . /root/sync
RUN cd /root/sync && \
    make conan && \
    make clean && \
    make build CMAKE_BUILD_TYPE=Debug && \
    make build CMAKE_BUILD_TYPE=Release && \
    rm -r /root/sync

WORKDIR /root