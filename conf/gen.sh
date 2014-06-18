#/bin/sh

BIN=../3rd/bin/protoc

if [ -f "./*.pb.*" ]
then
    rm ./*.pb.*
fi

$BIN --cpp_out=./ ./*.proto
