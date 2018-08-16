#include "stdafx.h"
#include "MxVideoPlayer.h"
#include "MxGLView.h"

int main(int argc, char** argv)
{
    QApplication app(argc, argv);
    MxVideoPlayer player;
    QWidget w;
    QMacCocoaViewContainer w1(createGLView(), &w);
    w.show();
    player.open("/Users/ws/Desktop/123.mp4");
    app.exec();
    return 0;
}
