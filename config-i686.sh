#!/bin/sh

CXXFLAGS="-O3 -mcpu=pentiumpro -march=pentiumpro -ffast-math -DNDEBUG -funroll-loops"

echo Configuring for compile with: ${CXXFLAGS}
rm -f config.cache
CXXFLAGS=${CXXFLAGS} ./configure $*
