cmake_minimum_required(VERSION 3.16)
project(Bomberman_AP VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/installed" CACHE PATH "Install path prefix" FORCE)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_library(bomberman_logic
        src/logic/Entity.cpp
        src/logic/Random.cpp
        src/logic/Score.cpp
        src/logic/Stopwatch.cpp
        src/logic/World.cpp
        src/logic/WorldAi.cpp
        src/logic/WorldBombs.cpp
        src/logic/WorldPowerUps.cpp)

target_include_directories(bomberman_logic
        PUBLIC
        ${PROJECT_SOURCE_DIR}/src)

target_compile_options(bomberman_logic PRIVATE -Wall -Wextra -Wpedantic)

find_package(SFML 2.5 COMPONENTS graphics window system REQUIRED)

add_executable(Bomberman_AP
        src/sfml/Game.cpp
        src/sfml/ConcreteFactory.cpp
        src/sfml/StateManager.cpp
        src/sfml/EntityView.cpp
        main.cpp)

target_link_libraries(Bomberman_AP
        PRIVATE
        bomberman_logic
        sfml-graphics
        sfml-window
        sfml-system)

target_compile_options(Bomberman_AP PRIVATE -Wall -Wextra -Wpedantic)

enable_testing()

add_executable(bomberman_logic_tests
        tests/logic_tests.cpp)

target_link_libraries(bomberman_logic_tests
        PRIVATE
        bomberman_logic)

target_compile_options(bomberman_logic_tests PRIVATE -Wall -Wextra -Wpedantic)

add_test(NAME bomberman_logic_tests COMMAND bomberman_logic_tests)

add_custom_command(TARGET Bomberman_AP POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${PROJECT_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:Bomberman_AP>/assets)

install(TARGETS Bomberman_AP DESTINATION bin)
install(DIRECTORY ${PROJECT_SOURCE_DIR}/assets DESTINATION .)

find_package(Doxygen)
if(DOXYGEN_FOUND)
    set(DOXYGEN_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/doc)
    set(DOXYGEN_RECURSIVE YES)
    set(DOXYGEN_EXTRACT_ALL YES)
    set(DOXYGEN_EXTRACT_ANON_NSPACES YES)
    set(DOXYGEN_HAVE_DOT YES)
    doxygen_add_docs(doc ${PROJECT_SOURCE_DIR}/src COMMENT "Generate Doxygen documentation")
endif()
