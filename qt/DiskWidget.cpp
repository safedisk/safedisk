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
#include <QMessageBox>
#include <QFileDialog>
#include <QInputDialog>
#include <QStandardPaths>

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

DiskWidget::DiskWidget(QWidget* parent, const Disk& disk)
	: QObject(parent)
	, m_parent(parent)
	, m_disk(disk)
	, m_menu(new QMenu(disk.name(), parent))
{
	m_toggleAction = m_menu->addAction("", this, SLOT(onAction()));
	m_revealAction = m_menu->addAction("Reveal Folder", this, SLOT(onReveal()));

	switch (m_disk.state()) {
	case DiskState::Invalid:
		qFatal("Invalid DiskState");
		break;
	case DiskState::Missing:
		m_menu->setIcon(QIcon(":/images/glyphicons_199_ban.png"));
		m_toggleAction->setText("Locate");
		m_revealAction->setEnabled(false);
		break;
	case DiskState::Locked:
		m_menu->setIcon(QIcon(":/images/glyphicons_203_lock.png"));
		m_toggleAction->setText("Unlock");
		m_revealAction->setEnabled(false);
		break;
	case DiskState::Unlocked:
		m_menu->setIcon(QIcon(":/images/glyphicons_204_unlock.png"));
		m_toggleAction->setText("Lock");
		m_revealAction->setEnabled(true);
		break;
	}
}

QMenu* DiskWidget::menu() const
{
	return m_menu;
}

void DiskWidget::onAction()
{
	m_parent->raise();

	switch (m_disk.state()) {
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

void DiskWidget::onReveal()
{
	m_parent->raise();
	m_disk.openVolume();
}

void DiskWidget::locate()
{
	QString dirName = prompt("Locate SafeDisk");
	if (dirName.isEmpty()) {
		return;
	}

	if (!m_disk.link(QDir(dirName))) {
		QMessageBox::critical(m_parent, "SafeDisk", QString("Located disk does not match: \"%1\"").arg(m_disk.name()));
		return;
	}

	if (m_disk.state() == DiskState::Locked) {
		unlock();
	}
}

bool DiskWidget::unlock()
{
	QString title = QString("Unlock \"%1\"").arg(m_disk.name());
	bool ok;
	QString password = QInputDialog::getText(m_parent, title, "Password:", QLineEdit::Password, "", &ok);
	if (!ok || password.isEmpty()) {
		return false;
	}

	if (!m_disk.unlock(password)) {
		QMessageBox::critical(m_parent, "SafeDisk", QString("Could not unlock disk: \"%1\"").arg(m_disk.name()));
		return false;
	}

	m_disk.openVolume();
	return true;
}

void DiskWidget::lock()
{
	m_disk.lock();
}
