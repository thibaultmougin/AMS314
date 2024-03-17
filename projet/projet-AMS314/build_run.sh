#!/bin/bash
cd src/
make -f makemesh.make
cd ..
./src/mesh ./data/carre_2h.mesh