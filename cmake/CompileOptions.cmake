#
# Platform and architecture setup
#
# Get upper case system name
string(TOUPPER ${CMAKE_SYSTEM_NAME} SYSTEM_NAME_UPPER)
message(STATUS "SYSTEM_NAME_UPPER: ${SYSTEM_NAME_UPPER} (CompileOptions.cmake)")

# Determine architecture (32/64 bit)
set(X64 OFF)
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(X64 ON)
endif()

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -D_DEBUG")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -D_DEBUG")

set(DEFAULT_PROJECT_OPTIONS
    DEBUG_POSTFIX             "-debug"
    RELEASE_POSTFIX           "-release"
    CXX_STANDARD              14
    LINKER_LANGUAGE           "CXX"
    POSITION_INDEPENDENT_CODE ON
    CXX_VISIBILITY_PRESET     "hidden"
    CXX_EXTENSIONS            Off
    ENABLE_EXPORTS            ON
    )

set(DEFAULT_INCLUDE_DIRECTORIES)
set(DEFAULT_LIBRARIES)

set(DEFAULT_COMPILE_DEFINITIONS
    SYSTEM_${SYSTEM_NAME_UPPER}
    SL_PROJECT_ROOT="${SL_PROJECT_ROOT}"
    SL_GIT_BRANCH="${GitBranch}"
    SL_GIT_COMMIT="${GitCommit}"
    SL_GIT_DATE="${GitDate}"
    )
message(STATUS "DEFAULT_COMPILE_DEFINITIONS: ${DEFAULT_COMPILE_DEFINITIONS}")

if (SL_BUILD_WAI)
	set(DEFAULT_COMPILE_DEFINITIONS
	    ${DEFAULT_COMPILE_DEFINITIONS}
	    SL_BUILD_WAI)
endif()

# MSVC compiler options
if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "MSVC")
    set(DEFAULT_COMPILE_DEFINITIONS ${DEFAULT_COMPILE_DEFINITIONS}
            _SCL_SECURE_NO_WARNINGS  # Calling any one of the potentially unsafe methods in the Standard C++ Library
            _CRT_SECURE_NO_WARNINGS  # Calling any one of the potentially unsafe methods in the CRT Library
            NOMINMAX #No min max makros (use the ones in std)
            )
    set(MSVC_COMPILE_FLAGS "/MP")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${MSVC_COMPILE_FLAGS}" )
endif ()

set(DEFAULT_COMPILE_OPTIONS)

# MSVC compiler options
if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "MSVC")
    set(DEFAULT_COMPILE_OPTIONS ${DEFAULT_COMPILE_OPTIONS}
        	/MP         # -> build with multiple processes
        	/W3         # -> warning level 3
        	/wd4251     # -> disable warning: 'identifier': class 'type' needs to have dll-interface to be used by clients of class 'type2'
        	/wd4592     # -> disable warning: 'identifier': symbol will be dynamically initialized (implementation limitation)
        	/wd4804     # -> disable warning: unsichere Verwendung des Typs "bool" in einer Operation	C:\Users\hsm4\Documents\GitHub\SLProject	C:\Users\hsm4\Documents\GitHub\SLProject\lib-SLProject\include\SLMaterial.h	88
        	/wd26495	# -> disable warning: C26495 MEMBER_UNINIT
			/wd26812	# -> disable warning: C26812: ' enum class ' vor ' ENUM ' bevorzugen (Enum. 3)
			/wd26451    # -> disable warning: C26451: Aritmetic overflow
			/bigobj

        $<$<CONFIG:Release>:
        /Gw           # -> whole program global optimization
        /GS-          # -> buffer security check: no
        /GL           # -> whole program optimization: enable link-time code generation (disables Zi)
        /GF           # -> enable string pooling
        >
        # No manual c++11 enable for MSVC as all supported MSVC versions for cmake-init have C++11 implicitly enabled (MSVC >=2013)
    )

	set(EXTERNAL_LIB_COMPILE_OPTIONS ${EXTERNAL_LIB_COMPILE_OPTIONS}
			/MP           # -> build with multiple processes
			#/W0           # -> warning level 0 all off
			#/w

			$<$<CONFIG:Release>:
			/Gw           # -> whole program global optimization
			/GS-          # -> buffer security check: no
			/GL           # -> whole program optimization: enable link-time code generation (disables Zi)
			/GF           # -> enable string pooling
			>
			# No manual c++11 enable for MSVC as all supported MSVC versions for cmake-init have C++11 implicitly enabled (MSVC >=2013)
			)
endif ()

# GCC and Clang compiler options
if ("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU" OR "${CMAKE_CXX_COMPILER_ID}" MATCHES "Clang")
    set(DEFAULT_COMPILE_OPTIONS ${DEFAULT_COMPILE_OPTIONS}

            -Werror=return-type
            -Wno-c++98-compat
            -Wno-c++98-compat-pedantic
            -Wno-covered-switch-default
            -Wno-double-promotion
            -Wno-exit-time-destructors
            -Wno-global-constructors
            -Wno-gnu-zero-variadic-macro-arguments
            -Wno-documentation
            -Wno-missing-variable-declarations
            -Wno-newline-eof
            -Wno-old-style-cast
            -Wno-switch-enum
            -Wno-unused-macros
            -Wno-unused-function
            -Wno-unused-parameter
            -Wno-unused-variable
            -Wno-unused-private-field
            -Wno-unused-value
            -Wno-used-but-marked-unused
            -Wno-extra-tokens
            -Wno-reorder
            -Wno-switch
            -Wno-char-subscripts
            -Wno-injected-class-name
            -Wno-format-security

            $<$<CXX_COMPILER_ID:GNU>:
                -Wmaybe-uninitialized
                $<$<VERSION_GREATER:$<CXX_COMPILER_VERSION>,4.8>:
                    -Wreturn-local-addr
                >
            >

            $<$<CXX_COMPILER_ID:Clang>:

            >

            $<$<PLATFORM_ID:Darwin>:
                -pthread
            >

            # Required for CMake < 3.1; should be removed if minimum required CMake version is raised.
            $<$<VERSION_LESS:${CMAKE_VERSION},3.1>:
                -std=c++14
            >
    )

	if ("${SYSTEM_NAME_UPPER}" STREQUAL "IOS")
		#iOS minimum supported version (deployment target version of iOS)
		set(DEPLOYMENT_TARGET 12.0)

		set(DEFAULT_PROJECT_OPTIONS
			${DEFAULT_PROJECT_OPTIONS}
			XCODE_ATTRIBUTE_IPHONEOS_DEPLOYMENT_TARGET ${DEPLOYMENT_TARGET}
		)

	    set(DEFAULT_COMPILE_OPTIONS ${DEFAULT_COMPILE_OPTIONS}
			-fobjc-arc #enable automatic reference counting
		)

		set(DEFAULT_COMPILE_DEFINITIONS
				${DEFAULT_COMPILE_DEFINITIONS}
				TARGET_OS_IOS #disable GLSLExtractor
				GLES_SILENCE_DEPRECATION
		)
		#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${MSVC_COMPILE_FLAGS} -fobjc-arc" )
	endif()

	# set correct architecture for MacOS (x86_64 or arm64): Needs cmake 3.19.2
	if("${SYSTEM_NAME_UPPER}" STREQUAL "DARWIN")
		set(CMAKE_OSX_ARCHITECTURES ${CMAKE_SYSTEM_PROCESSOR})
	endif()
endif ()

set(EXTERNAL_LIB_COMPILE_OPTIONS)

#
# Linker options
#

set(DEFAULT_LINKER_OPTIONS)

# Use pthreads on mingw and linux
if("${CMAKE_CXX_COMPILER_ID}" MATCHES "GNU" OR "${CMAKE_SYSTEM_NAME}" MATCHES "Linux")
    set(DEFAULT_LINKER_OPTIONS
        -pthread
    )
endif()
