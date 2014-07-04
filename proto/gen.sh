#/bin/sh

BIN=../3rd/bin/protoc

if [ -f "./*.pb.*" ]
then
    rm ./*.pb.*
fi

$BIN --descriptor_set_out=gnet.desc --include_imports --cpp_out=./ *.proto
