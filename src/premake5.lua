workspace("TheHolyCompiler")
    location "../solution/"
    cppdialect "c++14"

    configurations {
        "Debug-Static",
        "Debug-Shared",
        "Release-Static",
        "Release-Shared"
    }

    platforms "x64"

    architecture "x64"

    callingconvention "FastCall"

    flags {
        "MultiProcessorCompile",
        "NoPCH"
    }

    filter("Debug-*")
        defines {
            "THC_DEBUG"
        }

        optimize "Off"
        inlining "Disabled"
    
    filter("Release-*")
        defines {
            "THC_RELEASE"
        }

        flags {
            "LinkTimeOptimization"
        }

        optimize "Speed"
        inlining "Auto"

    filter("*-Static")
        defines {
            "THC_BUILD_STATIC"
        }
    
    filter("*-Shared")
        defines {
            "THC_BUILD_SHARED"
        }

    filter ""

project("TheHolyCompiler-core")
    location "../solution/TheHolyCompiler-core/"
    targetdir "%{sln.location}/../bin/TheHolyCompiler-core/"
    objdir "%{sln.location}/../bin/TheHolyCompiler-core/intermediates/"

    files {
        "TheHolyCompiler-core/**.h",
        "TheHolyCompiler-core/**.cpp",
        "TheHolyCompiler-core/**.c"
    }

    filter("*-Static")
        kind "StaticLib"

    filter("*-Shared")
        kind "SharedLib"

    filter { "" }

project("TheHolyCompiler")
    removeconfigurations {
        "Debug-Shared",
        "Release-Shared"
    }

    kind "ConsoleApp"
    location "../solution/TheHolyCompiler/"
    targetdir "%{sln.location}/../bin/TheHolyCompiler/"
    objdir "%{sln.location}/../bin/TheHolyCompiler/intermediates/"
    filename "thc"

    dependson "TheHolyCompiler-core"

    files {
        "TheHolyCompiler/**.h",
        "TheHolyCompiler/**.cpp",
        "TheHolyCompiler/**.c"
    }


