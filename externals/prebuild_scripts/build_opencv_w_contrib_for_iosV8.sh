#!/bin/sh

# ####################################################
# Build script for OpenCV with contributions for iOS
# ####################################################

CV_VERSION=$1
ARCH="iosV8"
ZIPFOLDER=build/"$ARCH"_opencv_"$CV_VERSION"
BUILD_D=build/"$ARCH"_debug_"$CV_VERSION"
BUILD_R=build/"$ARCH"_release_"$CV_VERSION"

clear
echo "============================================================"
echo "Building OpenCV Version: $CV_VERSION for architecture: $ARCH"
echo "============================================================"

# Check tag parameter
if [ "$1" == "" ]; then
    echo "No OpenCV tag passed as 1st parameter"
    exit
fi

# Cloning OpenCV
if [ ! -d "opencv/.git" ]; then
    git clone https://github.com/Itseez/opencv.git
fi

# Cloning OpenCV contributions
if [ ! -d "opencv_contrib/.git" ]; then
    git clone https://github.com/Itseez/opencv_contrib.git
fi

# Get all OpenCV tags and check if the requested exists
cd opencv
git tag > opencv_tags.txt
if grep -Fx "$CV_VERSION" opencv_tags.txt > /dev/null; then
    git checkout $CV_VERSION
    git pull origin $CV_VERSION
    cd ..
    cd opencv_contrib
    git checkout $CV_VERSION
    git pull origin $CV_VERSION
    rm -f opencv_tags.txt
    cd ..
else
    echo "No valid OpenCV tag passed as 1st parameter"
    exit
fi

# Make build folder for debug version
echo "============================================================"
cd opencv
if [ ! -d "build" ]; then
    mkdir build
fi
rm -rf $BUILD_D
mkdir $BUILD_D
cd $BUILD_D

echo "====================================================== cmake"
# Run cmake to configure and generate for iosV8 debug
cmake \
-DCMAKE_CONFIGURATION_TYPES=Debug \
-DCMAKE_BUILD_TYPE=Debug \
-DBUILD_WITH_DEBUG_INFO=ON \
-DCMAKE_INSTALL_PREFIX=./install \
-DBUILD_opencv_python_bindings_generator=OFF \
-DBUILD_opencv_python2=OFF \
-DBUILD_opencv_java_bindings_generator=OFF \
-DBUILD_opencv_world=OFF \
-DBUILD_PERF_TESTS=OFF \
-DBUILD_TESTS=OFF \
-DWITH_MATLAB=OFF \
-DOPENCV_EXTRA_MODULES_PATH=../../../opencv_contrib/modules \
-DWITH_CUDA=OFF \
-DWITH_OPENCL=OFF \
-DWITH_OPENCL_SVM=OFF \
-DWITH_OPENCLAMDFFT=OFF \
-DWITH_OPENCLAMDBLAS=OFF \
-DWITH_VA_INTEL=OFF \
-GXcode \
-DAPPLE_FRAMEWORK=ON \
-DPLATFORM=OS64 \
-DCMAKE_TOOLCHAIN_FILE=../../../ios.toolchain.cmake \
-DENABLE_NEON=ON \
-DENABLE_ARC=OFF \
../..

cmake --build . --config Debug --target install

: '
echo "================================================= xcodebuild"
xcodebuild \
IPHONEOS_DEPLOYMENT_TARGET=8.0 \
ARCHS=arm64 \
-sdk=phoneos \
-configuration=Debug \
-jobs=1 \
-target=ALL_BUILD \
-parallelizeTargets \
build

cd ../.. # back to opencv

# Make build folder for release version
echo "============================================================"
rm -rf $BUILD_R
mkdir $BUILD_R
cd $BUILD_R

# Run cmake to configure and generate the make files
cmake \
-DCMAKE_CONFIGURATION_TYPES=Release \
-DCMAKE_BUILD_TYPE=Release \
-DBUILD_WITH_DEBUG_INFO=0 \
-DCMAKE_INSTALL_PREFIX=./install \
-DBUILD_opencv_python_bindings_generator=0 \
-DBUILD_opencv_python2=0 \
-DBUILD_opencv_java_bindings_generator=0 \
-DBUILD_opencv_world=0 \
-DBUILD_PERF_TESTS=0 \
-DBUILD_TESTS=0 \
-DWITH_MATLAB=0 \
-DOPENCV_EXTRA_MODULES_PATH=../../../opencv_contrib/modules \
-DWITH_OPENCL=0 \
-DWITH_OPENCLAMDFFT=0 \
-DWITH_OPENCLAMDBLAS=0 \
-DWITH_VA_INTEL=0 \
-GXcode \
-DPLATFORM=OS64 \
-DCMAKE_TOOLCHAIN_FILE=../../../ios.toolchain.cmake \
-DENABLE_NEON=1 \
-DENABLE_ARC=0 \
../..

cmake --build . --config Release --target install

cd ../.. # Back to opencv

# Create zip folder for debug and release version
rm -rf $ZIPFOLDER
mkdir $ZIPFOLDER
cp -R $BUILD_R/install/include $ZIPFOLDER/include
cp -R $BUILD_R/install/lib     $ZIPFOLDER/release
cp -R $BUILD_D/install/lib     $ZIPFOLDER/debug
cp LICENSE $ZIPFOLDER
cp README.md $ZIPFOLDER
'