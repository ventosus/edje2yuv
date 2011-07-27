cmake_minimum_required (VERSION 2.8)

project (edje2yuv)
set (CLI_LIBS "")

# pkg-config
find_package (PkgConfig) # ${PKG_CONFIG_FOUND}

# eina
pkg_search_module (EINA REQUIRED eina eina-1 eina-0)
include_directories (${EINA_INCLUDE_DIRS})
set (CLI_LIBS ${CLI_LIBS} ${EINA_LDFLAGS})

# ecore
pkg_search_module (ECORE REQUIRED ecore ecore-1 ecore-0)
include_directories (${ECORE_INCLUDE_DIRS})
set (CLI_LIBS ${CLI_LIBS} ${ECORE_LDFLAGS})

# ecore-evas
pkg_search_module (ECORE_EVAS REQUIRED ecore-evas ecore-evas-1 ecore-evas-0)
include_directories (${ECORE_EVAS_INCLUDE_DIRS})
set (CLI_LIBS ${CLI_LIBS} ${ECORE_EVAS_LDFLAGS})

# evas
pkg_search_module (EVAS REQUIRED evas evas-1 evas-0)
include_directories (${EVAS_INCLUDE_DIRS})
set (CLI_LIBS ${CLI_LIBS} ${EVAS_LDFLAGS})

# edje
pkg_search_module (EDJE REQUIRED edje edje-1 edje-0)
include_directories (${EDJE_INCLUDE_DIRS})
set (CLI_LIBS ${CLI_LIBS} ${EDJE_LDFLAGS})

# mjepgtools
pkg_search_module (MJPEGTOOLS REQUIRED mjpegtools)
include_directories (${MJPEGTOOLS_INCLUDE_DIRS})
set (CLI_LIBS ${CLI_LIBS} ${MJPEGTOOLS_LDFLAGS})

# libswscale
pkg_search_module (SWSCALE REQUIRED libswscale)
include_directories (${SWSCALE_INCLUDE_DIRS})
set (CLI_LIBS ${CLI_LIBS} ${SWSCALE_LDFLAGS})

add_subdirectory (src)
