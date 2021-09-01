#!/bin/bash

mkdir ../build 2> /dev/null
cd ../build && cmake -DCMAKE_BUILD_TYPE=release .. && make 