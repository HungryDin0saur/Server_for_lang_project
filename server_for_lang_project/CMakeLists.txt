cmake_minimum_required(VERSION 3.5)

project(srv_for_lang_project LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(THREADS_PREFER_PTHREAD_FLAG ON) #Подключаем многопоточность
set(COMPILE_OPTIONS "-Wpedantic;-Wall;-Wextra;-Werror;-Wconversion")

add_executable(srv_for_lang_project main.cpp)

#set(CMAKE_TOOLCHAIN_FILE /home/lost_brain/vcpkg/scripts/buildsystems/vcpkg.cmake)

find_path(UWEBSOCKETS_INCLUDE_DIRS "uwebsockets/App.h")
find_package(Threads REQUIRED) #Подключаем многопоточность

target_link_libraries(srv_for_lang_project PRIVATE Threads::Threads) #Подключаем многопоточность

target_include_directories(srv_for_lang_project PRIVATE ${UWEBSOCKETS_INCLUDE_DIRS})

target_link_libraries(srv_for_lang_project PRIVATE /home/lost_brain/vcpkg/packages/usockets_x64-linux/lib/libuSockets.a)
target_link_libraries(srv_for_lang_project PRIVATE z) #Подключили zlib
