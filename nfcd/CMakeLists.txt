project( nfcd )

cmake_minimum_required(VERSION 2.6)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/src/cmake/modules)

# pkg-config modules
find_package( PkgConfig REQUIRED )
## LibNFC
pkg_check_modules( LIBNFC libnfc )
link_directories( ${LIBNFC_LIBRARY_DIRS} )
include_directories( ${LIBNFC_INCLUDE_DIRS} )

# Documentation
include(UseDoxygen)

## LibFreeFare
pkg_check_modules( LIBFREEFARE libfreefare )
link_directories( ${LIBFREEFARE_LIBRARY_DIRS} )
include_directories( ${LIBFREEFARE_INCLUDE_DIRS} )  

## LibNDEF
pkg_check_modules( LIBNDEF ndef )
link_directories( ${LIBNDEF_LIBRARY_DIRS} )
include_directories( ${LIBNDEF_INCLUDE_DIRS} )  

# Qt4
find_package( Qt4 REQUIRED )
set(QT_DONT_USE_QTGUI true)
set(QT_USE_QTDBUS true)

# Qt Includes
include(${QT_USE_FILE})

if(CMAKE_COMPILER_IS_GNU_CC)
  # Use C99 standard
  add_definitions(-std=c99)
  # Make sure we will not miss some warnings ;)
  add_definitions(-Wall -pedantic)
endif(CMAKE_COMPILER_IS_GNU_CC)

# Current Project Include
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
include_directories(${CMAKE_CURRENT_BINARY_DIR}/src)

add_subdirectory(src)

