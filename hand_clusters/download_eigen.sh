#!/bin/bash

wget https://gitlab.com/libeigen/eigen/-/archive/master/eigen-master.zip
unzip eigen-master.zip
rm eigen-master.zip
mv eigen-master/Eigen .
rm -rf eigen-master

echo "Eigen library downloaded and unzipped successfully!"
