#! /bin/sh

DIR=$(pwd)
export GOPATH=$GOPATH:${DIR}

CGO_LDFLAGS=" -std=c99"
CGO_LDFLAGS+=" -lm"
export CGO_LDFLAGS

go build main.go
