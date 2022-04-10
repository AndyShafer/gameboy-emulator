add_executable(${PROJECT_NAME}_test main.cpp)

target_include_directories(${PROJECT_NAME}_test PUBLIC ../../src)

target_link_directories(${PROJECT_NAME}_test PRIVATE ../../src)

target_link_libraries(${PROJECT_NAME}_test gbemulator)
