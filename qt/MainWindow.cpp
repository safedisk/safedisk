/*  SafeDisk
 *  Copyright (C) 2014 Frank Laub
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU Affero General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Affero General Public License for more details.
 *
 *  You should have received a copy of the GNU Affero General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "MainWindow.h"
#include "ui_About.h"
#include "CreateDiskDialog.h"

#include <QMessageBox>
#include <QApplication>

MainWindow::MainWindow()
	: m_aboutAction("About SafeDisk", this)
	, m_aboutQtAction("About Qt", this)
	, m_createAction("Create...", this)
	, m_attachAction("Attach...", this)
	, m_quitAction("Quit", this)
	, m_trayIconMenu(this)
	, m_trayIcon(this)
{
	setWindowFlags(windowType() | Qt::FramelessWindowHint);
	resize(0, 0);

	connect(&m_aboutAction, SIGNAL(triggered()), this, SLOT(onAbout()));
	connect(&m_aboutQtAction, SIGNAL(triggered()), QApplication::instance(), SLOT(aboutQt()));
	connect(&m_createAction, SIGNAL(triggered()), this, SLOT(onCreate()));
	connect(&m_attachAction, SIGNAL(triggered()), this, SLOT(onAttach()));
	connect(&m_quitAction, SIGNAL(triggered()), QApplication::instance(), SLOT(quit()));

	m_trayIconMenu.addAction(&m_aboutAction);
	m_trayIconMenu.addAction(&m_aboutQtAction);
	m_trayIconMenu.addSeparator();
	m_trayIconMenu.addAction(&m_createAction);
	m_trayIconMenu.addAction(&m_attachAction);
	m_trayIconMenu.addSeparator();
	m_disksSeparator = m_trayIconMenu.addSeparator();
	m_trayIconMenu.addAction(&m_quitAction);

	m_trayIcon.setContextMenu(&m_trayIconMenu);
	m_trayIcon.setIcon(QIcon(":/app"));
	m_trayIcon.show();

	connect(&m_trayIconMenu, SIGNAL(aboutToShow()), this, SLOT(refresh()));
}

void MainWindow::onAbout()
{
	QDialog dialog(this);
	Ui::AboutDialog ui;
	ui.setupUi(&dialog);
	dialog.exec();
}

void MainWindow::onCreate()
{
	raise();

	CreateDiskDialog dialog(this);
	int result = dialog.exec();
	if (result == QDialog::Accepted) {
		Disk* disk = Disk::create(
					dialog.storagePath(),
					dialog.volumeName(),
					dialog.password(),
					dialog.size());
		if (disk->state() == DiskState::Invalid) {
			QMessageBox::critical(this, "SafeDisk", QString("Could not create disk: \"%1\"").arg(dialog.volumeName()));
		}
		else {
			disk->openVolume();
		}
	}

	refresh();
}

void MainWindow::onAttach()
{
	raise();

	QString dirName = DiskWidget::prompt("Attach SafeDisk");
	if (dirName.isEmpty()) {
		return;
	}

	openDisk(dirName);
}

void MainWindow::openDisk(const QString& dirName)
{
	raise();

	Disk* disk = Disk::attach(QDir(dirName));
	DiskWidget widget(this, disk);

	if (disk->state() == DiskState::Locked) {
		widget.unlock();
	}
	else {
		disk->openVolume();
	}

	refresh();
}

void MainWindow::refresh()
{
	for (auto disk : m_disks) {
		m_trayIconMenu.removeAction(disk->menu()->menuAction());
		delete disk;
	}

	m_disks.clear();

	for (auto disk : Disk::fetch()) {
		DiskWidget* widget = new DiskWidget(this, disk);
		m_trayIconMenu.insertMenu(m_disksSeparator, widget->menu());
		m_disks.append(widget);
	}
}
