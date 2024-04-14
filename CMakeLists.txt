
add_library(max5134_driver INTERFACE)
target_sources(max5134_driver PUBLIC max5134.cpp)
target_link_libraries(max5134_driver INTERFACE combase_driver)
target_include_directories(max5134_driver INTERFACE ${CMAKE_CURRENT_LIST_DIR})
