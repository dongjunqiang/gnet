#/bin/sh

BIN=../3rd/protobuf-2.5.0/bin/protoc

$BIN --descriptor_set_out=gnet.pb --include_imports --proto_path=./protobuf/ --cpp_out=./ ./protobuf/*.proto
