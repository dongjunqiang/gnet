#/bin/sh

BIN=../3rd/protobuf-2.5.0/bin/protoc

$BIN --cpp_out=./ ./*.proto
