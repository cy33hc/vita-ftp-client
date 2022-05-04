cmake_minimum_required(VERSION 2.8)

if(NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  if(DEFINED ENV{VITASDK})
    set(CMAKE_TOOLCHAIN_FILE "$ENV{VITASDK}/share/vita.toolchain.cmake" CACHE PATH "toolchain file")
  else()
    message(FATAL_ERROR "Please define VITASDK to point to your SDK path!")
  endif()
endif()

project(ftpclient)
include("${VITASDK}/share/vita.cmake" REQUIRED)

set(VITA_APP_NAME "Ftp Client")
set(VITA_TITLEID  "FTPCLI001")
set(VITA_VERSION  "01.30")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fpermissive")

include_directories(
  src
)

add_executable(${PROJECT_NAME}
  src/textures.cpp
  src/fs.cpp
  src/main.cpp
  src/inifile.c
  src/config.cpp
  src/ime_dialog.cpp
  src/net.cpp
  src/ftpclient.cpp
  src/vitaaudiolib.c
  src/gui.cpp
  src/windows.cpp
  src/actions.cpp
  src/updater.cpp
  src/lang.cpp
  src/sfo.cpp
  src/audio.c
  src/ogg.c
  src/sha1.c
  src/zip.c
  src/unzip.c
  src/ioapi.c
  src/debugScreen.c
)

target_link_libraries(${PROJECT_NAME}
  imgui
  vorbisfile
  vorbis
  ogg
  vita2d
  vitaGL
  vitashark
  mathneon
  png
  stdc++
  c
  z
  SceCommonDialog_stub
  SceLibKernel_stub
  SceDisplay_stub
  SceKernelDmacMgr_stub
  SceGxm_stub
  SceSysmodule_stub
  SceCtrl_stub
  SceTouch_stub
  SceAudio_stub
  SceHid_stub
  SceNet_stub
  SceNetCtl_stub
  SceHttp_stub
  SceSsl_stub
  ScePspnetAdhoc_stub
  ScePromoterUtil_stub
  SceShellSvc_stub
  m
  SceAppMgr_stub
  SceAppUtil_stub
  SceShaccCg_stub
)

vita_create_self(eboot.bin ${PROJECT_NAME} UNSAFE)
vita_create_vpk(${PROJECT_NAME}.vpk ${VITA_TITLEID} eboot.bin
  VERSION ${VITA_VERSION}
  NAME ${VITA_APP_NAME}
  FILE sce_sys/icon0.png sce_sys/icon0.png
  FILE sce_sys/livearea/contents/bg.png sce_sys/livearea/contents/bg.png
  FILE sce_sys/livearea/contents/startup.png sce_sys/livearea/contents/startup.png
  FILE sce_sys/livearea/contents/template.xml sce_sys/livearea/contents/template.xml
  FILE default_style.ini default_style.ini
  FILE head.bin head.bin
  FILE music.ogg music.ogg
  FILE icons/connect.png icons/connect.png
  FILE icons/disconnect.png icons/disconnect.png
  FILE icons/search.png icons/search.png
  FILE icons/refresh.png icons/refresh.png
  FILE icons/square.png icons/square.png
  FILE icons/circle.png icons/circle.png
  FILE icons/triangle.png icons/triangle.png
  FILE icons/cross.png icons/cross.png
  FILE icons/folder.png icons/folder.png
  FILE icons/file.png icons/file.png
  FILE icons/update.png icons/update.png
  FILE icons/catalog.png icons/catalog.png
  FILE lang/English.ini lang/English.ini
  FILE version.txt version.txt
)
