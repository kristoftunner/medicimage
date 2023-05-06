echo "Configuring git submodule dependecies"
git submodule init
git submodule update --init --recursive
cmake -B build . -DCMAKE_TOOLCHAIN_FILE=d:/dev/libs/vcpkg/scripts/buildsystems/vcpkg.cmake