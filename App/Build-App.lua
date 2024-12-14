project "App"
   kind "ConsoleApp"
   language "C++"
   cppdialect "C++20"
   targetdir "Binaries/%{cfg.buildcfg}"
   staticruntime "off"

   files { "Source/**.h", "Source/**.cpp" }

   includedirs
   {
      "Source",

	  -- Include Core
	  "../Core/Source",
	-- Vulkan
	"../Dependencies/Vulkan/",
	-- GLFW
	"../Dependencies/GLFW/include",
	-- stb-master
	"../Dependencies/stb-master",
	-- tinyobjloader
	"../Dependencies/tinyobjloader-release"

   }

   links
   {
      "Core",
	"../Dependencies/Vulkan/Lib/**.lib",
	"../Dependencies/GLFW/lib-vc2022/glfw3.lib" 
}

   targetdir ("../Binaries/" .. OutputDir .. "/%{prj.name}")
   objdir ("../Binaries/Intermediates/" .. OutputDir .. "/%{prj.name}")

   filter "system:windows"
       systemversion "latest"
       defines { "WINDOWS" }

   filter "configurations:Debug"
       defines { "DEBUG" }
       runtime "Debug"
       symbols "On"

   filter "configurations:Release"
       defines { "RELEASE" }
       runtime "Release"
       optimize "On"
       symbols "On"

   filter "configurations:Dist"
       defines { "DIST" }
       runtime "Release"
       optimize "On"
       symbols "Off"
