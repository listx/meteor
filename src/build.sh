#!/bin/zsh

make -j$(</proc/cpuinfo grep "^processor" | wc -l) -B

# vim:syntax=zsh
