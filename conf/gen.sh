#/bin/sh

BIN=../3rd/bin/protoc

rm ./*.pb.h ./*.pb.cc
$BIN --cpp_out=./ ./*.proto
