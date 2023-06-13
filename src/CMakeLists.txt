# ======================================================================================
#  __    ____  __  __  ____  ___
# (  )  (_  _)(  \/  )( ___)/ __)
#  )(__  _)(_  )    (  )__) \__ \
# (____)(____)(_/\/\_)(____)(___/
#
#  This file is part of the Limes open source library and is licensed under the terms of the GNU Public License.
#
#  Commercial licenses are available; contact the maintainers at ben.the.vining@gmail.com to inquire for details.
#
# ======================================================================================

target_sources (
    lfilesystem
    PRIVATE lfilesystem_CFile.cpp
            lfilesystem_Directory.cpp
            lfilesystem_DynamicLibrary.cpp
            lfilesystem_File.cpp
            lfilesystem_FilesystemEntry.cpp
            lfilesystem_Paths.cpp
            lfilesystem_Permissions.cpp
            lfilesystem_SimpleWatcher.cpp
            lfilesystem_SpecialDirs_Common.cpp
            lfilesystem_SymLink.cpp
            lfilesystem_Volume_Common.cpp)

if (IOS OR ANDROID OR EMSCRIPTEN)
    target_sources (lfilesystem PRIVATE native/lfilesystem_FileWatcher_Unsupported.cpp)
elseif (APPLE)
    target_sources (lfilesystem PRIVATE native/lfilesystem_FileWatcher_Mac.mm)
elseif (WIN32)
    target_sources (lfilesystem PRIVATE native/lfilesystem_FileWatcher_Windows.cpp)
else ()
    target_sources (lfilesystem PRIVATE native/lfilesystem_FileWatcher_Linux.cpp)
endif ()

if (APPLE)
    target_sources (
        lfilesystem PRIVATE native/lfilesystem_Entries_Mac.mm native/lfilesystem_SpecialDirs_Mac.mm
                            native/lfilesystem_Volume_Mac.mm)
elseif (WIN32)
    target_sources (
        lfilesystem
        PRIVATE native/lfilesystem_Entries_Windows.cpp native/lfilesystem_SpecialDirs_Windows.cpp
                native/lfilesystem_Volume_Windows.cpp)
else ()
    target_sources (lfilesystem PRIVATE native/lfilesystem_Entries_Linux.cpp
                                        native/lfilesystem_Volume_Linux.cpp)

    if (ANDROID)
        target_sources (lfilesystem PRIVATE native/lfilesystem_SpecialDirs_Android.cpp)
    else ()
        target_sources (lfilesystem PRIVATE native/lfilesystem_SpecialDirs_Linux.cpp)
    endif ()
endif ()
