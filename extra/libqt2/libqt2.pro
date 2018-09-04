include(../../JetsenNet2Base.pri)

CONFIG += staticlib
DESTDIR = $(JETSENNET2_PATH)/lib/$$OS_PLATFORM

win32 {
QMAKE_CFLAGS += /TP
}
else {
CONFIG -= precompile_header
QMAKE_CFLAGS += -x c++
}

CONFIG(debug, debug|release){
TARGET = qt2D
}else{
TARGET = qt2
}

HEADERS += \
    stdafx.h \
    workarounds.h \
    quicktime.h \
    qtprivate.h \
    lqt_version.h \
    lqt_qtvr.h \
    lqt_private.h \
    lqt_funcprotos.h \
    lqt_codecinfo.h \
    lqt_codecapi.h \
    lqt_atoms.h \
    lqt.h \
    libqt2.h \
    config.h \
    colormodels.h \
    charset.h

SOURCES += \
    workarounds.c \
    wave.c \
    vrsc.c \
    vrnp.c \
    vrni.c \
    vmhd.c \
    util.c \
    useratoms.c \
    udta.c \
    tref.c \
    translation.c \
    trak.c \
    tmcd.c \
    tkhd.c \
    timecode.c \
    texttrack.c \
    tcmi.c \
    sdtp.c \
    stts.c \
    stsz.c \
    stss.c \
    stsdtable.c \
    stsd.c \
    stsc.c \
    stps.c \
    stco.c \
    stbl.c \
    smhd.c \
    qtvr.c \
    qtatom.c \
    pHdr.c \
    pdat.c \
    pasp.c \
    pano.c \
    obji.c \
    nmhd.c \
    nloc.c \
    ndhd.c \
    navg.c \
    mvhd.c \
    multichannel.c \
    moov.c \
    minf.c \
    mdia.c \
    mdhd.c \
    mdat.c \
    matrix.c \
    lqt_quicktime.c \
    lqt_qtvr.c \
    log.c \
    language.c \
    iods.c \
    impn.c \
    imgp.c \
    hdlr.c \
    gmin.c \
    gmhd_text.c \
    gmhd.c \
    gama.c \
    ftyp.c \
    ftab.c \
    frma.c \
    fiel.c \
    esds.c \
    enda.c \
    elst.c \
    edts.c \
    dref.c \
    dinf.c \
    ctts.c \
    ctab.c \
    colr.c \
    colormodels.c \
    clap.c \
    charset.c \
    chan.c \
    audio.c \
    atom.c
