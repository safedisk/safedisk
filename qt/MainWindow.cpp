#include "MainWindow.h"
#include "MakeUnique.h"

#include <QApplication>
#include <QMessageBox>

MainWindow::MainWindow()
{
	m_createAction = make_unique<QAction>("Create SafeDisk", this);
	connect(m_createAction.get(), SIGNAL(triggered()), this, SLOT(createDisk()));

	m_restoreAction = make_unique<QAction>("&Restore", this);
	connect(m_restoreAction.get(), SIGNAL(triggered()), this, SLOT(restoreDisk()));

	m_quitAction = make_unique<QAction>("&Quit", this);
	connect(m_quitAction.get(), SIGNAL(triggered()), QApplication::instance(), SLOT(quit()));

	m_trayIconMenu = make_unique<QMenu>(this);
	m_trayIconMenu->addAction(m_createAction.get());
	m_trayIconMenu->addAction(m_restoreAction.get());
	m_trayIconMenu->addSeparator();
	m_trayIconMenu->addSeparator();
	m_trayIconMenu->addAction(m_quitAction.get());

	m_trayIcon = make_unique<QSystemTrayIcon>(this);
	m_trayIcon->setContextMenu(m_trayIconMenu.get());
	m_trayIcon->setIcon(QIcon(":/images/heart.png"));
	m_trayIcon->show();
}

void MainWindow::createDisk()
{
	QMessageBox::information(this, "SafeDisk", "Create Disk");
}

void MainWindow::restoreDisk()
{
}
