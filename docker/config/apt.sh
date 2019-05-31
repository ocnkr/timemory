#!/bin/bash -e

# function for running command verbosely
run-verbose()
{
    echo -e "\n\t##### Running : '$@'... #####\n"
    eval $@
}

# enable manpages to be installed
sed -i 's/path-exclude/# path-exclude/g' /etc/dpkg/dpkg.cfg.d/excludes

#-----------------------------------------------------------------------------#
#
#   apt configuration
#
#-----------------------------------------------------------------------------#

run-verbose apt-get update
run-verbose apt-get install -y software-properties-common
run-verbose add-apt-repository -u -y ppa:ubuntu-toolchain-r/test
run-verbose apt-get dist-upgrade -y

#-----------------------------------------------------------------------------#
#
#   Base packages
#
#-----------------------------------------------------------------------------#

run-verbose apt-get install -y wget curl build-essential cmake git-core ssed

#-----------------------------------------------------------------------------#
#
#   Compiler specific installation
#
#-----------------------------------------------------------------------------#

if [ "${COMPILER_TYPE}" = "gcc" ]; then

    # install compilers
    run-verbose apt-get -y install gcc-${COMPILER_VERSION} g++-${COMPILER_VERSION} gcc-${COMPILER_VERSION}-multilib

elif [ "${COMPILER_TYPE}" = "llvm" ]; then

    # install compilers
    run-verbose apt-get -y install clang-${COMPILER_VERSION} libc++-dev libc++abi-dev

fi

#-----------------------------------------------------------------------------#
#
#   Install supplemental packages
#
#-----------------------------------------------------------------------------#

run-verbose apt-get install -y \
    libxerces-c-dev libexpat1-dev libhdf5-dev libhdf5-mpich-dev libmpich-dev mpich \
    xserver-xorg freeglut3-dev libx11-dev libx11-xcb-dev libxpm-dev libxft-dev libxmu-dev libxv-dev libxrandr-dev \
    libglew-dev libftgl-dev libxkbcommon-x11-dev libxrender-dev libxxf86vm-dev libxinerama-dev qt5-default \
    python python-dev ninja-build clang-tidy clang-format \
    manpages manpages-dev cppman manpages-posix manpages-posix-dev \
    qtcreator emacs-nox vim-nox \
    libgoogle-perftools-dev google-perftools libtbb-dev valgrind papi-tools libpapi-dev

#-----------------------------------------------------------------------------#
#   UPDATE ALTERNATIVES -- GCC
#-----------------------------------------------------------------------------#
priority=10
for i in 5 6 7 8
do
    if [ -n "$(which gcc-${i})" ]; then
        run-verbose update-alternatives --install $(which gcc) gcc $(which gcc-${i}) ${priority} \
            --slave $(which g++) g++ $(which g++-${i})
        run-verbose priority=$(( ${priority}+10 ))
    fi
done

#-----------------------------------------------------------------------------#
#   UPDATE ALTERNATIVES -- CLANG
#-----------------------------------------------------------------------------#
priority=10
for i in 5 6 7
do
    if [ -n "$(which clang-${i}.0)" ]; then
        run-verbose update-alternatives --install /usr/bin/clang clang $(which clang-${i}.0) ${priority} \
            --slave /usr/bin/clang++ clang++ $(which clang++-${i}.0)
        run-verbose priority=$(( ${priority}+10 ))
    fi
done

#-----------------------------------------------------------------------------#
#   UPDATE ALTERNATIVES -- cc/c++
#-----------------------------------------------------------------------------#
priority=10
if [ -n "$(which clang)" ]; then
    run-verbose update-alternatives --install $(which cc)  cc  $(which clang)   ${priority}
    run-verbose update-alternatives --install $(which c++) c++ $(which clang++) ${priority}
    run-verbose priority=$(( ${priority}+10 ))
fi

if [ -n "$(which gcc)" ]; then
    run-verbose update-alternatives --install $(which cc)  cc  $(which gcc)     ${priority}
    run-verbose update-alternatives --install $(which c++) c++ $(which g++)     ${priority}
    run-verbose priority=$(( ${priority}+10 ))
fi

if [ -d /opt/intel ]; then
    export PATH=/opt/intel/bin:${PATH}
    if [ -n "$(which icc)" ]; then
        run-verbose update-alternatives --install $(which cc)  cc  $(which icc)     ${priority}
        run-verbose update-alternatives --install $(which c++) c++ $(which icpc)     ${priority}
        run-verbose priority=$(( ${priority}+10 ))
    fi
fi

#-----------------------------------------------------------------------------#
#   UPDATE ALTERNATIVES -- CUDA compilers
#-----------------------------------------------------------------------------#
priority=10
for i in clang++-6.0 clang++-7.0 nvcc
do
    if [ -n "$(which ${i})" ]; then
        run-verbose update-alternatives --install /usr/bin/cu cu $(which ${i}) ${priority}
    fi
done

#-----------------------------------------------------------------------------#
#   CLEANUP
#-----------------------------------------------------------------------------#
run-verbose apt-get -y autoclean
run-verbose rm -rf /var/lib/apt/lists/*

#-----------------------------------------------------------------------------#
#   CONDA
#-----------------------------------------------------------------------------#
wget https://repo.continuum.io/miniconda/Miniconda3-latest-Linux-x86_64.sh -O miniconda.sh
bash miniconda.sh -b -p /opt/conda
export PATH="/opt/conda/bin:${PATH}"
conda config --set always_yes yes --set changeps1 no
conda update -c defaults -n base conda
conda install -n base -c defaults -c conda-forge python=3.6 pyctest cmake
conda clean -a -y
