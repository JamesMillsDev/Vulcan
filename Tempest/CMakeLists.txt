file(
        GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/include/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/source/*cpp"
)

include(FetchContent)
add_library(Tempest STATIC ${SOURCES})

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
        box3d
        GIT_REPOSITORY https://github.com/erincatto/box3d.git
        GIT_TAG v0.1.0
)
FetchContent_Declare(
        poly2tri
        GIT_REPOSITORY https://github.com/jhasse/poly2tri.git
        GIT_BRANCH master
)
FetchContent_Declare(
        stb
        GIT_REPOSITORY https://github.com/nothings/stb.git
        GIT_BRANCH master
)

SET(GLM_TEST_ENABLE OFF CACHE BOOL "GLM Build unit tests")

set(BUILD_SHARED_LIBS OFF CACHE INTERNAL "Build shared libraries" FORCE)
set(ASSIMP_BUILD_ZLIB ON CACHE INTERNAL "Build zlib" FORCE)
set(ASSIMP_BUILD_TESTS OFF CACHE INTERNAL "Build tests" FORCE)

FetchContent_MakeAvailable(glm)
FetchContent_MakeAvailable(nlohmann_json)
FetchContent_MakeAvailable(assimp)
FetchContent_MakeAvailable(vma)
FetchContent_MakeAvailable(glfw)
FetchContent_MakeAvailable(box3d)
FetchContent_MakeAvailable(poly2tri)
FetchContent_MakeAvailable(stb)

find_package(Vulkan REQUIRED)

target_link_libraries(Tempest
        PUBLIC glm::glm
        PUBLIC nlohmann_json
        PUBLIC assimp
        PUBLIC glfw
        PUBLIC Vulkan::Vulkan
        PUBLIC GPUOpen::VulkanMemoryAllocator
        PUBLIC box3d
        PUBLIC poly2tri
)

target_include_directories(Tempest
        PUBLIC "include"
        PUBLIC ${CMAKE_CURRENT_BINARY_DIR}
        PUBLIC ${poly2tri_SOURCE_DIR}
        PUBLIC ${Vulkan_INCLUDE_DIRS}
        PUBLIC ${vma_SOURCE_DIR}/include
        PUBLIC ${stb_SOURCE_DIR}
)