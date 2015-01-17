#include "linplayer.h"
#include <QMessageBox>
#include "Log.h"


LinPlayer::LinPlayer(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	LogIns.Init("LinPlayer.log");
	CPlayCore::InitEnv();
	

	connect(ui.btn_Open, SIGNAL(clicked()), this, SLOT(ctrl_open()));

	connect(ui.btn_Play, SIGNAL(clicked()), this, SLOT(ctrl_play()));
	connect(ui.btn_Stop, SIGNAL(clicked()), this, SLOT(ctrl_stop()));
	connect(ui.btn_Previous, SIGNAL(clicked()), this, SLOT(ctrl_previous()));
	connect(ui.btn_Next, SIGNAL(clicked()), this, SLOT(ctrl_next()));

	connect(ui.btn_List, SIGNAL(clicked()), this, SLOT(ctrl_list()));
	connect(ui.btn_FullScreen, SIGNAL(clicked()), this, SLOT(ctrl_fullscreen()));

	connect(ui.btn_Sound, SIGNAL(clicked()), this, SLOT(ctrl_sound()));
}

LinPlayer::~LinPlayer()
{

}

void LinPlayer::ctrl_open(void)
{
	ctrl_stop();

	QString filter =tr( "");
	QString selFilter = tr("视频文件 (*.avi *.h264);;媒体文件（所有文件）(*.*)");
	strUrl = QFileDialog::getOpenFileName(this, "Select Media File", NULL, selFilter);

	ctrl_play();
}

void LinPlayer::ctrl_play(void)
{
	if (!strUrl.isEmpty())
	{
		player = new CPlayCore((HWND)ui.graphicsView->winId());
		player->play(strUrl.toStdString().c_str());
	}
}
	
void LinPlayer::ctrl_stop(void)
{
	
}
void LinPlayer::ctrl_previous(void)
{

}
void LinPlayer::ctrl_next(void)
{

}

void LinPlayer::ctrl_list(void)
{

}
void LinPlayer::ctrl_fullscreen(void)
{

}

void LinPlayer::ctrl_sound(void)
{

}
