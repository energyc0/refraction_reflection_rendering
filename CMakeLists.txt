cmake_minimum_required (VERSION 3.28)

if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project ("graphics_rendering1")
set(CMAKE_BUILD_TYPE Debug)

add_executable (graphics_rendering1 
	"src/main.cpp"
	"externals/tinyobjloader/tiny_obj_loader.h"
	"externals/ImGui/imconfig.h"
	"externals/ImGui/imgui.cpp"
	"externals/ImGui/imgui.h"
	"externals/ImGui/imgui_demo.cpp" 
	"externals/ImGui/imgui_draw.cpp"
	"externals/ImGui/imgui_internal.h"
	"externals/ImGui/imgui_tables.cpp"
	"externals/ImGui/imgui_widgets.cpp"
	"externals/ImGui/imstb_rectpack.h"
	"externals/ImGui/imstb_textedit.h"
	"externals/ImGui/imstb_truetype.h"
	"externals/ImGui/backends/imgui_impl_glfw.cpp"
	"externals/ImGui/backends/imgui_impl_glfw.h"
	"externals/ImGui/backends/imgui_impl_vulkan.cpp" 
	"externals/ImGui/backends/imgui_impl_vulkan.h" 
	"src/App.h"
	"src/App.cpp"
	"src/VulkanUtilities.h"
	"src/VulkanUtilities.cpp"
	"src/Camera.h"
	"src/Camera.cpp"
	"src/Timer.h"
	"src/Timer.cpp"
	"src/RendererBase.h"
	"src/VulkanContext.h"
	"src/VulkanContext.cpp"
	"src/RendererBase.cpp"
	"src/CubeRenderer.h" 
	"src/CubeRenderer.cpp"
	"src/MeshRenderer.h" 
	"src/MeshRenderer.cpp"
	"src/ImGuiRenderer.h"
	"src/ImGuiRenderer.cpp")
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_CURRENT_LIST_DIR})

if (CMAKE_VERSION VERSION_GREATER 3.12)
  set_property(TARGET graphics_rendering1 PROPERTY CXX_STANDARD 20)
endif()
find_package(Vulkan REQUIRED)
if(NOT Vulkan_FOUND)
	message(FATAL_ERROR "FAILED TO FIND VULKAN!")
endif()
message(STATUS "Found Vulkan: $ENV{VULKAN_SDK}")

set(GLFW_PATH ${CMAKE_CURRENT_LIST_DIR}/externals/glfw)
set(glm_DIR "externals/glm")
if(NOT TINYOBJ_PATH)
	set(TINYOBJ_PATH externals/tinyobjloader)
	message(STATUS "TINYOBJ_PATH is not specified: ${TINYOBJ_PATH}")
endif()
set(STB_PATH "externals/stb-master")
set(IMGUI_PATH externals/ImGui)

target_include_directories(graphics_rendering1 PRIVATE 
	${Vulkan_INCLUDE_DIRS}
	${GLFW_PATH}/include
	${TINYOBJ_PATH}
	${IMGUI_PATH}
	${glm_DIR}
	${STB_PATH}
)
target_link_libraries(graphics_rendering1
	PRIVATE
	${Vulkan_LIBRARIES}
	${GLFW_PATH}/bin/glfw3.lib
)