%% gcc must be in the PATH
call gcc -o FlowGraph.exe main.c -s -static -O2 -std=c99 -Wall -Iexternal -DPLATFORM_DESKTOP -lraylib -lopengl32 -lgdi32 -lwinmm

%% replace with path/to/emsdk_base_directory/emsdk_env.bat
call emsdk_env.bat

%% emcc must be in the PATH
call emcc -o Flowgraph.html main.c -Os -Wall .\raylib_web\libraylib.a -I. -I.\raylib_web -L. -L.\raylib_web -s USE_GLFW=3 --shell-file .\raylib_web\shell.html -DPLATFORM_WEB -s FORCE_FILESYSTEM=1 --preload-file .\resources\CourierNewBold.ttf 


