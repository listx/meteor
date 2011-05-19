#!/bin/zsh

make -j$(</proc/cpuinfo grep "^processor" | wc -l) -B -f Makefile-debug

# vim:syntax=zsh
