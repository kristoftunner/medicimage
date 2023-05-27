echo "Configuring git submodule dependecies"
git submodule init
git submodule update --init --recursive
cmake -B build . 