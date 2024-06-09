QT       += core gui sql
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
greaterThan(QT_MAJOR_VERSION, 5): QT += core5compat


CONFIG += c++17

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    fontsizedialog.cpp \
    form_mode_change.cpp \
    form_sql_config.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    fontsizedialog.h \
    form_mode_change.h \
    form_sql_config.h \
    sql_config.h \
    mainwindow.h

FORMS += \
    form_mode_change.ui \
    form_sql_config.ui \
    mainwindow.ui

RC_ICONS = myico.ico

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    res.qrc
