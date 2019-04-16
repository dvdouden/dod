cmake_minimum_required(VERSION 3.14)
project(DodFix)


set(DODFIX_SRC
        src/dodfix.cpp
        include/Atom.hpp
        src/Atom.cpp
        )


set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")


add_executable(DodFix ${DODFIX_SRC})

target_include_directories(DodFix
        PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include
		)
target_link_libraries(DodFix -static-libgcc -static-libstdc++)
