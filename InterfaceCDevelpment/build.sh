#!/bin/bash
# Setup MSYS2 environment and build the project

export PATH="/c/msys64/mingw64/bin:$PATH"

echo "Building DonCEyKongJr Client..."
make "$@"

if [ $? -eq 0 ]; then
    echo "Build successful!"
else
    echo "Build failed!"
    exit 1
fi
