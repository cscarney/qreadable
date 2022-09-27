# SPDX-FileCopyrightText: 2022 Connor Carney <hello@connorcarney.com>
# SPDX-License-Identifier: GPL-3.0-or-later

set(libqreadability_SRCS
    gumbovisitor.h
    gumbovisitor.cpp
    domsupport.h
    domsupport.cpp
    dombuilder.h
    dombuilder.cpp
    readability.qrc
    )

set(qreadability_SRCS
    main.cpp)

add_library(libqreadability ${libqreadability_SRCS})
target_link_libraries(libqreadability PRIVATE htmlparser Qt5::Core Qt5::Qml)

add_executable(qreadability ${qreadability_SRCS})
target_link_libraries(qreadability libqreadability Qt5::Core Qt5::Qml Qt5::Network)
