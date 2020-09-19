# Cutepiler-Sysy2020

This is a compiler for Sysy2020 (a C-like toy language) on ARMv7-a. For more information about Sysy2020, see https://compiler.educg.net/. 

## Dependency 

This project uses C++17 without any third-party packages, and is tested with clang 11.0.0. Other utilities needed are listed as follow.
- `cmake` is used to generate makefile. 
- `qemu-arm-static` are used to test generated target code. See https://gist.github.com/Liryna/10710751 for details. 
- `python` is used to run automatic test.
- `flex/bison` are used to generate parser. **Attention:** If you want to generate a new parser, you may need to modify the source code of it in order to make it valid under C++17. 

## Compiling

```
mkdir build
cd build
cmake ..
make
cd ..
```

## Usage 

```
cutepiler [-S] [-o output_file] [-O opt_level] input_file
```

- `-o`: Name of output file. 
- `-O`: Set optimization level, `0` default. 

## Testing

TODO
