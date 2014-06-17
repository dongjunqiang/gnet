#/bin/sh

BIN=../3rd/bin/protoc

$BIN --cpp_out=./ ./*.proto
