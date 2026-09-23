project "Wreck-Compiler"
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
        "src"
    }

    filter "configurations:Debug"
        defines "COMP_DEBUG"
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        defines "COMP_RELEASE"
        runtime "Release"
        optimize "on"