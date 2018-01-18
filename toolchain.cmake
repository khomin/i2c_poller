# CMake system name must be something like "Linux".
# This is important for cross-compiling.
set(CMAKE_SYSTEM_PROCESSOR )
set(CMAKE_C_COMPILER arm-dey-linux-gnueabi-gcc)
set(CMAKE_CXX_COMPILER arm-dey-linux-gnueabi-g++)
set(CMAKE_C_FLAGS "   --sysroot=/opt/dey/2.2-r3/sysroots/ccimx6ul-kbo  -march=armv7ve -marm -mfpu=neon  -mfloat-abi=hard -mcpu=cortex-a7 " CACHE STRING "CFLAGS")
set(CMAKE_CXX_FLAGS "   --sysroot=/opt/dey/2.2-r3/sysroots/ccimx6ul-kbo  -march=armv7ve -marm -mfpu=neon  -mfloat-abi=hard -mcpu=cortex-a7  -fpermissive" CACHE STRING "CXXFLAGS")
set(CMAKE_C_FLAGS_RELEASE "  --sysroot=/opt/dey/2.2-r3/sysroots/ccimx6ul-kbo  -march=armv7ve -marm -mfpu=neon  -mfloat-abi=hard -mcpu=cortex-a7  -DNDEBUG" CACHE STRING "CFLAGS for release")
set(CMAKE_CXX_FLAGS_RELEASE "  --sysroot=/opt/dey/2.2-r3/sysroots/ccimx6ul-kbo  -march=armv7ve -marm -mfpu=neon  -mfloat-abi=hard -mcpu=cortex-a7  -DNDEBUG" CACHE STRING "CXXFLAGS for release")
set(CMAKE_C_LINK_FLAGS "" CACHE STRING "LDFLAGS")
set(CMAKE_CXX_LINK_FLAGS "" CACHE STRING "LDFLAGS")

# only search in the paths provided so cmake doesnt pick
# up libraries and tools from the native build machine
set(CMAKE_FIND_ROOT_PATH /opt/dey/2.2-r3/sysroots/x86_64-deysdk-linux /opt/dey/2.2-r3/sysroots/ccimx6ul-kbo )
set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

# Use qt.conf settings
set(ENV{QT_CONF_PATH} qt.conf)

# We need to set the rpath to the correct directory as cmake does not provide any
# directory as rpath by default
set(CMAKE_INSTALL_RPATH )

# Use native cmake modules
set(CMAKE_MODULE_PATH /cmake/Modules/)

# add for non /usr/lib libdir, e.g. /usr/lib64
set(CMAKE_LIBRARY_PATH ${libdir} ${base_libdir})
