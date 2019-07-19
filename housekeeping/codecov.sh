#!/bin/bash -xe

buildDir=$1
token=$2


if [ -z "$buildDir" ]; then
    echo "buildDir is empty"
    exit 1
fi

if [ -z "$token" ]; then
    echo "token arg is empty"
    exit 2
fi

cd $buildDir
bash <(curl -s https://codecov.io/bash) -s $buildDir -t $token
