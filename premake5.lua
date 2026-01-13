workspace "skygfx"
	configurations { "Debug", "Release" }
	location "build"
	startproject "skygfx"

	-- Resource definitions
	defines {
		"rsc_CompanyName=\"aap\"",
		"rsc_LegalCopyright=\"\"",
		"rsc_FileVersion=\"4.2.0.0\"",
		"rsc_ProductVersion=\"4.2.0.0\"",
		"rsc_InternalName=\"%{prj.name}\"",
		"rsc_ProductName=\"%{prj.name}\"",
		"rsc_OriginalFilename=\"%{prj.name}.dll\"",
		"rsc_FileDescription=\"https://github.com/aap\"",
		"rsc_UpdateUrl=\"https://github.com/aap/skygfx\""
	}

	-- Source files
	files {
		"external/*.*",
		"resources/*.*",
		"shaders/*.*",
		"src/*.*"
	}

	-- Include directories
	includedirs {
		"external/injector/include",
		"external",
		"resources",
		"shaders",
		"src"
	}

	-- RWSDK include path from environment variable
	local rwsdk = os.getenv("RWSDK36")
	if rwsdk then
		includedirs { rwsdk }
	end

	-- Shader compilation commands
	prebuildcommands {
		[[for /r "../shaders/ps/2_a/" %%f in (*.hlsl) do "fxc.exe" /T ps_2_a /nologo /E main /Fo ../resources/cso/%%~nf.cso %%f]],
		[[for /r "../shaders/ps/" %%f in (*.hlsl) do "fxc.exe" /T ps_2_a /nologo /E main /Fo ../resources/cso/%%~nf.cso %%f]],
		[[for /r "../shaders/vs/" %%f in (*.hlsl) do "fxc.exe" /T vs_2_0 /nologo /E main /Fo ../resources/cso/%%~nf.cso %%f]]
	}

project "skygfx"
	kind "SharedLib"
	language "C++"
	targetdir "bin/%{cfg.buildcfg}"
	targetextension ".dll"
	characterset "MBCS"

	buildoptions { "/Zc:threadSafeInit-" }

	-- Common settings for all configurations
	staticruntime "On"

	filter "configurations:Debug"
		defines { "DEBUG" }
		symbols "On"
		optimize "Debug"

	filter "configurations:Release"
		defines { "NDEBUG" }
		optimize "Full"
		linktimeoptimization "On"
