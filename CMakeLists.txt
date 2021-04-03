project( gba-emulator )
cmake_minimum_required( VERSION 2.8 )

set(CMAKE_CXX_STANDARD 17)

add_executable( ${PROJECT_NAME}
    main.cpp
    opcodes.cpp assembly.cpp
    GBA_Memory.cpp GBA_Cpu.cpp
    bit_utils.cpp )

target_compile_options(${PROJECT_NAME} PRIVATE
  $<$<CXX_COMPILER_ID:MSVC>:/W4 /WX>
  $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall -Wextra -Wpedantic -Werror>
)

if (MSVC)
    #find_package(unofficial-sqlite3 CONFIG REQUIRED)
    #target_link_libraries(pokelib PRIVATE unofficial::sqlite3::sqlite3)

    find_package(fmt 7.1.3 CONFIG REQUIRED)
    target_link_libraries(${PROJECT_NAME} PRIVATE fmt::fmt)
else ()
    target_link_libraries(${PROJECT_NAME} -lfmt)
endif (MSVC)

