# Lib_Helper — CMake test target (PHASE 1, STEP 1)
# Mirrors Lib_Helper.vcxproj: static lib, MFC shared, MBCS, Win32.
# Sources listed explicitly (verified present via directory listing).
add_library(Lib_Helper STATIC
    StdAfx.cpp
    CompactFdi.Cpp
    HLibColorValue.cpp
    HLibDataConvert.cpp
    HLibTimeFunctions.cpp
    BmpButton.cpp
    Bubble.cpp
    CheckListBoxLog.cpp
    CJFlatComboBox.cpp
    CoolDialogBar.cpp
    DIB.cpp
    DxControlFunc.cpp
    EtcFunction.cpp
    flatedit.cpp
    GetDxVer.cpp
    HyperlinkStatic.cpp
    MacButtons.cpp
    MacProgressCtrl.cpp
    memdc.cpp
    MultiSlider.cpp
    ReportCtrl.cpp
    Separator.cpp
    StColor.cpp
    ToolBarEx.cpp
    WusDV_Util.cpp
    rol_clipboard.cpp
    StopWatch.cpp
)

target_include_directories(Lib_Helper PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

# Match .vcxproj: dynamic MFC + MultiByte charset
target_compile_definitions(Lib_Helper PUBLIC
    WIN32
    _LIB
)

if(MSVC)
    # Use MFC in shared DLL (matches <UseOfMfc>Dynamic</UseOfMfc>)
    set_target_properties(Lib_Helper PROPERTIES
        MFC_FLAG 2
    )
    target_compile_options(Lib_Helper PRIVATE
        /Yu"stdafx.h"
    )
    set_source_files_properties(StdAfx.cpp PROPERTIES
        COMPILE_FLAGS "/Yc\"stdafx.h\""
    )
endif()

set_target_properties(Lib_Helper PROPERTIES
    CXX_STANDARD 17
    CXX_STANDARD_REQUIRED ON
    CXX_EXTENSIONS OFF
)
