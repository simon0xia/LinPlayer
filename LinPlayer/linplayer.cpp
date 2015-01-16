#include "linplayer.h"
#include <QMessageBox>
#include "Log.h"


LinPlayer::LinPlayer(QWidget *parent)
	: QMainWindow(parent)
{
	ui.setupUi(this);

	LogIns.Init("LinPlayer");
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
	QMessageBox::information(this, tr("Systray"),
		tr("The program will keep running in the "
		"system tray. To terminate the program, "
		"choose <b>Quit</b> in the context menu "
		"of the system tray entry."));
}

void LinPlayer::ctrl_play(void)
{
	char *url = "rtsp://218.204.223.237:554/live/1/66251FC11353191F/e7ooqwcfbqjoo80j.sdp";
	player = new CPlayCore((HWND)ui.graphicsView->winId());
	player->play(url);
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
