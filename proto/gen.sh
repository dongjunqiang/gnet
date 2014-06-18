#/bin/sh

BIN=../3rd/bin/protoc

if [ -f "./*.pb.*" ]
then
    rm ./*.pb.*
fi

$BIN --descriptor_set_out=gnet.pb --include_imports --proto_path=./protobuf/ --cpp_out=./ ./protobuf/*.proto
