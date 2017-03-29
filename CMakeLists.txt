project(XROMM-VR)

cmake_minimum_required (VERSION 2.8.2) 

if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE 
      Debug CACHE STRING
      "The type of build, options are: None Debug Release RelWithDebInfo MinSizeRel." 
      FORCE)
endif()

if (CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX 
      "${${PROJECT_NAME}_SOURCE_DIR}/build/install" CACHE PATH 
      "default install path" 
      FORCE)
endif()
message("-- Make install goes to: " ${CMAKE_INSTALL_PREFIX})


set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# To distinguish between debugging, release, and other builds, we'll
# add a postfix to the name of the lib or exe that we generate.

if(NOT CMAKE_DEBUG_POSTFIX)
  set(CMAKE_DEBUG_POSTFIX "d")
endif()
set(CMAKE_RELEASE_POSTFIX "")
set(CMAKE_RELWITHDEBINFO_POSTFIX "rd")

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DMinVR_DEBUG")

##
## You can set the MinVR location with
## -DMINVR_INSTALL_DIR=/my/location/of/MinVR/install
## Remember, you're looking for the install location, not the repo
## location.  The default install is at MINVR_REPO/build/install.
##
### Look for MinVR
if(MINVR_INSTALL_DIR)
  message("-- Using MinVR: " ${MINVR_INSTALL_DIR})
else(MINVR_INSTALL_DIR)
  message("MinVR install location not specified.  Using a default that is likely to be incorrect.")
  message("Use  -DMINVR_INSTALL_DIR=/my/location/of/MinVR/install to specify it.")
  set(MINVR_INSTALL_DIR "$ENV{HOME}/projects/MinVR-cmake-test/build/install" 
    CACHE PATH "The path of the MinVR *install* directory.")
endif(MINVR_INSTALL_DIR)
  
find_path(MINVR_INCLUDE_DIR
          NAME plugin
	  HINTS ${MINVR_INSTALL_DIR}/include)

find_library(MINVR_LIBRARY
          NAME MinVR
	  HINTS ${MINVR_INSTALL_DIR}/lib)
######### Found it and set a couple of variables.
#### Trust but verify
if(MINVR_INCLUDE_DIR)
  message("-- Found MinVR include files: " ${MINVR_INCLUDE_DIR})
else(MINVR_INCLUDE_DIR)
  message("Couldn't find MinVR include files.")
endif(MINVR_INCLUDE_DIR)

if(MINVR_LIBRARY)
  message("-- Found the MinVR library: " ${MINVR_LIBRARY})
else(MINVR_LIBRARY)
  message("Couldn't find the MinVR library.")
endif(MINVR_LIBRARY)
#### Done verification

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I${MINVR_INCLUDE_DIR}")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -I${MINVR_INCLUDE_DIR}")

# Build output directories.
make_directory(${CMAKE_BINARY_DIR}/bin)
set (CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if (${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  # Linux specific code
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
  set(CMAKE_CXX_FLAGS_DEBUG "-lsvml -lirc ${CMAKE_CXX_FLAGS_DEBUG}")
endif (${CMAKE_SYSTEM_NAME} MATCHES "Linux")

if (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
  # Apple-specific code
  set(CMAKE_CXX_FLAGS "-DOSX")
endif (${CMAKE_SYSTEM_NAME} MATCHES "Darwin")

if (WIN32)
  # Windows-specific
endif (WIN32)

#enable_testing()

#add_subdirectory(external)

set(img_src_dir ${CMAKE_CURRENT_SOURCE_DIR}/src)
add_subdirectory(src)
