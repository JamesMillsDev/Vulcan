file(
        GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/Source/inc/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/Source/src/*cpp"
)

include(FetchContent)
add_executable(ExampleGame ${SOURCES})

FetchContent_Declare(
        glm
        GIT_REPOSITORY https://github.com/g-truc/glm.git
        GIT_TAG 1.0.3
)
FetchContent_Declare(
        nlohmann_json
        GIT_REPOSITORY https://github.com/nlohmann/json.git
        GIT_TAG v3.12.0
)
FetchContent_Declare(
        assimp
        GIT_REPOSITORY https://github.com/assimp/assimp.git
        GIT_TAG v6.0.5
)
FetchContent_Declare(
        vma
        GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator.git
        GIT_TAG v3.4.0
)
FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG 3.4
)
FetchContent_Declare(
        ktx
        GIT_REPOSITORY https://github.com/KhronosGroup/KTX-Software.git
        GIT_TAG v4.4.2
)

SET(GLM_TEST_ENABLE OFF CACHE BOOL "GLM Build unit tests")

set(BUILD_SHARED_LIBS ON CACHE BOOL "Build shared libraries" FORCE)

set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "Build shared libraries" FORCE)
set(ASSIMP_BUILD_ZLIB ON CACHE INTERNAL "Build zlib" FORCE)
set(ASSIMP_BUILD_TESTS OFF CACHE INTERNAL "Build tests" FORCE)

FetchContent_MakeAvailable(glm)
FetchContent_MakeAvailable(nlohmann_json)
FetchContent_MakeAvailable(assimp)
FetchContent_MakeAvailable(vma)
FetchContent_MakeAvailable(glfw)
FetchContent_MakeAvailable(ktx)

find_package(Python REQUIRED COMPONENTS Interpreter)
find_package(Vulkan REQUIRED)

target_link_libraries(ExampleGame
        PUBLIC glm::glm
        PUBLIC nlohmann_json
        PUBLIC assimp
        PUBLIC glfw
        PUBLIC Vulkan::Vulkan
        PUBLIC GPUOpen::VulkanMemoryAllocator
        PUBLIC ktx
        PUBLIC Vulcan
)

target_precompile_headers(ExampleGame PRIVATE
        <string>
        <vector>
)

target_include_directories(ExampleGame
        PUBLIC Source/inc
        PUBLIC ${CMAKE_CURRENT_BINARY_DIR}
        PUBLIC ${Vulkan_INCLUDE_DIRS}
        PUBLIC ${vma_SOURCE_DIR}/include
)

add_custom_command(
    TARGET ExampleGame
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E echo "Running post-build script..."
    COMMAND Python::Interpreter ${CMAKE_SOURCE_DIR}/BuildScripts/PostBuild.py $<TARGET_FILE_DIR:ExampleGame>
    VERBATIM
)