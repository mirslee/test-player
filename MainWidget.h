//
//  MainWidget.hpp
//  cvtest
//
//  Created by sz17112850M01 on 2018/7/31.
//

#ifndef MainWidget_hpp
#define MainWidget_hpp

#include "../MxPlayer/MxPlayer.h"
#include "MxPointer.h"
class MainWidget: public QWidget
{
    Q_OBJECT
public:
    MainWidget(QWidget* parent = 0);
    
protected:
    void resizeEvent(QResizeEvent* e);
    
protected slots:
    void btnPlayClicked();
    void btnPauseClicked();
    void btnStopClicked();
    
private:
    
private:
    QPushButton* m_pBtnPlay;
    QPushButton* m_pBtnPause;
    QPushButton* m_pBtnStop;
    
    CMxObjectPointer<MxPlayer> player;
};


#endif /* MainWidget_hpp */
