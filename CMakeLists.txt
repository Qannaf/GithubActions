cmake_minimum_required(VERSION 3.16)

project(main)

#+------------------------------
# Add the headers(/*.h)
set(hdr "include/")
include_directories(${hdr})

#+------------------------------
# Add the sources (/*.cpp)
file(GLOB USER_CU "src/*.cpp" "src/*.cu"  )

add_executable(${PROJECT_NAME} ${USER_CU})

install(TARGETS main)

enable_testing()
add_test(NAME main COMMAND main)
