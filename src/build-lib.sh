#!/bin/zsh

make clean
make -j$(</proc/cpuinfo grep "^processor" | wc -l) -B -f Makefile-production
make lib

# vim:syntax=zsh
