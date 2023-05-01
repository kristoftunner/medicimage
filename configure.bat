echo "Configuring git submodule dependecies"
git submodule init
git submodule update --init --recursive
conan install . --output-folder=build --build=missing
cd build
cmake .. -G "Visual Studio 17 2022" -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake