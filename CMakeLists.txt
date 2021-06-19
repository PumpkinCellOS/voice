cmake_minimum_required(VERSION 3.0)
project(Voice)

find_package(SFML 2.5 COMPONENTS audio graphics REQUIRED)
add_executable(Voice
    "main.cpp"
)
target_link_libraries(Voice sfml-audio sfml-graphics)

target_compile_options(Voice PRIVATE -Wall -Wextra -pedantic -Werror)
