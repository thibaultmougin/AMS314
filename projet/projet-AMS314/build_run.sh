#!/bin/bash
cd src/
make -f makemesh.make
cd ..
./src/mesh data/carre_4h.mesh