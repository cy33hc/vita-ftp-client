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
set(VITA_TITLEID  "FTPCLIENT")
set(VITA_VERSION  "01.00")

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
  src/style.cpp
  src/ime_dialog.cpp
  src/net.cpp
  src/ftpclient.cpp
  src/vitaaudiolib.c
  src/gui.cpp
  src/windows.cpp
  src/actions.cpp
  src/updater.cpp
  src/sfo.cpp
  src/audio.c
  src/ogg.c
  src/sha1.c
  src/zip.c
  src/unzip.c
  src/ioapi.c
)

target_link_libraries(${PROJECT_NAME}
  debugnet
  vorbisfile
  vorbis
  ogg
  vita2d
  png
  imgui_vita2d
  stdc++
  c
  z
  SceCommonDialog_stub
  SceLibKernel_stub
  SceDisplay_stub
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
)
