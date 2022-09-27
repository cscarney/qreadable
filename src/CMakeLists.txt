# SPDX-FileCopyrightText: 2022 Connor Carney <hello@connorcarney.com>
# SPDX-License-Identifier: GPL-3.0-or-later

set(libqreadable_SRCS
    gumbovisitor.h
    gumbovisitor.cpp
    domsupport.h
    domsupport.cpp
    dombuilder.h
    dombuilder.cpp
    readable.h
    readable.cpp
    readability.qrc
    )

set(qreadable_SRCS
    main.cpp)

add_library(libqreadable ${libqreadable_SRCS})
target_link_libraries(libqreadable PRIVATE htmlparser Qt5::Core Qt5::Qml)
target_compile_options(libqreadable PRIVATE -fvisibility=hidden)
target_compile_definitions(libqreadable PRIVATE LIBQREADABLE=1)

if(NOT QREADABLE_LIB_ONLY)
add_executable(qreadable ${qreadable_SRCS})
target_link_libraries(qreadable libqreadable Qt5::Core Qt5::Qml Qt5::Network)
install(TARGETS qreadable DESTINATION bin)
endif()
