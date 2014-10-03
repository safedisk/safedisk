#include <QApplication>
#include <QQmlApplicationEngine>
#include <QMessageBox>
#include <QMenu>
#include <QAction>
#include <QSystemTrayIcon>

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);

	if (!QSystemTrayIcon::isSystemTrayAvailable()) {
		QMessageBox::critical(nullptr, "SafeDisk", "Could not detect system tray on this OS.");
		return 1;
	}

	QQmlApplicationEngine engine;
	engine.load(QUrl("qrc:/main.qml"));

	QObject* root = engine.rootObjects().at(0);

	QAction* minimizeAction = new QAction("Mi&nimize", root);
	root->connect(minimizeAction, SIGNAL(triggered()), root, SLOT(hide()));
	QAction *maximizeAction = new QAction(QObject::tr("Ma&ximize"), root);
	root->connect(maximizeAction, SIGNAL(triggered()), root, SLOT(showMaximized()));
	QAction *restoreAction = new QAction(QObject::tr("&Restore"), root);
	root->connect(restoreAction, SIGNAL(triggered()), root, SLOT(showNormal()));
	QAction* quitAction = new QAction("&Quit", root);
	root->connect(quitAction, SIGNAL(triggered()), &app, SLOT(quit()));

	QMenu* trayIconMenu = new QMenu();
	trayIconMenu->addAction(minimizeAction);
	trayIconMenu->addAction(maximizeAction);
	trayIconMenu->addAction(restoreAction);
	trayIconMenu->addSeparator();
	trayIconMenu->addAction(quitAction);

	QSystemTrayIcon* trayIcon = new QSystemTrayIcon(root);
	trayIcon->setContextMenu(trayIconMenu);
	trayIcon->setIcon(QIcon(":/images/heart.png"));
	trayIcon->show();

    return app.exec();
}
