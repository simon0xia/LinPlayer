#include "linplayer.h"
#include <QMessageBox>
#include <QSettings>
#include "Log.h"

LinPlayer::LinPlayer(QWidget *parent)
: QMainWindow(parent), player(NULL)
{
	ui.setupUi(this);

	LogIns.Init("LinPlayer.log");
	CPlayCore::InitEnv();
	ReadSettings();

	ui.btn_Stop->setDisabled(true);
	ui.btn_Previous->setDisabled(true);
	ui.btn_Next->setDisabled(true);
	ui.btn_boost->setDisabled(true);
	ui.btn_reduce->setDisabled(true);
	
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
	//先停止当前正在播放内容，再弹出对话框供用户选择播放文件。
	ctrl_stop();

	//打开”选择文件“对话框时，直接切换到上次播放所在的路径。
	QString strPath = strUrl;
	if (!strPath.isEmpty())	{
		
		int index = strPath.lastIndexOf("/");
		strPath.truncate(index);
	}

	QString title = QStringLiteral("选择文件");
	QString selFilter = QStringLiteral("视频文件 (*.avi *.h264);;媒体文件（所有文件）(*.*)");
	strUrl = QFileDialog::getOpenFileName(this, title, strPath, selFilter);

	ctrl_play();
}

void LinPlayer::ctrl_play(void)
{
	if (player == nullptr)
	{
		//如果没有播放，则生成播放器，并播放strUrl指向的多媒体体文件。	
		ui.graphicsView->setVisible(true);
		if (!strUrl.isEmpty())
		{
			player = new CPlayCore((HWND)ui.graphicsView->winId());
			player->play(strUrl.toStdString().c_str());

			setStyleSheet_btn_play(player->isPause());
			ui.btn_Stop->setDisabled(false);
		}
	}
	else //if (player->isPlaying())
	{
		//如果正在播放，则暂停播放，并切换‘播放’钮钮的状态。
		player->pause();
		setStyleSheet_btn_play(player->isPause());
		return;
	}
}
	
void LinPlayer::setStyleSheet_btn_play(bool bPause)
{
	if (bPause)
	{
		ui.btn_Play->setStyleSheet("QPushButton{ border:none; border-image: url(:/LinPlayer/Resources/button.png)0 768 192 0;} "
								"QPushButton:hover   { border-image: url(:/LinPlayer/Resources/button.png) 64 768 128 0;} "
								"QPushButton:pressed { border-image: url(:/LinPlayer/Resources/button.png) 128 768 64 0;}"
								"QPushButton:disabled{ border-image: url(:/LinPlayer/Resources/button.png) 192 768 0 0;}");
	}
	else
	{
		ui.btn_Play->setStyleSheet("QPushButton{ border:none; border-image: url(:/LinPlayer/Resources/button.png)0 704 192 64;} "
								"QPushButton:hover   { border-image: url(:/LinPlayer/Resources/button.png) 64 704 128 64;}"
								"QPushButton:pressed { border-image: url(:/LinPlayer/Resources/button.png) 128 704 64 64;}"
								"QPushButton:disabled{ border-image: url(:/LinPlayer/Resources/button.png) 192 704 0 64;}");
	}
}

void LinPlayer::setStyleSheet_btn_Sound(bool bMute)
{
	if (bMute)
	{
		ui.btn_Sound->setStyleSheet("QPushButton{ border:none; border-image: url(:/LinPlayer/Resources/button.png)0 256 192 512;} "
								"QPushButton:hover   { border-image: url(:/LinPlayer/Resources/button.png) 64 256 128 512;}"
								"QPushButton:pressed { border-image: url(:/LinPlayer/Resources/button.png) 128 256 64 512;}"
								"QPushButton:disabled{ border-image: url(:/LinPlayer/Resources/button.png) 192 256 0 512;}");
	}
	else
	{
		ui.btn_Sound->setStyleSheet("QPushButton{ border:none; border-image: url(:/LinPlayer/Resources/button.png)0 192 192 512;} "
								"QPushButton:hover   { border-image: url(:/LinPlayer/Resources/button.png) 64 192 128 512;}"
								"QPushButton:pressed { border-image: url(:/LinPlayer/Resources/button.png) 128 192 64 512;}"
								"QPushButton:disabled{ border-image: url(:/LinPlayer/Resources/button.png) 192 192 0 512;}");
	}
}

void LinPlayer::ctrl_stop(void)
{
	if (player != nullptr)
	{
		player->stop();

		delete player;
		player = nullptr;

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
	player->setMute(!player->isMute());
	setStyleSheet_btn_Sound(player->isMute());
}

void LinPlayer::ReadSettings()
{
	QSettings set("./setting.ini", QSettings::IniFormat);
	
	strUrl = set.value("comm/preMedia", "").toString();
}