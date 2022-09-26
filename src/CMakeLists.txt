# SPDX-FileCopyrightText: 2022 Connor Carney <hello@connorcarney.com>
# SPDX-License-Identifier: GPL-3.0-or-later

set(qreadability_SRCS
    gumbovisitor.h
    gumbovisitor.cpp
    domsupport.h
    domsupport.cpp
    dombuilder.h
    dombuilder.cpp
    main.cpp
    readability.qrc
    )

add_executable(qreadability ${qreadability_SRCS})

target_link_libraries(qreadability htmlparser Qt5::Core Qt5::Qml Qt5::Network)
