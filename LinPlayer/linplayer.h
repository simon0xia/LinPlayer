#ifndef LINPLAYER_H
#define LINPLAYER_H

#include <QtWidgets/QMainWindow>
#include "ui_linplayer.h"

class LinPlayer : public QMainWindow
{
	Q_OBJECT

public:
	LinPlayer(QWidget *parent = 0);
	~LinPlayer();

private:
	Ui::LinPlayerClass ui;
};

#endif // LINPLAYER_H
