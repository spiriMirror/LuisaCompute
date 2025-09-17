file(GLOB_RECURSE LUISA_COMPUTE_MUDA_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp ${CMAKE_CURRENT_SOURCE_DIR}/*.h)

add_library(luisa-compute-muda STATIC ${LUISA_COMPUTE_MUDA_SOURCES})
target_link_libraries(luisa-compute-muda PUBLIC         
        luisa-compute-dsl
        luisa-compute-runtime
        luisa-compute-api
        luisa-compute-backends)

set_target_properties(luisa-compute-muda PROPERTIES
        UNITY_BUILD ${LUISA_COMPUTE_ENABLE_UNITY_BUILD}
        OUTPUT_NAME lc-muda)

luisa_compute_install(muda)
