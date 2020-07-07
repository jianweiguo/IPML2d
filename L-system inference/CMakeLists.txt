
project (IPML2d)

cmake_minimum_required(VERSION 2.8)

find_package(OpenGL)

find_package(BOOST)

if(Boost_FOUND)   
    include_directories(${Boost_INCLUDE_DIRS})  
    #target_link_libraries(foo ${Boost_LIBRARIES})
endif()

set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
find_package(Qt5Core REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5OpenGL REQUIRED)
find_package(Qt5Concurrent REQUIRED)
find_package(Qt5Test REQUIRED)
find_package(Qt5Gui REQUIRED)
find_package(Qt5PrintSupport REQUIRED)
find_package(Qt5Svg REQUIRED)

# Set Qt files and resources here
set(uis       resource/mainwindow.ui)
QT5_WRAP_UI(ui_srcs ${uis})

# add headers to project
file(GLOB MY_HEADERS "${PROJECT_SOURCE_DIR}/src/*.h")
list(APPEND MAINHEADERS ${MY_HEADERS})

file(GLOB_RECURSE R2_HEADERS "${PROJECT_SOURCE_DIR}/src/R2/*.h")
list(APPEND R2HEADERS ${R2_HEADERS})

file(GLOB_RECURSE R3_HEADERS "${PROJECT_SOURCE_DIR}/src/R3/*.h")
list(APPEND R3HEADERS ${R3_HEADERS})

# add sources to project
file(GLOB MY_SOURCES "${PROJECT_SOURCE_DIR}/src/*.cpp")
list(APPEND MAINSOURCES ${MY_SOURCES})

file(GLOB_RECURSE R2_SOURCES "${PROJECT_SOURCE_DIR}/src/R2/*.cpp")
list(APPEND R2SOURCES ${R2_SOURCES})

file(GLOB_RECURSE R3_SOURCES "${PROJECT_SOURCE_DIR}/src/R3/*.cpp")
list(APPEND R3SOURCES ${R3_SOURCES})

set(incs    ${MAINHEADERS})
set(srcs    ${MAINSOURCES})

set(incs    ${R2_HEADERS} ${R3_HEADERS})
set(srcs    ${R2_SOURCES} ${R3_SOURCES})

set(Headers     "Header Files")
set(Sources     "Source Files")

#set(header_main              ${Headers}\\main)
set(header_R2        		 ${Headers}\\R2)
set(header_R3                ${Headers}\\R3)

#set(source_main              ${Sources}\\main)
set(source_R2        	     ${Sources}\\R2)
set(source_R3       		 ${Sources}\\R3)

# Organize files
SOURCE_GROUP("Resources" FILES ${uis})
SOURCE_GROUP("Generated" FILES ${ui_srcs} ${moc_srcs})
SET_SOURCE_FILES_PROPERTIES(${srcs} PROPERTIES OBJECT_DEPENDS "${ui_srcs}")

SOURCE_GROUP(${header_R2}  FILES ${R2HEADERS})
SOURCE_GROUP(${header_R3} FILES ${R3HEADERS})

SOURCE_GROUP(${source_R2}  FILES ${R2SOURCES})
SOURCE_GROUP(${source_R3}  FILES ${R3SOURCES})


# Put the ui in the windows project file
IF (${CMAKE_BUILD_TOOL} MATCHES "msdev")
  SET (srcs ${srcs} ${uis})
ENDIF (${CMAKE_BUILD_TOOL} MATCHES "msdev")
IF (${CMAKE_BUILD_TOOL} MATCHES "devenv")
  SET (srcs ${srcs} ${uis})
ENDIF (${CMAKE_BUILD_TOOL} MATCHES "devenv")

set(exe_name IPML2d)
add_executable(${exe_name} ${ui_srcs} ${moc_srcs} ${MAINHEADERS} ${MAINSOURCES} ${srcs} ${incs})

target_link_libraries(${exe_name} 
	${GLUT_LIBRARY} ${OPENGL_LIBRARY}
	Qt5::Core Qt5::Widgets Qt5::OpenGL Qt5::Concurrent Qt5::Test Qt5::Gui Qt5::PrintSupport Qt5::Svg
)

# add_definitions(-D_USE_MATH_DEFINES)
# add_definitions(-DNOMINMAX)
# add_definitions(-DOM_STATIC_BUILD)

if(WIN32 AND MSVC)
   set_target_properties(${exe_name} PROPERTIES LINK_FLAGS_RELEASE /OPT:REF)
elseif(CMAKE_SYSTEMname STREQUAL "Darwin")
  if(NOT CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set_target_properties(${exe_name} PROPERTIES LINK_FLAGS -Wl)
  endif()
elseif(__COMPILER_PATHSCALE)
  set_target_properties(${exe_name} PROPERTIES LINK_FLAGS -mp)
else()
  set_target_properties(${exe_name} PROPERTIES LINK_FLAGS -Wl)
endif()

set_target_properties(${exe_name} PROPERTIES DEBUG_POSTFIX _debug)
set_target_properties(${exe_name} PROPERTIES RELEASE_POSTFIX _release)

