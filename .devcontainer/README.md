# AWeb Amiga Development Container

This devcontainer provides a complete development environment for building AWeb using the Amiga m68k cross-compilation toolchain.

## Features

- **Base Image**: `amigadev/crosstools:m68k-amigaos` - Pre-configured Amiga cross-compilation environment
- **Compiler**: m68k-amigaos-gcc (Bartman's GCC port)
- **Build System**: CMake + Make support
- **IDE Integration**: VS Code with C/C++ extensions and IntelliSense configured

## Quick Start

1. **Open in VS Code**: Click "Reopen in Container" when prompted, or use Command Palette > "Dev Containers: Reopen in Container"

2. **Build the project**:
   ```bash
   # Configure CMake with the Amiga toolchain
   cmake -DCMAKE_TOOLCHAIN_FILE=toolchain/m68k-bartmanjob.cmake .

   # Build
   make
   ```

3. **Alternative build with Make**:
   ```bash
   # Use the traditional Makefile if preferred
   make -f Makefile
   ```

## Environment Variables

The container sets up the following environment variables:

- `AMIGA_TOOLCHAIN=/opt/amiga` - Path to the Amiga toolchain
- `CC=m68k-amigaos-gcc` - C compiler
- `CXX=m68k-amigaos-gcc` - C++ compiler (same as C for Amiga)
- `TOOLCHAIN_PATH=/opt/amiga` - Toolchain path for CMake

## Helpful Aliases

- `agcc` - Shortcut for `m68k-amigaos-gcc`
- `amake` - Make with correct compiler set

## VS Code Configuration

The devcontainer automatically configures:

- **IntelliSense**: Recognizes Amiga-specific headers and defines
- **CMake Tools**: Integration for building and debugging
- **C/C++ Extension**: Proper syntax highlighting and code completion

### Key Settings Applied

- Compiler path: `/opt/amiga/bin/m68k-amigaos-gcc`
- Include paths: Project headers + Amiga system headers
- Defines: `AMIGA`, `__amigaos__`, `__stdargs=`, `__saveds=`, `BARTMAN_GCC`

## Toolchain Details

This environment uses the Bartman GCC toolchain specifically designed for Amiga development:

- **Target**: m68k-amigaos (68000 processor)
- **System Headers**: Located in `/opt/amiga/m68k-amigaos/sys-include`
- **Libraries**: Amiga system libraries and C runtime

## Project Structure Support

The devcontainer is configured to work with AWeb's structure:

- Main source: `aweb/`
- Libraries: `aweblibs/`
- Configuration: `awebcfg/`
- Platform headers: `Include/`
- Toolchain: `toolchain/` (git submodule)

## Troubleshooting

### Build Issues
- Ensure the toolchain submodule is initialized: `git submodule update --init`
- Verify CMake uses the correct toolchain: Check that `-DCMAKE_TOOLCHAIN_FILE` points to the right file

### VS Code Issues
- Reload window if IntelliSense doesn't work: `Ctrl+Shift+P` > "Developer: Reload Window"
- Check that the C/C++ extension is active

### Container Issues
- Rebuild container: `Ctrl+Shift+P` > "Dev Containers: Rebuild Container"
- Check Docker is running and has sufficient resources