if exist build (
  cd build
) else (
  mkdir build && cd build
)

git clone https://github.com/octoon/octoon-mdl-sdk-bin.git
cd octoon-mdl-sdk-bin/mdl-sdk
conan create . -s build_type=Debug
conan create . -s build_type=Release
cd ../../

cd ../
conan install . --output-folder=build -s arch=x86_64 -s build_type=Debug -s compiler.runtime=dynamic --build missing
conan install . --output-folder=build -s arch=x86_64 -s build_type=Release -s compiler.runtime=dynamic --build missing

if %errorlevel% == 0 (
  cd ./build
  conanbuild.bat
  cmake .. -G "Visual Studio 16" -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake
) else (
  goto ExitLabelFailure
)

if %errorlevel% == 0 (
  cmake --build . --config Debug
  cmake --build . --config Release
) else (
  goto ExitLabelFailure
)

if %errorlevel% == 0 (
  where makensis
  if %errorlevel% == 0 (
    makensis ../samples/unreal/nsis/install.nsi
  )
) else (
  goto ExitLabelFailure
)

:ExitLabelSuccess
cd ..
echo Success Compilation
goto EndLabel

:ExitLabelFailure
cd ..
echo Error Compilation
goto EndLabel

:EndLabel
pause