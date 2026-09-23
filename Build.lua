workspace "Wreck"
    architecture "x64"
    configurations { "Debug", "Release" }
    startproject "Wreck-App"

    output_bin = "%{wks.location}/Build/output"
    output_int = "%{wks.location}/Build/intermediate/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/%{prj.name}"

    filter "system:windows"
        systemversion "latest"
        buildoptions {"/utf-8"}

-- Third-party Projects
include "App/vendor/raylib/Raylib.lua"

-- Engine Projects
include "App/App.lua"
include "Compiler/Compiler.lua"