project "Wreck-App"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"
    staticruntime "off"

    targetdir (output_bin)
    objdir (output_int)

    files {
        "src/**.h",
        "src/**.cpp"
    }

    includedirs {
        "src",
        "vendor/raylib/src",
        "vendor/raylib/src/external/glfw/include"
    }
    
    links {
        "Raylib"
    }

    filter "system:windows"
        links {
            "opengl32.lib",
            "winmm.lib",
            "user32.lib"
        }
        linkoptions {
            "/FORCE:MULTIPLE"
        }

    filter "configurations:Debug"
        defines "APP_DEBUG"
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        defines "APP_RELEASE"
        runtime "Release"
        optimize "on"