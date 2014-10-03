#include "MainWindow.h"

#include <QApplication>
#include <QMessageBox>

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);

	if (!QSystemTrayIcon::isSystemTrayAvailable()) {
		QMessageBox::critical(nullptr, "SafeDisk", "Could not detect the system tray on this OS.");
		return 1;
	}

	MainWindow window;
	return app.exec();
}
