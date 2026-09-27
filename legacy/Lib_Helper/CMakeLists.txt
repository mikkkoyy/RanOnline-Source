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

target_include_directories(Lib_Helper PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    $<$<CONFIG:Debug>:
        ${CMAKE_CURRENT_SOURCE_DIR}/Sources
        ${CMAKE_CURRENT_SOURCE_DIR}/Sources/DaumParam
        ${CMAKE_CURRENT_SOURCE_DIR}/Sources/ExLibs
        ${CMAKE_CURRENT_SOURCE_DIR}/Sources/RanOnlineLanchClipboard
        ${CMAKE_CURRENT_SOURCE_DIR}/../Lib_Engine/Sources/Common
        ${CMAKE_CURRENT_SOURCE_DIR}/../Lib_Engine/Sources/DxCommon
        ${CMAKE_CURRENT_SOURCE_DIR}/../Lib_Engine/Sources/DxCommon9
        ${CMAKE_CURRENT_SOURCE_DIR}/../Lib_Engine/Sources/DxTools
        ${CMAKE_CURRENT_SOURCE_DIR}/../Lib_Engine/Sources/G-Logic
        ${CMAKE_CURRENT_SOURCE_DIR}/../Dependency/common
        ${CMAKE_CURRENT_SOURCE_DIR}/../Dependency/directx
        ${CMAKE_CURRENT_SOURCE_DIR}/../Dependency/lzo
    >
    $<$<CONFIG:Release>:
        ${CMAKE_CURRENT_SOURCE_DIR}/../Lib_Engine/Common
        ${CMAKE_CURRENT_SOURCE_DIR}/../Lib_Engine/DxCommon
        ${CMAKE_CURRENT_SOURCE_DIR}/../Lib_Engine/DxCommon9
        ${CMAKE_CURRENT_SOURCE_DIR}/../Lib_Engine/DxFrame
        ${CMAKE_CURRENT_SOURCE_DIR}/../Lib_Engine/G-Logic
        ${CMAKE_CURRENT_SOURCE_DIR}/../Lib_Engine/GUInterface
        ${CMAKE_CURRENT_SOURCE_DIR}/../Lib_Engine/Utils
        ${CMAKE_CURRENT_SOURCE_DIR}/../Tik/DXInclude
        ${CMAKE_CURRENT_SOURCE_DIR}/../Tik/DXLib
        ${CMAKE_CURRENT_SOURCE_DIR}/../Tik/Include
        ${CMAKE_CURRENT_SOURCE_DIR}/../Tik/Library
        ${CMAKE_CURRENT_SOURCE_DIR}/../Tik/TbbLib
    >
)

target_compile_definitions(Lib_Helper PRIVATE
    WIN32
    _LIB
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:Release>:NDEBUG>
)

if(MSVC)
    set_target_properties(Lib_Helper PROPERTIES MFC_FLAG 2)

    target_compile_options(Lib_Helper PRIVATE
        "SHELL:/Yu\"stdafx.h\""
        $<$<CONFIG:Debug>:/W3>
        $<$<CONFIG:Debug>:/Od>
        $<$<CONFIG:Release>:/W0>
        $<$<CONFIG:Release>:/O2>
        /MP
    )

    set_source_files_properties(StdAfx.cpp PROPERTIES
        COMPILE_FLAGS "/Yc\"stdafx.h\""
    )
endif()
