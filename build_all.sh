#!/bin/bash

build()
{
    build_dir="$1_$2"
    mkdir -p $build_dir
    cd $build_dir
    echo -ne "Build type: $1,\twith clutchlog: $2...\t"
    (cmake -DCMAKE_BUILD_TYPE=$1 -DWITH_CLUTCHLOG=$2 .. && make && ctest) 2>&1 >> ../build_all.log
    if [[ $? == 0 ]]; then
        echo "OK"
    else
        echo "ERROR"
    fi
    cd ..
}

rm -f build_all.log

for t in "Debug" "Release" "RelWithDebInfo"; do
    for w in "ON" "OFF"; do
        build $t $w
    done
done

echo "Update documentation"
cd Debug_ON
cmake -DBUILD_DOCUMENTATION=ON ..
make doc
cp -r html/* ../docs/

