set(IMGUI_SOURCES 
	"src/imconfig.h"
	"src/imgui.h"
	"src/imgui_internal.h"
	"src/imstb_rectpack.h"
	"src/imstb_textedit.h"
	"src/imstb_truetype.h"
	"src/backends/imgui_impl_glfw.h"
	"src/backends/imgui_impl_vulkan.h"
	"src/imgui.cpp"
	"src/imgui_demo.cpp"
	"src/imgui_draw.cpp"
	"src/imgui_tables.cpp"
	"src/imgui_widgets.cpp"
	"src/backends/imgui_impl_glfw.cpp"
	"src/backends/imgui_impl_vulkan.cpp"
)

include(FetchContent)
add_library(ImGui STATIC ${IMGUI_SOURCES})

FetchContent_Declare(
        glfw
        GIT_REPOSITORY https://github.com/glfw/glfw.git
        GIT_TAG 3.4
)

FetchContent_MakeAvailable(glfw)

find_package(Vulkan REQUIRED)

target_link_libraries(ImGui
        PUBLIC glfw
        PUBLIC Vulkan::Vulkan
)

target_include_directories(ImGui
		PUBLIC "src"
        PUBLIC ${Vulkan_INCLUDE_DIRS}
)