file(
        GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/source/*cpp"
)

include(FetchContent)
add_executable(TempestEditor ${SOURCES})

FetchContent_Declare(
        glm
        GIT_REPOSITORY https://github.com/g-truc/glm.git
        GIT_TAG 1.0.3
)
FetchContent_Declare(
        ktx
        GIT_REPOSITORY https://github.com/KhronosGroup/KTX-Software.git
        GIT_TAG v4.4.2
)

SET(GLM_TEST_ENABLE OFF CACHE BOOL "GLM Build unit tests")
set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "Build shared libraries" FORCE)

FetchContent_MakeAvailable(glm)
FetchContent_MakeAvailable(ktx)

find_package(Python REQUIRED COMPONENTS Interpreter)
find_package(Vulkan REQUIRED)

target_link_libraries(TempestEditor
        PUBLIC glm::glm
        PUBLIC Tempest
)

target_include_directories(TempestEditor
        PUBLIC include
        PUBLIC ${CMAKE_CURRENT_BINARY_DIR}
)

add_custom_command(
    TARGET TempestEditor
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "Running post-build script..."
    COMMAND Python::Interpreter ${CMAKE_SOURCE_DIR}/BuildScripts/PostBuild.py $<TARGET_FILE_DIR:TempestEditor>
    VERBATIM
)