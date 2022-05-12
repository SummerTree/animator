if exist build (
  cd build
) else (
  mkdir build && cd build
)

conan remote add iteale http://conan.iteale.com:19479

conan install .. -g cmake_multi -s arch=x86_64 -s build_type=Debug -s compiler.runtime=MDd
conan install .. -g cmake_multi -s arch=x86_64 -s build_type=Release -s compiler.runtime=MD

cmake .. -G "Visual Studio 16"

cmake --build . --config Debug
cmake --build . --config Release

cd ..