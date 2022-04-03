@ECHO OFF

SetLocal EnableDelayedExpansion

SET cFilenames=
FOR /R ../code %%f in (*.cpp) do (
	SET cFilenames=!cFilenames! %%f
)

ECHO "Files: " %cFilenames%

SET compilerFlags=-std=c++17 -Wvarargs -Wall -Wno-unused-variable -Wno-unused-function -Werror
SET defines=-DCRT_SECURE_NO_WARNINGS
SET includeFlags=-I../code -I../raylib/include
SET linkerFlags=-l../raylib/bin/raylib

ECHO "Building game..."
clang++ %cFilenames% %compilerFlags% -o ../bin/game.exe %defines% %includeFlags% %linkerFlags%