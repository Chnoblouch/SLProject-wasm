mkdir BUILD_IOS
cd BUILD_IOS
cmake .. -GXcode -DCMAKE_SYSTEM_NAME=iOS -DCMAKE_SYSTEM_PROCESSOR=arm -DCMAKE_OSX_DEPLOYMENT_TARGET=8.0