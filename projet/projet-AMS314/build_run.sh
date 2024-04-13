#!/bin/bash
cd src/
make -f makemesh.make
cd ..
./src/mesh ./box.mesh