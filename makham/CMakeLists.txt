add_library(f5-makham
        executor.cpp
    )
target_include_directories(f5-makham PUBLIC ../include)
target_link_libraries(f5-makham PUBLIC fost-core thread-pool)
target_compile_features(f5-makham PUBLIC cxx_std_17)

if(${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang" OR ${CMAKE_CXX_COMPILER_ID} STREQUAL "AppleClang")
    target_compile_options(f5-makham PUBLIC -fcoroutines-ts)
elseif(${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
    target_compile_options(f5-makham PUBLIC -fcoroutines)
else()
    message(SEND_ERROR "Unknown compiler ID: ${CMAKE_CXX_COMPILER_ID}")
endif()

install(DIRECTORY ../include/f5 DESTINATION include)
install(TARGETS f5-makham LIBRARY DESTINATION lib ARCHIVE DESTINATION lib)
