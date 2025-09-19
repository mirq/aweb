cmake_minimum_required(VERSION 2.8.5)

project(AWeb LANGUAGES C)

string(TOLOWER ${PROJECT_NAME} PROJECT_NAME_LOWER)

if(NOT AMIGA)
    message(SEND_ERROR "This project only compiles for Amiga")
endif()

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS}  -DAMIGA -D__amigaos__ -DDEVELOPER -Dstpbrk=strpbrk -Wall -Wextra -fomit-frame-pointer")
set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DDEVELOPER -DBETAKEYFILE")

file(GLOB_RECURSE SOURCES  ${PROJECT_SOURCE_DIR}/aweb/*.c)
include_directories(${PROJECT_SOURCE_DIR}/aweb/include)
include_directories(${PROJECT_SOURCE_DIR}/Include)
include_directories(${PROJECT_SOURCE_DIR}/aweblibs/include)
include_directories(${PROJECT_SOURCE_DIR}/amissl/include)
include_directories(${PROJECT_SOURCE_DIR}/zlib)
include_directories(${PROJECT_SOURCE_DIR}/titlebar_ic/include)


set(TARGET_NAME ${PROJECT_NAME_LOWER})

if(ELF2HUNK)
    message("bartman")
    # Add elf2hunk step for Bartman compiler
    set(AWEB_LINKED ${TARGET_NAME}.elf) # Intermediate executable
    set(AWEB_EXE ${TARGET_NAME}.exe) # Use this to launch the game
    add_executable(${AWEB_LINKED} ${SOURCES} ${HEADERS})
    add_custom_command(
            TARGET ${AWEB_LINKED} POST_BUILD
            COMMAND cp ${AWEB_LINKED} ../dist/
    )
    add_custom_command(
            TARGET ${AWEB_LINKED} POST_BUILD
            COMMAND ${ELF2HUNK} ${AWEB_LINKED} ../dist/${AWEB_EXE}
    )
    target_link_libraries(${AWEB_LINKED} awebcommon)
    target_link_libraries(${AWEB_LINKED} awebcfg)
    target_link_libraries(${AWEB_LINKED} zlib)
    target_link_libraries(${AWEB_LINKED} m)

else()


    SET(AWEB_EXE ${TARGET_NAME})
    add_executable(${AWEB_EXE} ${SOURCES} ${HEADERS})
    add_custom_command(
            TARGET ${AWEB_EXE} POST_BUILD
            COMMAND cp ${AWEB_EXE} ${PROJECT_SOURCE_DIR}/dist/
    )
endif()

file(GLOB_RECURSE COMMON_SOURCES  ${PROJECT_SOURCE_DIR}/awebcommon/*.c)
add_library(awebcommon STATIC ${COMMON_SOURCES})
target_include_directories(awebcommon PRIVATE ${PROJECT_SOURCE_DIR}/Include/)
target_include_directories(awebcommon PRIVATE ${PROJECT_SOURCE_DIR}/awebcfg/include)
target_include_directories(awebcommon PRIVATE ${PROJECT_SOURCE_DIR}/awebclib/include)
target_include_directories(awebcommon PRIVATE ${PROJECT_SOURCE_DIR}/aweblibs/include)
target_include_directories(awebcommon PRIVATE ${PROJECT_SOURCE_DIR}/aweb/include)

file(GLOB_RECURSE CFG_SOURCES  ${PROJECT_SOURCE_DIR}/awebcfg/*.c)
add_library(awebcfg STATIC ${CFG_SOURCES})


target_include_directories(awebcfg PRIVATE ${PROJECT_SOURCE_DIR}/Include/)
target_include_directories(awebcfg PRIVATE ${PROJECT_SOURCE_DIR}/awebcfg/include)
target_include_directories(awebcfg PRIVATE ${PROJECT_SOURCE_DIR}/awebclib/include)
target_include_directories(awebcfg PRIVATE ${PROJECT_SOURCE_DIR}/aweblibs/include)
target_include_directories(awebcfg PRIVATE ${PROJECT_SOURCE_DIR}/aweb/include)



file(GLOB_RECURSE ZLIB_SRC  ${PROJECT_SOURCE_DIR}/zlib/*.c)
add_library(zlib STATIC ${ZLIB_SRC})
target_include_directories(zlib PRIVATE ${PROJECT_SOURCE_DIR}/zlib)


target_link_libraries(${AWEB_EXE} awebcommon)
target_link_libraries(${AWEB_EXE} awebcfg)
target_link_libraries(${AWEB_EXE} zlib)
target_link_libraries(${AWEB_EXE} m)
