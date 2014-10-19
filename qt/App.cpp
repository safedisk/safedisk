#include "App.h"

#include <QFileOpenEvent>

App::App(int& argc, char** argv)
	: QApplication(argc, argv)
{
}

bool App::event(QEvent* evt)
{
	switch (evt->type()) {
	case QEvent::FileOpen:
		m_mainWindow.openDisk(static_cast<QFileOpenEvent*>(evt)->file());
		return true;
	default:
		return QApplication::event(evt);
	}
}

int App::run()
{
	m_mainWindow.show();
	return exec();
}
