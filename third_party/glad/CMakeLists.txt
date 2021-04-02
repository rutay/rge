

set(GLAD_SRC
    "src/glad.c"
    "include/glad/glad.h"
    "include/KHR/khrplatform.h"
)

set(GLAD_INCLUDE_DIR "include")

add_library(glad ${GLAD_SRC})

target_include_directories(glad PUBLIC ${GLAD_INCLUDE_DIR})
