add_library(Lib_BugTrap SHARED
    AboutDlg.cpp
    AnimProgressBar.cpp
    AssemblyInfo.cpp
    BugTrap.cpp
    BugTrapNet.cpp
    BugTrapUI.cpp
    BugTrapUtils.cpp
    CMapi.cpp
    ColHelper.cpp
    DescribeErrorDlg.cpp
    Encoding.cpp
    EnumProcess.cpp
    FileStream.cpp
    Globals.cpp
    HexView.cpp
    HyperLink.cpp
    ImageView.cpp
    InMemLogFile.cpp
    InputStream.cpp
    LayoutManager.cpp
    LogFile.cpp
    LogStream.cpp
    MachineInfoDlg.cpp
    MachineStateDlg.cpp
    MainDlg.cpp
    MemStream.cpp
    ModuleImportTable.cpp
    NetThunks.cpp
    OutputStream.cpp
    PreviewDlg.cpp
    ResManager.cpp
    SendMailDlg.cpp
    SimpleDlg.cpp
    Splitter.cpp
    StrHolder.cpp
    StrStream.cpp
    SymEngine.cpp
    SymEngineNet.cpp
    TextFormat.cpp
    TextLogFile.cpp
    TextView.cpp
    ThemeXP.cpp
    TransferProgressDlg.cpp
    WaitCursor.cpp
    WaitDlg.cpp
    XmlLogFile.cpp
    XmlReader.cpp
    XmlWriter.cpp
    BugTrap.rc
    stdafx.cpp
)

target_include_directories(Lib_BugTrap PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/res
    $<$<CONFIG:Debug>:
        ${CMAKE_CURRENT_SOURCE_DIR}/Sources
        ${CMAKE_SOURCE_DIR}/Lib_ZLib/Sources
    >
    $<$<CONFIG:Release>:
        ${CMAKE_SOURCE_DIR}/Lib_ZLib
    >
)

target_compile_definitions(Lib_BugTrap PRIVATE
    WIN32
    _WINDOWS
    _USRDLL
    BUGTRAP_EXPORTS
    _WINSOCK_DEPRECATED_NO_WARNINGS
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:Release>:NDEBUG>
)

if(MSVC)
    target_compile_options(Lib_BugTrap PRIVATE
        "SHELL:/Yu\"stdafx.h\""
        $<$<CONFIG:Debug>:/W3>
        $<$<CONFIG:Debug>:/Od>
        $<$<CONFIG:Release>:/W0>
        $<$<CONFIG:Release>:/O2>
        /MP
    )

    set_source_files_properties(stdafx.cpp PROPERTIES
        COMPILE_FLAGS "/Yc\"stdafx.h\""
    )

    set_target_properties(Lib_BugTrap PROPERTIES
        OUTPUT_NAME "$<$<CONFIG:Debug>:BugTrapD>$<$<CONFIG:Release>:BugTrap>"
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
        LINK_OPTIONS "/DEF:${CMAKE_CURRENT_SOURCE_DIR}/BugTrap.def"
    )

    target_link_libraries(Lib_BugTrap PRIVATE
        Lib_ZLib
        ws2_32.lib
        comctl32.lib
        shlwapi.lib
        version.lib
        wininet.lib
        nothrownew.obj
    )
endif()
