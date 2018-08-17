#include "QtOpenGL.h"

QtOpenGL::QtOpenGL(QWidget *parent)
	: QWidget(parent)
{
	ui.setupUi(this);

	glContext = new GLContext();
	HWND hWnd = (HWND)this->winId();
	glContext->setup(hWnd,GetDC(hWnd));
}

QtOpenGL::~QtOpenGL()
{
	delete glContext;
}

void QtOpenGL::paintEvent(QPaintEvent *event)
{
	QWidget::paintEvent(event);
	glClear(GL_COLOR_BUFFER_BIT);
	glClearColor(1, 0, 0, 1);
	glContext->swapBuffer();
}