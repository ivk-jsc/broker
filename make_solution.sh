#!/bin/bash

if [ ! $1  ]; then
    echo "please, set configuration, Debug or Release"
    echo "for ex., ./make_solution Debug"
    exit 1
fi

NCPU=$([[ $(uname) = 'Darwin' ]] &&
                       sysctl -n hw.logicalcpu_max ||
                       lscpu -p | egrep -v '^#' | wc -l)

CMAKE_EXECUTABLE="cmake"
SOURCE_DIR=$PWD
BUILD_DIR=${SOURCE_DIR}/cmake-$1-build

if [ -d "${BUILD_DIR}" ]; then
  rm -Rf "${BUILD_DIR}"
fi

mkdir "${BUILD_DIR}"

cd "${BUILD_DIR}"
${CMAKE_EXECUTABLE} -DCMAKE_BUILD_TYPE=$1 -DCMAKE_INSTALL_PREFIX="~/upmq" "${SOURCE_DIR}"
cd "${SOURCE_DIR}"

${CMAKE_EXECUTABLE} --build "${BUILD_DIR}" -- -j${NCPU}

if [ "$2" == "install" ]; then
    ${CMAKE_EXECUTABLE} --build "${BUILD_DIR}" --target install
fi
