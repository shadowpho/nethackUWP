cmake_minimum_required(VERSION 3.4)
project(nethack)

set(NETHACK_SOURCES
  nethack_original/git_dump_3.6/src/allmain.c
  nethack_original/git_dump_3.6/src/alloc.c
  nethack_original/git_dump_3.6/src/apply.c
  nethack_original/git_dump_3.6/src/artifact.c
  nethack_original/git_dump_3.6/src/attrib.c
  nethack_original/git_dump_3.6/src/ball.c
  nethack_original/git_dump_3.6/src/bones.c
  nethack_original/git_dump_3.6/src/botl.c
  nethack_original/git_dump_3.6/src/cmd.c
  nethack_original/git_dump_3.6/src/dbridge.c
  nethack_original/git_dump_3.6/src/decl.c
  nethack_original/git_dump_3.6/src/detect.c
  nethack_original/git_dump_3.6/src/dig.c
  nethack_original/git_dump_3.6/src/display.c
  nethack_original/git_dump_3.6/src/dlb.c
  nethack_original/git_dump_3.6/src/do.c
  nethack_original/git_dump_3.6/src/dog.c
  nethack_original/git_dump_3.6/src/dogmove.c
  nethack_original/git_dump_3.6/src/dokick.c
  nethack_original/git_dump_3.6/src/dothrow.c
  nethack_original/git_dump_3.6/src/do_name.c
  nethack_original/git_dump_3.6/src/do_wear.c
  nethack_original/git_dump_3.6/src/drawing.c
  nethack_original/git_dump_3.6/src/dungeon.c
  nethack_original/git_dump_3.6/src/eat.c
  nethack_original/git_dump_3.6/src/end.c
  nethack_original/git_dump_3.6/src/engrave.c
  nethack_original/git_dump_3.6/src/exper.c
  nethack_original/git_dump_3.6/src/explode.c
  nethack_original/git_dump_3.6/src/extralev.c
  nethack_original/git_dump_3.6/src/files.c
  nethack_original/git_dump_3.6/src/fountain.c
  nethack_original/git_dump_3.6/src/hack.c
  nethack_original/git_dump_3.6/src/hacklib.c
  nethack_original/git_dump_3.6/src/invent.c
  nethack_original/git_dump_3.6/src/light.c
  nethack_original/git_dump_3.6/src/lock.c
  nethack_original/git_dump_3.6/src/mail.c
  nethack_original/git_dump_3.6/src/makemon.c
  nethack_original/git_dump_3.6/src/mapglyph.c
  nethack_original/git_dump_3.6/src/mcastu.c
  nethack_original/git_dump_3.6/src/mhitm.c
  nethack_original/git_dump_3.6/src/mhitu.c
  nethack_original/git_dump_3.6/src/minion.c
  nethack_original/git_dump_3.6/src/mklev.c
  nethack_original/git_dump_3.6/src/mkmap.c
  nethack_original/git_dump_3.6/src/mkmaze.c
  nethack_original/git_dump_3.6/src/mkobj.c
  nethack_original/git_dump_3.6/src/mkroom.c
  nethack_original/git_dump_3.6/src/mon.c
  nethack_original/git_dump_3.6/src/mondata.c
  nethack_original/git_dump_3.6/src/monmove.c
  nethack_original/git_dump_3.6/src/monst.c
  nethack_original/git_dump_3.6/src/monstr.c
  nethack_original/git_dump_3.6/src/mplayer.c
  nethack_original/git_dump_3.6/src/mthrowu.c
  nethack_original/git_dump_3.6/src/muse.c
  nethack_original/git_dump_3.6/src/music.c
  nethack_original/git_dump_3.6/src/objects.c
  nethack_original/git_dump_3.6/src/objnam.c
  nethack_original/git_dump_3.6/src/options.c
  nethack_original/git_dump_3.6/src/o_init.c
  nethack_original/git_dump_3.6/src/pager.c
  nethack_original/git_dump_3.6/src/pickup.c
  nethack_original/git_dump_3.6/src/pline.c
  nethack_original/git_dump_3.6/src/polyself.c
  nethack_original/git_dump_3.6/src/potion.c
  nethack_original/git_dump_3.6/src/pray.c
  nethack_original/git_dump_3.6/src/priest.c
  nethack_original/git_dump_3.6/src/quest.c
  nethack_original/git_dump_3.6/src/questpgr.c
  nethack_original/git_dump_3.6/src/read.c
  nethack_original/git_dump_3.6/src/rect.c
  nethack_original/git_dump_3.6/src/region.c
  nethack_original/git_dump_3.6/src/restore.c
  nethack_original/git_dump_3.6/src/rip.c
  nethack_original/git_dump_3.6/src/rnd.c
  nethack_original/git_dump_3.6/src/role.c
  nethack_original/git_dump_3.6/src/rumors.c
  nethack_original/git_dump_3.6/src/save.c
  nethack_original/git_dump_3.6/src/shk.c
  nethack_original/git_dump_3.6/src/shknam.c
  nethack_original/git_dump_3.6/src/sit.c
  nethack_original/git_dump_3.6/src/sounds.c
  nethack_original/git_dump_3.6/src/spell.c
  nethack_original/git_dump_3.6/src/sp_lev.c
  nethack_original/git_dump_3.6/src/steal.c
  nethack_original/git_dump_3.6/src/steed.c
  nethack_original/git_dump_3.6/src/sys.c
  nethack_original/git_dump_3.6/src/teleport.c
  nethack_original/git_dump_3.6/src/tile.c
  nethack_original/git_dump_3.6/src/timeout.c
  nethack_original/git_dump_3.6/src/topten.c
  nethack_original/git_dump_3.6/src/track.c
  nethack_original/git_dump_3.6/src/trap.c
  nethack_original/git_dump_3.6/src/uhitm.c
  nethack_original/git_dump_3.6/src/u_init.c
  nethack_original/git_dump_3.6/src/vault.c
  nethack_original/git_dump_3.6/src/version.c
  nethack_original/git_dump_3.6/src/vision.c
  nethack_original/git_dump_3.6/src/weapon.c
  nethack_original/git_dump_3.6/src/were.c
  nethack_original/git_dump_3.6/src/wield.c
  nethack_original/git_dump_3.6/src/windows.c
  nethack_original/git_dump_3.6/src/wizard.c
  nethack_original/git_dump_3.6/src/worm.c
  nethack_original/git_dump_3.6/src/worn.c
  nethack_original/git_dump_3.6/src/write.c
  nethack_original/git_dump_3.6/src/zap.c
  nethack_original/git_dump_3.6/sys/share/cppregex.cpp
  nethack_original/git_dump_3.6/sys/share/random.c
  nethack_original/git_dump_3.6/sys/uwp/uwp.cpp
  nethack_original/git_dump_3.6/win/tty/getline.c
  nethack_original/git_dump_3.6/win/tty/termcap.c
  nethack_original/git_dump_3.6/win/tty/topl.c
  nethack_original/git_dump_3.6/win/tty/wintty.c
)

include_directories(nethack_original/git_dump_3.6/include)

add_library(nethack_original STATIC ${NETHACK_SOURCES})

set(APP_MANIFEST NethackUWP/Package.appxmanifest)

set(CONTENT_FILES
  nethack_original/git_dump_3.6/dat/quest.dat
  nethack_original/git_dump_3.6/dat/dungeon
)
set(ASSET_FILES
  NethackUWP/Assets/LockScreenLogo.scale-200.png
  NethackUWP/Assets/SplashScreen.scale-200.png
  NethackUWP/Assets/Square150x150Logo.scale-200.png
  NethackUWP/Assets/Square44x44Logo.scale-200.png
  NethackUWP/Assets/Square44x44Logo.targetsize-24_altform-unplated.png
  NethackUWP/Assets/StoreLogo.png
  NethackUWP/Assets/Wide310x150Logo.scale-200.png
)

set(UWP_SOURCES
  NethackUWP/App.xaml.cpp
  NethackUWP/MainPage.xaml.cpp
  NethackUWP/pch.cpp
  NethackUWP/Package.appxmanifest
  NethackUWP/App.xaml.h
  NethackUWP/MainPage.xaml.h
  NethackUWP/App.xaml
  NethackUWP/MainPage.xaml
  )

    #     <DependentUpon>App.xaml</DependentUpon>
    # </ClCompile>
    #   <DependentUpon>MainPage.xaml</DependentUpon>
    # </ClCompile>

set_property(SOURCE NethackUWP/App.xaml PROPERTY VS_XAML_TYPE ApplicationDefinition)
set_property(SOURCE ${CONTENT_FILES} PROPERTY VS_DEPLOYMENT_CONTENT 1)
set_property(SOURCE ${ASSET_FILES} PROPERTY VS_DEPLOYMENT_CONTENT 1)
set_property(SOURCE ${ASSET_FILES} PROPERTY VS_DEPLOYMENT_LOCATION "Assets")
add_executable(NethackUWP ${UWP_SOURCES} ${CONTENT_FILES} ${ASSET_FILES} NethackUWP/NethackUWP_TemporaryKey.pfx)
target_include_directories(NethackUWP PRIVATE NethackUWP)
target_link_libraries(NethackUWP nethack_original)
set_property(TARGET NethackUWP PROPERTY VS_WINRT_COMPONENT TRUE)