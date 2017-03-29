project(XROMM-VR)

set(CMAKE_MODULE_PATH ${CMAKE_SOURCE_DIR}/cmake)

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
FIND_PACKAGE(FTGL REQUIRED )
FIND_PACKAGE(PNG REQUIRED )
FIND_PACKAGE(ZLIB REQUIRED )
FIND_PACKAGE(Freetype REQUIRED) # if it fails, check this:

message("-- GLM includes: " ${GLM_INCLUDE_DIR})
message("-- OpenGL includes: " ${OPENGL_INCLUDE_DIR})
message("-- OpenGL library: " ${OPENGL_LIBRARY})
message("-- GLEW library: " ${GLEW_LIBRARY})

include_directories(
  ${OPENGL_INCLUDE_DIR}
  ${MINVR_INCLUDE_DIR}
  ${GLM_INCLUDE_DIR}
  ${GLEW_INCLUDE_DIRS}
  ${FTGL_INCLUDE_DIR}
  )

# tgm
add_executable(XROMM-VR
  main.cpp
  VRMenu.h
  VRMenu.cpp
  VRMenuElement.h
  VRMenuElement.cpp
  VRButton.h
  VRButton.cpp
  VRTextBox.h
  VRTextBox.cpp
  VRMultiLineTextBox.h
  VRMultiLineTextBox.cpp
  VRGraph.cpp
  VRGraph.h
  tinyxml2.cpp
  VRFontHandler.cpp
  VRFontHandler.h
  VRMenuHandler.h
  VRToggle.h
  VRToggle.cpp
  XMAObject.cpp
  XMAObject.h
  glm.cpp
  glm.h
)

INCLUDE_DIRECTORIES(${FREETYPE_INCLUDE_DIRS})

target_link_libraries(XROMM-VR
  ${MINVR_LIBRARY}
  ${OPENGL_LIBRARY}
  ${GLEW_LIBRARY}
  ${FTGL_LIBRARIES}
  ${ZLIB_LIBRARIES}
  ${PNG_LIBRARIES}
  ${ALL_LIBS}
)

