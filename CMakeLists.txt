cmake_minimum_required(VERSION 3.16)
project(Bomberman_AP VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(bomberman_logic
        src/logic/Entity.cpp
        src/logic/Random.cpp
        src/logic/Score.cpp
        src/logic/Stopwatch.cpp
        src/logic/World.cpp)

target_include_directories(bomberman_logic
        PUBLIC
        ${PROJECT_SOURCE_DIR}/include)

target_compile_options(bomberman_logic PRIVATE -Wall -Wextra -Wpedantic)

find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)

add_executable(Bomberman_AP
        src/sfml/Game.cpp
        src/sfml/SfmlFactory.cpp
        src/sfml/StateManager.cpp
        src/sfml/SpriteView.cpp
        main.cpp)

target_link_libraries(Bomberman_AP
        PRIVATE
        bomberman_logic
        sfml-graphics
        sfml-window
        sfml-system)

target_compile_options(Bomberman_AP PRIVATE -Wall -Wextra -Wpedantic)

add_custom_command(TARGET Bomberman_AP POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${PROJECT_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:Bomberman_AP>/assets)

add_executable(bomberman_logic_tests tests/logic_tests.cpp)
target_link_libraries(bomberman_logic_tests PRIVATE bomberman_logic)
target_compile_options(bomberman_logic_tests PRIVATE -Wall -Wextra -Wpedantic)