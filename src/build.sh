#!/bin/zsh

make -j$(</proc/cpuinfo grep "^processor" | wc -l) -B -f Makefile-production

# vim:syntax=zsh
