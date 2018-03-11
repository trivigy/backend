.PHONY: list
list:
	@$(MAKE) -pRrq -f $(lastword $(MAKEFILE_LIST)) : 2>/dev/null | awk -v RS= -F: '/^# File/,/^# Finished Make data base/ {if ($$1 !~ "^[#.]") {print $$1}}' | sort | egrep -v -e '^[^[:alnum:]]' -e '^$@$$' | xargs | tr -s ' '  '\n'

conan:
	conan export conan/libmicrohttpd syncaide/stable
	conan export conan/fifo_map syncaide/stable
	conan export conan/libjson-rpc-cpp syncaide/stable
	conan export conan/json syncaide/stable
	conan export conan/Protobuf syncaide/stable
	conan export conan/gRPC syncaide/stable
	conan export conan/fmt syncaide/stable
	conan export conan/aide syncaide/stable
.PHONY: conan

TARGET=all
CMAKE_BUILD_TYPE=Debug
VERBOSE=OFF
build:
	cmake -H. -Bbuild/${CMAKE_BUILD_TYPE}/ \
		-DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE} \
		-DCMAKE_TOOLCHAIN_FILE=$(shell pwd)/cmake/toolchain.cmake \
		-DCMAKE_VERBOSE_MAKEFILE:BOOL=${VERBOSE} \
		-G "Unix Makefiles"
	cmake --build build/${CMAKE_BUILD_TYPE}/ --target ${TARGET} -- -j4
.PHONY: build

rebuild: clean build
.PHONY: rebuild

test: build
	cd build/${CMAKE_BUILD_TYPE}; \
	ctest -V
.PHONY: test

clean:
	rm -rf build
.PHONY: clean

TAG=gcr.io/syncaide-187503/sync:latest
SSH_PRV_KEY=/home/trivigy/Syncaide/Keys/github/sync/deploy
SSH_PUB_KEY=/home/trivigy/Syncaide/Keys/github/sync/deploy.pub
docker/build:
	docker build -t ${TAG} --build-arg ssh_prv_key="$$(cat ${SSH_PRV_KEY})" --build-arg ssh_pub_key="$$(cat ${SSH_PUB_KEY})" .
.PHONY: docker/build

docker/run: docker/build
	docker run -v $(shell pwd):/root/sync/ --rm -ti ${TAG}
.PHONY: docker/run

docs/build:
	cd doc; \
	doxygen
.PHONY: docs/build

docs/run: docs/build
	cd doc/html; \
	python3 -m http.server
.PHONY: docs/run

deps/bitcoin/clean:
	sudo rm -rf deps/bitcoin deps/db-4.8.30.NC
.PHONY: deps/bitcoin/clean

BDB_PREFIX=deps/db-4.8.30.NC/build_unix/build
deps/bitcoin/install:
	sudo apt-get update
	sudo apt-get install -y git
	mkdir deps/db-4.8.30.NC
	cd deps; \
	wget http://download.oracle.com/berkeley-db/db-4.8.30.NC.tar.gz; \
	tar -xvf db-4.8.30.NC.tar.gz
	cd deps/db-4.8.30.NC/build_unix; \
	mkdir -p build; \
	../dist/configure --disable-shared --enable-cxx --with-pic --prefix=${BDB_PREFIX}; \
	make install
	sudo apt-get install -y \
		autoconf \
		libboost-all-dev \
		libssl-dev \
		libprotobuf-dev \
		protobuf-compiler \
		libqt4-dev \
		libqrencode-dev \
		libtool
	mkdir -p deps/bitcoin
	cd deps; \
	git clone https://github.com/bitcoin/bitcoin.git
	cd deps/bitcoin; \
	./autogen.sh; \
	./configure CPPFLAGS="-I${BDB_PREFIX}/include/ -O2" LDFLAGS="-L${BDB_PREFIX}/lib/" --with-gui --with-boost=/usr/local --with-boost-libdir=/usr/local/lib; \
	make
.PHONY: deps/bitcoin/install