#include "MainWindow.h"
#include "MakeUnique.h"

#include <QApplication>

MainWindow::MainWindow()
{
	m_minimizeAction = make_unique<QAction>("Mi&nimize", this);
	connect(m_minimizeAction.get(), SIGNAL(triggered()), this, SLOT(hide()));

	m_maximizeAction = make_unique<QAction>("Ma&ximize", this);
	connect(m_maximizeAction.get(), SIGNAL(triggered()), this, SLOT(showMaximized()));

	m_restoreAction = make_unique<QAction>("&Restore", this);
	connect(m_restoreAction.get(), SIGNAL(triggered()), this, SLOT(showNormal()));

	m_quitAction = make_unique<QAction>("&Quit", this);
	connect(m_quitAction.get(), SIGNAL(triggered()), QApplication::instance(), SLOT(quit()));

	m_trayIconMenu = make_unique<QMenu>(this);
	m_trayIconMenu->addAction(m_minimizeAction.get());
	m_trayIconMenu->addAction(m_maximizeAction.get());
	m_trayIconMenu->addAction(m_restoreAction.get());
	m_trayIconMenu->addSeparator();
	m_trayIconMenu->addAction(m_quitAction.get());

	m_trayIcon = make_unique<QSystemTrayIcon>(this);
	m_trayIcon->setContextMenu(m_trayIconMenu.get());
	m_trayIcon->setIcon(QIcon(":/images/heart.png"));
	m_trayIcon->show();
}
