add_library(Lib_ZLib STATIC
    adler32.c
    compress.c
    crc32.c
    deflate.c
    gzio.c
    infback.c
    inffast.c
    inflate.c
    inftrees.c
    ioapi.c
    iowin32.c
    mztools.c
    trees.c
    uncompr.c
    unzip.c
    zip.c
    zutil.c
)

target_compile_definitions(Lib_ZLib PRIVATE
    WIN32
    _LIB
    _CRT_SECURE_NO_WARNINGS
    $<$<CONFIG:Debug>:_DEBUG>
    $<$<CONFIG:Release>:NDEBUG>
)

if(MSVC)
    target_compile_options(Lib_ZLib PRIVATE
        $<$<CONFIG:Debug>:/W3>
        $<$<CONFIG:Debug>:/Od>
        $<$<CONFIG:Release>:/W0>
        $<$<CONFIG:Release>:/O2>
        /MP
    )

    set_target_properties(Lib_ZLib PROPERTIES
        MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
    )
endif()
