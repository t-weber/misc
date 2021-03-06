#!/bin/bash


echo -e "\n\n--------------------------------------------------------------------------------"
echo -e "Building py interface"
echo -e "--------------------------------------------------------------------------------\n"
swig -v -c++ -python expr_parser.i
g++ -v -std=c++17 -I/usr/include/python3.7/ -o _expr_parser.so -shared -fpic expr_parser_wrap.cxx


echo -e "\n\n--------------------------------------------------------------------------------"
echo -e "Building java interface"
echo -e "--------------------------------------------------------------------------------\n"
swig -v -c++ -java expr_parser.i
g++ -v -std=c++17 -I/usr/lib/jvm/java-11-openjdk-11.0.6.10-0.fc31.x86_64/include/ -I/usr/lib/jvm/java-11-openjdk-11.0.6.10-0.fc31.x86_64/include/linux/ -o expr_parser.so -shared -fpic expr_parser_wrap.cxx
