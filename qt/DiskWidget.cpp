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

#include "DiskWidget.h"

#include <QDebug>
#include <QProcess>
#include <QEventLoop>
#include <QMessageBox>
#include <QFileDialog>
#include <QInputDialog>
#include <QProgressBar>
#include <QStandardPaths>
#include <QProgressDialog>

QString DiskWidget::prompt(const QString& caption)
{
	QStringList paths = QStandardPaths::standardLocations(QStandardPaths::DesktopLocation);
	QFileDialog dialog(nullptr, caption);
	//	dialog.setFileMode(QFileDialog::Directory);
	//	dialog.setOption(QFileDialog::ShowDirsOnly);
	dialog.setFileMode(QFileDialog::ExistingFile);
	dialog.setNameFilter("*.disk");
	dialog.setDirectory(paths[0]);
	if (!dialog.exec()) {
		return "";
	}

	return dialog.selectedFiles().first();
}

DiskWidget::DiskWidget(QWidget* parent, Disk* disk)
	: QObject(parent)
	, m_parent(parent)
	, m_disk(disk)
	, m_menu(new QMenu(disk->name(), parent))
{
	m_disk->setParent(this);
	m_toggleAction = m_menu->addAction("", this, SLOT(onToggle()));
	m_openAction = m_menu->addAction("Open Volume", this, SLOT(onOpen()));
#if defined(Q_OS_WIN) || defined(Q_OS_MAC)
	m_revealAction = m_menu->addAction("Reveal Image", this, SLOT(onReveal()));
#endif
	m_removeAction = m_menu->addAction("Remove", this, SLOT(onRemove()));

	switch (m_disk->state()) {
	case DiskState::Invalid:
		qFatal("Invalid DiskState");
		break;
	case DiskState::Missing:
		m_menu->setIcon(QIcon(":/icons/ban"));
		m_toggleAction->setText("Locate");
		m_openAction->setEnabled(false);
#if defined(Q_OS_WIN) || defined(Q_OS_MAC)
		m_revealAction->setEnabled(false);
#endif
		break;
	case DiskState::Locked:
		m_menu->setIcon(QIcon(":/icons/locked"));
		m_toggleAction->setText("Unlock");
		m_openAction->setEnabled(false);
#if defined(Q_OS_WIN) || defined(Q_OS_MAC)
		m_revealAction->setEnabled(false);
#endif
		break;
	case DiskState::Unlocked:
		m_menu->setIcon(QIcon(":/icons/unlocked"));
		m_openAction->setEnabled(true);
		m_toggleAction->setText("Lock");
#if defined(Q_OS_WIN) || defined(Q_OS_MAC)
		m_revealAction->setEnabled(true);
#endif
		break;
	}
}

QMenu* DiskWidget::menu() const
{
	return m_menu;
}

void DiskWidget::onToggle()
{
	m_parent->raise();

	switch (m_disk->state()) {
	case DiskState::Invalid:
		qFatal("Invalid DiskState");
		break;
	case DiskState::Missing:
		locate();
		break;
	case DiskState::Locked:
		unlock();
		break;
	case DiskState::Unlocked:
		lock();
		break;
	}
}

void DiskWidget::onOpen()
{
	m_parent->raise();
	m_disk->openVolume();
}

void DiskWidget::onReveal()
{
	m_parent->raise();
	m_disk->revealImage();
}

void DiskWidget::onRemove()
{
	m_parent->raise();
	lock();
	m_disk->remove(false);
}

void DiskWidget::locate()
{
	QString dirName = prompt("Locate SafeDisk");
	if (dirName.isEmpty()) {
		return;
	}

	QDir dir(dirName);
	if (!m_disk->match(dir)) {
		QMessageBox::critical(m_parent, "SafeDisk", QString("Located disk does not match: \"%1\"").arg(m_disk->name()));
		return;
	}

	if (!m_disk->link(QDir(dirName))) {
		QMessageBox::critical(m_parent, "SafeDisk", QString("Could not attach disk: \"%1\"").arg(m_disk->name()));
		return;
	}

	if (m_disk->state() == DiskState::Locked) {
		unlock();
	}
}

bool DiskWidget::unlock()
{
	QString title = QString("Unlock \"%1\"").arg(m_disk->name());
	bool ok;
	QString password = QInputDialog::getText(m_parent, title, "Password:", QLineEdit::Password, "", &ok);
	if (!ok || password.isEmpty()) {
		return false;
	}

	bool result = false;
	QEventLoop loop;

	QProgressDialog progress("Unlocking SafeDisk...", "Cancel", 0, 100);
	progress.setWindowModality(Qt::WindowModal);
	progress.setMinimumDuration(500);
	progress.setValue(0);
	QProgressBar bar;
	bar.setRange(0, 0);
	progress.setBar(&bar);

	connect(&progress, &QProgressDialog::canceled, [this] () {
		m_disk->cancel();
	});

	auto conn1 = connect(m_disk, &Disk::unlocked, [&] () {
		loop.exit();
		result = true;
	});

	auto conn2 = connect(m_disk, &Disk::error, [&] (int /*exitCode*/) {
		progress.reset();
		QMessageBox::critical(m_parent, "SafeDisk", QString("Could not unlock disk: \"%1\"").arg(m_disk->name()));
		loop.exit();
		result = false;
	});

	m_disk->unlock(password);

	loop.exec();

	disconnect(conn1);
	disconnect(conn2);

	if (result) {
		m_disk->openVolume();
	}
	return result;
}

void DiskWidget::lock()
{
	QEventLoop loop;

	QProgressDialog progress("Locking SafeDisk...", "Cancel", 0, 100);
	progress.setWindowModality(Qt::WindowModal);
	progress.setMinimumDuration(500);
	progress.setValue(0);
	QProgressBar bar;
	bar.setRange(0, 0);
	progress.setBar(&bar);

	connect(&progress, &QProgressDialog::canceled, [this] () {
		m_disk->cancel();
	});

	auto conn1 = connect(m_disk, &Disk::locked, [&loop] () {
		loop.exit();
	});

	auto conn2 = connect(m_disk, &Disk::error, [&] (int /*exitCode*/) {
		progress.reset();
		QMessageBox::critical(m_parent, "SafeDisk", QString("Could not lock disk: \"%1\"").arg(m_disk->name()));
		loop.exit();
	});

	m_disk->lock();

	loop.exec();

	disconnect(conn1);
	disconnect(conn2);
}
