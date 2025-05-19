# Template and Configs
TARGET = minesweeper
TEMPLATE = app

# Common Configs
CONFIG += c++20 strict_c++

# Qt Modules (Common)
QT += core gui widgets testlib
QT += core testlib

# Include Paths
INCLUDEPATH += . include/

#---------------------------------------------------------------------
# Solution (GUI) Configuration
# Активируется через CONFIG += solution
#---------------------------------------------------------------------
solution {
    # Disable console for GUI app
    CONFIG -= console
    CONFIG += qt

    # Sources and Headers
    SOURCES += src/main.cpp \
               src/mainwindow.cpp \
               src/MineSweeper.cpp \
               src/Save.cpp \
               src/TableState.cpp \
               src/ActiveDelegate.cpp \
               src/InactiveDelegate.cpp \
               src/SettingsDialog.cpp \
               src/TableView.cpp \
               src/TopWidget.cpp

    HEADERS += include/mainwindow.h \
               include/Constants.h \
               include/Preferences.h \
               include/GameField.h \
               include/MineSweeper.h \
               include/Save.h \
               include/TableState.h \
               include/ActiveDelegate.h \
               include/InactiveDelegate.h \
               include/SettingsDialog.h \
               include/TableView.h \
               include/TopWidget.h

    # Resources
    RESOURCES += images.qrc
    TRANSLATIONS += trans_pl.ts \
                    translations/ru_RU.ts \
                    translations/en_US.ts
}

#---------------------------------------------------------------------
# Tests Configuration
# Активируется через CONFIG += tests
#---------------------------------------------------------------------
tests {
    # Test-specific config
    CONFIG += console cmdline
    CONFIG -= app_bundle

    # Test Sources
    SOURCES += test/tests.cpp \
               src/MineSweeper.cpp \
               src/Save.cpp \
               src/TableState.cpp \
               src/TopWidget.cpp \
               src/ActiveDelegate.cpp \
               src/InactiveDelegate.cpp \
               src/TableView.cpp \
               src/SettingsDialog.cpp \
               src/mainwindow.cpp

    # Test Headers
    HEADERS += include/MineSweeper.h \
               include/GameField.h \
               include/Save.h \
               include/TableState.h \
               include/Constants.h \
               include/Preferences.h \
               include/mainwindow.h \
               include/SettingsDialog.h \
               include/TableView.h \
               include/ActiveDelegate.h \
               include/InactiveDelegate.h \
               include/TopWidget.h

    # GoogleTest Integration
    !isEmpty(GTEST_ROOT) {
        INCLUDEPATH += $$GTEST_ROOT/include
        LIBS += -L$$GTEST_ROOT/lib -lgtest -lgtest_main

        !win32 {
            LIBS += -lpthread
        }

        # Windows: Add DLL path
        win32:!static {
            LIBS += -L$$GTEST_ROOT/bin
            QMAKE_LFLAGS += /LIBPATH:$$GTEST_ROOT/bin
        }
    }
}

#---------------------------------------------------------------------
# Platform-specific Overrides
#---------------------------------------------------------------------
win32 {
    # Windows specific settings
    QMAKE_CXXFLAGS += /permissive-
    QMAKE_LFLAGS_RELEASE = /NOLOGO /INCREMENTAL:NO
}

unix {
    # Linux/MacOS settings
    QMAKE_CXXFLAGS += -Wall -Wextra
}

!win32 {
    QMAKE_CXXFLAGS += -Wno-deprecated-copy
}
