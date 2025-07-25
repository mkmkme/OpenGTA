option(SANITIZE "Enable one of the sanitizers" "")

if(SANITIZE)
  if(SANITIZE STREQUAL "address")
    set(ASAN_FLAGS
        "-fsanitize=address -fsanitize-address-use-after-scope -fno-omit-frame-pointer"
    )
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${ASAN_FLAGS}")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${ASAN_FLAGS}")
  elseif(SANITIZE STREQUAL "memory")
    set(MSAN_FLAGS
        "-fsanitize=memory -fsanitize-memory-use-after-dtor -fsanitize-memory-track-origins -fno-optimize-sibling-calls"
    )
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${MSAN_FLAGS}")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${MSAN_FLAGS}")
  elseif(SANITIZE STREQUAL "undefined")
    set(UBSAN_FLAGS
        "-fsanitize=undefined -fno-sanitize-recover=all -fno-sanitize=float-divide-by-zero"
    )
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${UBSAN_FLAGS}")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${UBSAN_FLAGS}")
  endif()
endif()
