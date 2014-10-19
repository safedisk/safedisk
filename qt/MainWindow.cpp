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
#include "CreateDiskDialog.h"

#include <QMessageBox>
#include <QApplication>

MainWindow::MainWindow()
{
	setWindowFlags(windowType() | Qt::FramelessWindowHint);
	resize(0, 0);

	m_createAction = new QAction("Create SafeDisk", this);
	connect(m_createAction, SIGNAL(triggered()), this, SLOT(createDisk()));

	m_attachAction = new QAction("Attach SafeDisk", this);
	connect(m_attachAction, SIGNAL(triggered()), this, SLOT(attachDisk()));

	m_quitAction = new QAction("&Quit", this);
	connect(m_quitAction, SIGNAL(triggered()), QApplication::instance(), SLOT(quit()));

	m_trayIconMenu = new QMenu(this);
	m_trayIconMenu->addAction(m_createAction);
	m_trayIconMenu->addAction(m_attachAction);
	m_trayIconMenu->addSeparator();
	m_disksSeparator = m_trayIconMenu->addSeparator();
	m_trayIconMenu->addAction(m_quitAction);

	m_trayIcon = new QSystemTrayIcon(this);
	m_trayIcon->setContextMenu(m_trayIconMenu);
	m_trayIcon->setIcon(QIcon(":/images/glyphicons_240_rotation_lock.png"));
	m_trayIcon->show();

	connect(m_trayIconMenu, SIGNAL(aboutToShow()), this, SLOT(refresh()));
}

void MainWindow::createDisk()
{
	raise();

	CreateDiskDialog dialog;
	int result = dialog.exec();
	if (result == QDialog::Accepted) {
		Disk disk = Disk::create(
					dialog.storagePath(),
					dialog.volumeName(),
					dialog.password(),
					dialog.size());
		if (disk.state() == DiskState::Invalid) {
			QMessageBox::critical(this, "SafeDisk", QString("Could not create disk: \"%1\"").arg(dialog.volumeName()));
		}
		else {
			disk.openVolume();
		}
	}

	refresh();
}

void MainWindow::attachDisk()
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

	Disk disk = Disk::attach(QDir(dirName));
	DiskWidget* widget = new DiskWidget(this, disk);

	if (disk.state() == DiskState::Locked) {
		widget->unlock();
	}
	else {
		disk.openVolume();
	}

	delete widget;

	refresh();
}

void MainWindow::refresh()
{
	for (auto disk : m_disks) {
		m_trayIconMenu->removeAction(disk->menu()->menuAction());
		delete disk;
	}

	m_disks.clear();

	for (auto disk : Disk::fetch()) {
		DiskWidget* widget = new DiskWidget(this, disk);
		m_trayIconMenu->insertMenu(m_disksSeparator, widget->menu());
		m_disks.append(widget);
	}
}
