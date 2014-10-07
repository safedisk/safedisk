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

#include "Disk.h"

#include <QDir>
#include <QFile>
#include <QDebug>
#include <QProcess>
#include <QTextStream>
#include <QMessageBox>
#include <QApplication>
#include <QInputDialog>
#include <QStandardPaths>

// SafeDisk bundle structure:
// ${DataLocation}
//   ${VolumeName}.disk
//     blocks
//     fuse
//     size

Disk::Disk(const QString& name, QWidget* parent)
	: QObject(parent)
	, m_name(name)
	, m_parent(parent)
	, m_bundleDir(rootPath(name))
{
	createMenu();
}

bool Disk::runScript(const QString& scriptName, const QStringList& args, const QString& input, QStringList* output)
{
	QDir appDir(QApplication::applicationDirPath());
	QString scriptPath = appDir.filePath(scriptName);

	QProcess script;
	script.setWorkingDirectory(appDir.absolutePath());
	script.start(scriptPath, args);
	if (!script.waitForStarted()) {
		return false;
	}

	if (!input.isEmpty()) {
		QTextStream stream(&script);
		stream << input << "\n";
		stream.flush();
	}

	if (!script.waitForFinished()) {
		return false;
	}

	while (!script.atEnd()) {
		QString line = script.readLine();
		if (output) {
			output->append(line);
		}
		qDebug() << line;
	}

	if (!(script.exitStatus() == QProcess::NormalExit && script.exitCode() == 0)) {
		qDebug() << "exitStatus:" << script.exitStatus() << "exitCode:" << script.exitCode();
		return false;
	}

	return true;
}

Disk* Disk::createDisk(QWidget* parent, const QString& name, const QString& password, uint64_t size)
{
	QDir bundleDir(rootPath(name));
	qDebug() << "bundleDir:" << bundleDir.absolutePath();

	QStringList args;
	args << bundleDir.absolutePath();
	args << name;
	args << QString::number(size);

	if (!runScript("create_disk.sh", args, password, nullptr)) {
		QMessageBox::critical(parent, "SafeDisk", QString("Could not create disk: \"%1\"").arg(name));
		return nullptr;
	}

	return new Disk(name, parent);
}

bool Disk::mount(const QString& password)
{
	QStringList args;
	args << m_bundleDir.absolutePath();

	QStringList out;
	if (!runScript("mount_disk.sh", args, password, &out)) {
		QMessageBox::critical(m_parent, "SafeDisk", QString("Could not unlock disk: \"%1\"").arg(m_name));
		return nullptr;
	}

	m_volumePath = out.first().trimmed();
	return true;
}

void Disk::unmount()
{
	QDir appDir(QApplication::applicationDirPath());
	QString scriptPath = appDir.filePath("unmount_disk.sh");
	QStringList args;
	args << m_volumePath;
	QProcess::startDetached(scriptPath, args);
}

QList<Disk*> Disk::listDisks(QWidget* parent)
{
	QList<Disk*> diskList;
	QStringList paths = QStandardPaths::standardLocations(QStandardPaths::DataLocation);
	QDir::root().mkpath(paths[0]);
	QDir dataPath(paths[0]);
	QStringList filters;
	filters << "*.disk";
	QFileInfoList disks = dataPath.entryInfoList(filters, QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
	for (auto it = disks.cbegin(); it != disks.cend(); it++) {
		diskList.append(new Disk(it->baseName(), parent));
	}
	return diskList;
}

QString Disk::rootPath(const QString& name)
{
	QStringList paths = QStandardPaths::standardLocations(QStandardPaths::DataLocation);
	return QDir::cleanPath(paths[0] + "/" + name + ".disk");
}

bool Disk::exists(const QString& name)
{
	return !QStandardPaths::locate(QStandardPaths::DataLocation, name + ".disk", QStandardPaths::LocateDirectory).isEmpty();
}

QMenu* Disk::menu() const
{
	return m_menu;
}

void Disk::createMenu()
{
	m_menu = new QMenu(m_name, m_parent);

	m_toggleAction = m_menu->addAction("", this, SLOT(toggleMount()));
	m_revealAction = m_menu->addAction("Reveal Folder", this, SLOT(revealFolder()));
	m_menu->addAction("Settings...", this, SLOT(displaySettings()));

	updateState();
}

void Disk::updateState()
{
	if (m_isLocked) {
		m_menu->setIcon(QIcon(":/images/glyphicons_203_lock.png"));
		m_toggleAction->setText("Unlock");
//		m_toggleAction->setIcon(QIcon(":/images/glyphicons_179_eject.png"));
		m_revealAction->setEnabled(false);
	}
	else {
		m_menu->setIcon(QIcon(":/images/glyphicons_204_unlock.png"));
		m_toggleAction->setText("Lock");
		m_toggleAction->setIcon(QIcon());
		m_revealAction->setEnabled(true);
	}
}

void Disk::toggleMount()
{
	if (m_isLocked) {
		m_parent->raise();

		QString title = QString("Unlock \"%1\"").arg(m_name);
		bool ok;
		QString password = QInputDialog::getText(m_parent, title, "Password:", QLineEdit::Password, "", &ok);
		if (!ok) {
			return;
		}
		if (!mount(password)) {
			return;
		}
		revealFolder();
	}
	else {
		unmount();
	}

	m_isLocked = !m_isLocked;
	updateState();
}

void Disk::displaySettings()
{
	m_parent->raise();
	QMessageBox::information(m_parent, "SafeDisk", "Settings");
}

void Disk::revealFolder()
{
#if defined(Q_OS_OSX)
	QStringList args;
	args << m_volumePath;
	QProcess::startDetached("open", args);
#elif defined(Q_OS_WIN)
	QStringList args;
	args << "/select," << m_volumePath;
	QProcess::startDetached("explorer", args);
#else
#endif
}
