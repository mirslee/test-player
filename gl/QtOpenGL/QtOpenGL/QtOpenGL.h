#pragma once

#include <QtWidgets/QWidget>
#include "ui_QtOpenGL.h"
#include "GLContext.h"

class QtOpenGL : public QWidget
{
    Q_OBJECT

public:
    QtOpenGL(QWidget *parent = Q_NULLPTR);
	~QtOpenGL();

protected:
	void paintEvent(QPaintEvent *event);

private:
    Ui::QtOpenGLClass ui;

	GLContext*       glContext;
};
