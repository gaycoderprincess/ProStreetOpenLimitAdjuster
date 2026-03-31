cmake_minimum_required(VERSION 3.27)
project(NFSPSOpenLimitAdjuster)

SET(CMAKE_SYSTEM_NAME Windows)

SET(CMAKE_C_COMPILER i686-w64-mingw32-gcc)
SET(CMAKE_CXX_COMPILER i686-w64-mingw32-g++)
SET(CMAKE_RC_COMPILER i686-w64-mingw32-windres)
SET(CMAKE_RANLIB i686-w64-mingw32-ranlib)

SET(CMAKE_CXX_STANDARD 20)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -masm=intel -s -static")

add_compile_definitions(IMGUI_USER_CONFIG="../../nya_imconfig.h" NYA_BACKEND_DX9 NYA_COMMON_NO_D3D NYA_NO_FONT)
add_library(NFSPSOpenLimitAdjuster SHARED main.cpp)
target_include_directories(NFSPSOpenLimitAdjuster PUBLIC ../nya-common)
target_include_directories(NFSPSOpenLimitAdjuster PUBLIC ../nya-common/3rdparty)
target_include_directories(NFSPSOpenLimitAdjuster PUBLIC ../nya-common-nfsps)
target_include_directories(NFSPSOpenLimitAdjuster PUBLIC ../CwoeeMenuLib/include)
target_link_options(NFSPSOpenLimitAdjuster PRIVATE "-Wl,--exclude-all-symbols")

set_target_properties(NFSPSOpenLimitAdjuster PROPERTIES PREFIX "")
set_target_properties(NFSPSOpenLimitAdjuster PROPERTIES SUFFIX "_gcp.asi")