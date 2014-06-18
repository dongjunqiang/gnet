#/bin/sh

BIN=../3rd/bin/protoc

rm ./*.pb.h ./*.pb.cc
$BIN --descriptor_set_out=gnet.pb --include_imports --proto_path=./protobuf/ --cpp_out=./ ./protobuf/*.proto
