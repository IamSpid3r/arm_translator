cmake -DCMAKE_CXX_COMPILER=g++ -DCMAKE_C_COMPILER=gcc -DCMAKE_BUILD_TYPE=Release -DLLVM_ENABLE_PIC=off -DLLVM_ENABLE_RTTI=off -DLLVM_ENABLE_TERMINFO=off -DLLVM_ENABLE_THREADS=off -DLLVM_ENABLE_TIMESTAMPS=off -DLLVM_ENABLE_ZLIB=off -DLLVM_TARGETS_TO_BUILD="X86" -DLLVM_BUILD_32_BITS=on -DLLVM_EXTERNAL_CLANG_BUILD=off -DLLVM_EXTERNAL_COMPILER_RT_BUILD=off  ..