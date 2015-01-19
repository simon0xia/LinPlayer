#include "linplayer.h"
#include <QMessageBox>
#include "Log.h"


LinPlayer::LinPlayer(QWidget *parent)
: QMainWindow(parent), player(NULL)
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

	ui.btn_Stop->setDisabled(true);
}

LinPlayer::~LinPlayer()
{

}

void LinPlayer::DisableAllButton()
{
	ui.btn_Open->setDisabled(true);
	ui.btn_Play->setDisabled(true);
	ui.btn_Stop->setDisabled(true);
	ui.btn_Previous->setDisabled(true);
	ui.btn_Next->setDisabled(true);
	ui.btn_FullScreen->setDisabled(true);
	ui.btn_Open->setDisabled(true);
	ui.btn_Open->setDisabled(true);
	ui.btn_Open->setDisabled(true);
}

void LinPlayer::ctrl_open(void)
{
	ctrl_stop();

	QString filter =tr( "" );
	QString selFilter = tr("视频文件 (*.avi *.h264);;媒体文件（所有文件）(*.*)");
	strUrl = QFileDialog::getOpenFileName(this, "Select Media File", NULL, selFilter);

	ctrl_play();
}

void LinPlayer::ctrl_play(void)
{
	if (player != NULL && player->isPlaying())
	{
		player->pause();
		return;
	}
	
	if (!strUrl.isEmpty())
	{
		player = new CPlayCore((HWND)ui.graphicsView->winId());
		player->play(strUrl.toStdString().c_str());

		ui.btn_Stop->setDisabled(false);
	}
}
	
void LinPlayer::ctrl_stop(void)
{
	if (player != NULL)
	{
		player->stop();
	}
	
	ui.btn_Stop->setDisabled(true);
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
	if (this->isFullScreen())
		this->showNormal();
	else
		this->showFullScreen();
}

void LinPlayer::ctrl_sound(void)
{

}
