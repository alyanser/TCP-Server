#!/bin/bash

[[ -d "build" ]] && cd build && [[ -x "tcpserver" ]] && ./tcpserver || echo executable not built/found