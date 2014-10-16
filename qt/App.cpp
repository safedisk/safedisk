#include "App.h"

#include <QMessageBox>
#include <QFileOpenEvent>

App::App(int& argc, char** argv)
	: QApplication(argc, argv)
{
}

bool App::event(QEvent* evt)
{
	switch (evt->type()) {
	case QEvent::FileOpen:
		openDisk(static_cast<QFileOpenEvent*>(evt)->file());
		return true;
	default:
		return QApplication::event(evt);
	}
}


void App::openDisk(const QString& filename)
{
	QMessageBox::information(nullptr, "SafeDisk", filename);
}
