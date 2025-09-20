# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

AWeb is an Amiga web browser written in C. This is the open source version released under the AWeb Public License (APL). The project is specifically designed for Amiga systems and uses Amiga-specific libraries and APIs.

## Build System

The project uses CMake as the active build configuration:

### Primary Build Commands

- `cmake .` - Configure CMake build (requires AMIGA flag set)
- `make` - Build the project after CMake configuration
- `make clean` - Remove build artifacts

### Build Requirements

- CMake 2.8.5 or higher
- Must be building for Amiga platform (AMIGA flag required)
- Uses m68k-amigaos-gcc cross-compiler

## Directory Structure

### Core Application
- **aweb/** - Main browser application source code (120+ C files)
  - Core browser functionality, HTML parsing, rendering, UI
  - `aweb/include/` - Comprehensive header files (70+ headers)
    - Key headers: `application.h`, `aweb.h`, `awebprotos.h`, `object.h`
    - Component headers: `frame.h`, `window.h`, `document.h`, `url.h`
    - UI elements: `button.h`, `form.h`, `table.h`, `textarea.h`

### Library Modules (aweblibs/)
- **arexx/** - ARexx scripting interface
- **authorize/** - HTTP authentication handling
- **awebjs/** - JavaScript engine integration
- **cachebrowser/** - Cache management interface
- **ftp/** - FTP protocol implementation (`ftp.c`, `ftpparse.c`)
- **gopher/** - Gopher protocol support
- **history/** - Browser history management
- **hotlist/** - Bookmark/favorites system
- **mail/** - Email protocol support
- **news/** - NNTP news protocol
- **print/** - Printing subsystem
- **startup/** - Application initialization
- **include/** - Library headers and pragma files
  - Contains pragma includes for all lib modules
  - Inline function definitions

### Configuration & Common Code
- **awebcfg/** - Configuration management system
  - `awebcfg/include/` - Configuration headers
- **awebcommon/** - Shared utilities and common code
- **awebclib/** - Core C library extensions
- **awebjs/** - JavaScript-related code

### Plugin System (plugins/)
- **charset/** - Character set conversion plugins
- **docs/** - Plugin documentation and examples
- **gif/** - GIF image format support
- **jfif/** - JPEG image format support  
- **png/** - PNG image format support
- **filterexample/** - Example content filter plugin
- **ilbmexample/** - Example ILBM image plugin
- **include/** - Plugin development headers (10 subdirs)

### Localization (catalogs/)
Extensive multi-language support with 30+ language catalogs:
- Major languages: deutsch, english, français, italiano, español
- Nordic: dansk, norsk, svenska, suomi
- Slavic: polski, hrvatski, srpski, slovenian, czech, slovak
- Others: magyar, nederlands, português, ελληνικά, türkçe, русский

### Platform Support
- **C/** - Amiga C runtime components
- **Include/** - Platform headers and definitions
  - `awebdef.h`, `aweblib.h`, `awebmath.h` - Core definitions
  - `awebprefs.h` - Preferences system (15KB header)
  - `platform_specific.h` - Platform abstraction (28KB)
  - `library_types_*.h` - OS3/OS4/MorphOS variants
- **module/** - Platform-specific modules
  - `clib2/`, `libnix/`, `newlib/` - Different C library variants

### Development & Build
- **tools/** - Build utilities
  - `builddate.c` - Build timestamp generation  
  - `tubsfind/` - File discovery for build system
  - `makeindex.c`, `searchindex.c` - Documentation indexing
- **zlib/** - Compression library with CVS history
- **charsets/** - Character encoding definitions

### Documentation & Assets
- **html/** - Documentation and help files
- **user/** - User-facing components
  - `Docs/` - User documentation
  - `Extras/` - Additional utilities
  - `Icons/` - Application icons
  - `Images/` - Image resources
  - `Storage/` - Configuration storage

### Key Files
- `CMakeLists.txt` - Build configuration
- `AWebAPL_README` - Developer guide and coding standards
- `ChangeLog` - Detailed change history (113KB)
- `ReadMe` - Basic project information
- `Makefile*` - Alternative build files (legacy)
- `Tubsengine*` - Custom build system files

## Development Environment

### Compiler Requirements

- **Target**: m68k-amigaos-gcc (Cross-compiler for Amiga)
- **Host**: gcc (for build tools)
- **Standards**: C11

### Key Flags

- `-DAMIGA -D__amigaos__` - Amiga platform defines
- `-m68000` - Target 68000 processor
- `-mcrt=nix20 -noixemul` - Amiga C runtime

## File Organization

The codebase follows Amiga conventions with extensive use of:
- Amiga-specific APIs and libraries
- Object-oriented C patterns (seen in extensive header files)
- Modular plugin architecture
- Catalog-based localization system

## Important Notes

- This is Amiga-specific code that requires cross-compilation setup
- The build system automatically handles dependencies between modules
- Plugins are dynamically loadable modules
- Extensive localization support through catalog system