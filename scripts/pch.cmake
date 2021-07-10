# Macro for setting up precompiled headers. Usage:
#
#   add_precompiled_header(target header.h [FORCEINCLUDE] [header_on_which_pch_depends header_on_which_pch_depends ...] )
#
# MSVC: A source file with the same name as the header must exist and
# be included in the target (E.g. header.cpp).
#
# MSVC: Add FORCEINCLUDE to automatically include the precompiled
# header file from every source file.
#
# GCC: The precompiled header is always automatically included from
# every header file.
#
# Notes:
#   ##   All dependencies of target must be defined at the point where
#        PCH is being added - this is required to correctly collect compile
#        options exported from dependencies.
#   ##   Unwrapping of generator expression is supported as long as there
#        are no multiple compile flags in single generator expression, i.e.
#        $<$<COFIG:DEBUG>:-flag> is supported, but $<$<COFIG:DEBUG>:-flag1 -flag2>
#        is not.

#
# Copyright (C) 2009-2013 Lars Christensen <larsch@belunktum.dk>
# Copyright (C) 2013-2016 Nenad Mikša <nenad.miksa@microblink.com>
#
# Permission is hereby granted, free of charge, to any person
# obtaining a copy of this software and associated documentation files
# (the ‘Software’), to deal in the Software without restriction,
# including without limitation the rights to use, copy, modify, merge,
# publish, distribute, sublicense, and/or sell copies of the Software,
# and to permit persons to whom the Software is furnished to do so,
# subject to the following conditions:
#
# The above copyright notice and this permission notice shall be
# included in all copies or substantial portions of the Software.
#
# THE SOFTWARE IS PROVIDED ‘AS IS’, WITHOUT WARRANTY OF ANY KIND,
# EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
# MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
# NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS
# BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
# ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
# CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.

macro( add_precompiled_header _targetName _input )
    get_filename_component( _inputWe ${_input} NAME_WE )
    set( pch_source ${_inputWe}.cpp )

    set( PCH_LOCAL_DEPENDS "")
    set( FORCEINCLUDE OFF )

    set( PCH_EXCLUDE "")
    set( FOUND_EXCLUDE OFF )

    foreach( arg ${ARGN} )
        if( arg MATCHES FORCEINCLUDE )
            set( FORCEINCLUDE ON )
        elseif( arg MATCHES EXCLUDE )
            set( FOUND_EXCLUDE ON )
        elseif( FOUND_EXCLUDE )
            list( APPEND PCH_EXCLUDE ${arg} )
        else()
            list( APPEND PCH_LOCAL_DEPENDS ${arg} )
        endif()
    endforeach()

    if( MSVC )
        get_target_property( sources ${_targetName} SOURCES )
        get_filename_component( PCH_NAME ${_input} NAME )
        set( _sourceFound FALSE )
        foreach( _source ${sources} )
            set( PCH_COMPILE_FLAGS "" )
            if( _source MATCHES \\.\(cc|cxx|cpp\)$ )
                get_filename_component( _sourceWe ${_source} NAME_WE )
                if( _sourceWe STREQUAL ${_inputWe} )
                    set( PCH_COMPILE_FLAGS "${PCH_COMPILE_FLAGS} /Yc${PCH_NAME}" )
                    set( _sourceFound TRUE )
                elseif(NOT _source IN_LIST PCH_EXCLUDE)
                    set( PCH_COMPILE_FLAGS "${PCH_COMPILE_FLAGS} /Yu${PCH_NAME}" )
                    if( FORCEINCLUDE )
                        set( PCH_COMPILE_FLAGS "${PCH_COMPILE_FLAGS} /FI${PCH_NAME}" )
                    endif( FORCEINCLUDE )
                endif()
                set_source_files_properties( ${_source} PROPERTIES COMPILE_FLAGS "${PCH_COMPILE_FLAGS}" )
            endif( _source MATCHES \\.\(cc|cxx|cpp\)$ )
        endforeach()
        if( NOT _sourceFound )
            message( FATAL_ERROR "A source file for ${_input} was not found. Required for MSVC builds." )
        endif( NOT _sourceFound )
    endif(MSVC)

    if( CMAKE_COMPILER_IS_GNUCXX OR ${CMAKE_CXX_COMPILER_ID} MATCHES "Clang" )
        # TODO excludes for GCC
        if( ${CMAKE_GENERATOR} STREQUAL "Xcode" )
            set_target_properties(${_targetName} PROPERTIES XCODE_ATTRIBUTE_GCC_PREFIX_HEADER ${_input} XCODE_ATTRIBUTE_GCC_PRECOMPILE_PREFIX_HEADER "Yes")
        else()
            add_library( ${_targetName}_PCH OBJECT ${_input} )
            set_source_files_properties( ${_input} PROPERTIES OBJECT_DEPENDS "${PCH_LOCAL_DEPENDS}" COMPILE_OPTIONS -xc++-header LANGUAGE "CXX" COMPILE_DEFINITIONS "USING_PCH" )

            get_target_property( dependencies ${_targetName} INTERFACE_LINK_LIBRARIES )
            target_link_libraries( ${_targetName}_PCH PRIVATE ${dependencies} )

            get_target_property( dependencies ${_targetName} LINK_LIBRARIES )
            target_link_libraries( ${_targetName}_PCH PRIVATE ${dependencies} )

            get_filename_component( _name ${_input} NAME )
            set( GCH_PATH "${CMAKE_CURRENT_BINARY_DIR}/${_targetName}-pch" )
            set( _output "${GCH_PATH}/${_name}.gch" )

            file( RELATIVE_PATH pch_relative_path ${CMAKE_CURRENT_SOURCE_DIR} ${_input} )
            # check if there are folder-ups - in that case CMake will generate absolute path within CMake.dir folder
            string( FIND "${pch_relative_path}" "../" pos_nix )
            string( FIND "${pch_relative_path}" "..\\" pos_win )
            if ( NOT ${pos_nix} EQUAL -1 OR NOT ${pos_win} EQUAL -1 )
                set( pch_relative_path ${_input} )
            endif()
            file( TO_NATIVE_PATH "${CMAKE_CURRENT_BINARY_DIR}/${CMAKE_FILES_DIRECTORY}/${_targetName}_PCH.dir/${pch_relative_path}.o" built_pch_path )

            if ( CMAKE_HOST_WIN32 )
                string( REGEX REPLACE "\\\\(.):" "\\\\\\1_" built_pch_path    "${built_pch_path}"    )
                string( REGEX REPLACE    "/(.):"    "/\\1_" pch_relative_path "${pch_relative_path}" )
                string( REPLACE       "//"       "/"        pch_relative_path "${pch_relative_path}" )
                file( TO_NATIVE_PATH "${_output}" _output )
                set( mklink del "${_output}" && mklink "${_output}" "${built_pch_path}" )
            else()
                set( mklink "${CMAKE_COMMAND}" -E create_symlink "${built_pch_path}" "${_output}" )
            endif()

            add_custom_target( ${_targetName}_PCH_symlink
                COMMAND
                    ${CMAKE_COMMAND} -E make_directory ${GCH_PATH}
                COMMAND
                    ${mklink}
                COMMENT
                    "Creating symbolic link to built precompiled header"
                VERBATIM
            )
            add_dependencies( ${_targetName}_PCH_symlink ${_targetName}_PCH )

            get_target_property( _target_include_dirs ${_targetName} INCLUDE_DIRECTORIES )
            if( _target_include_dirs )
                target_include_directories( ${_targetName}_PCH PRIVATE ${_target_include_dirs} )
            endif()

            get_target_property( _target_interface_include_dirs ${_targetName} INTERFACE_INCLUDE_DIRECTORIES )
            if( _target_interface_include_dirs )
                target_include_directories( ${_targetName}_PCH PRIVATE ${_target_interface_include_dirs} )
            endif()

            get_target_property( _target_system_include_dirs ${_targetName} INTERFACE_SYSTEM_INCLUDE_DIRECTORIES )
            if( _target_system_include_dirs )
                target_include_directories( ${_targetName}_PCH SYSTEM PRIVATE ${_target_system_include_dirs} )
            endif()

            get_target_property( _target_compile_definitions ${_targetName} COMPILE_DEFINITIONS )
            if( _target_compile_definitions )
                target_compile_definitions( ${_targetName}_PCH PRIVATE ${_target_compile_definitions} )
            endif()

            get_target_property( _target_compile_options ${_targetName} COMPILE_OPTIONS )
            if( _target_compile_options )
                target_compile_options( ${_targetName}_PCH PRIVATE ${_target_compile_options} )
            endif()

            get_target_property( _target_compile_features ${_targetName} COMPILE_FEATURES )
            if( _target_compile_features )
                target_compile_features( ${_targetName}_PCH PRIVATE ${_target_compile_features} )
            endif()

            get_target_property( _interface_target_compile_features ${_targetName} INTERFACE_COMPILE_FEATURES )
            if( _interface_target_compile_features )
                target_compile_features( ${_targetName}_PCH PRIVATE ${_interface_target_compile_features} )
            endif()

            ## Implementation note:
            ## We always build single PCH for target. However, some source files in target can have different compile flags.
            ## A right thing to do (TM) would be to generate PCH for each group of files with same flags. However, we do not
            ## do that. So, instead of including PCH globally on whole target, we will include PCH only on those sources that
            ## do not already have specific flags.
            ## Also, specific for C source files, we filter-out C sources with regex that matches only C++ files since
            ## generator expression $<COMPILE_LANGUAGE:CXX> works only for target_compile_options function.
            ##
            ##                                                                         Nenad Miksa (28.02.2017.)

            get_target_property( target_sources ${_targetName} SOURCES )
            foreach( source ${target_sources} )
                get_source_file_property( old_flags ${source} COMPILE_FLAGS )
                if( NOT old_flags AND ${source} MATCHES ".*\\.c.+" )
                    set_source_files_properties( "${source}" PROPERTIES COMPILE_FLAGS "-include ${GCH_PATH}/${_name} -Winvalid-pch" )
                endif()
            endforeach()
            add_dependencies( ${_targetName} ${_targetName}_PCH_symlink )
        endif()
    endif( CMAKE_COMPILER_IS_GNUCXX OR ${CMAKE_CXX_COMPILER_ID} MATCHES "Clang" )
endmacro()
