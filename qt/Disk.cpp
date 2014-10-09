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
#include <QFileDialog>
#include <QApplication>
#include <QInputDialog>
#include <QStandardPaths>

// SafeDisk system structure:
// ${DataLocation}
//   ${DiskName}.disk
//     blocks
//     fuse
//     size
//     volume -> /Volumes/${VolumeName}

Disk::Disk(const QString& name, QWidget* parent)
	: QObject(parent)
	, m_name(name)
	, m_parent(parent)
	, m_bundleDir(systemPath(name))
{
	createMenu();
}

bool Disk::isMounted() const
{
	QFileInfo fi(volumePath());
	return fi.exists();
}

bool Disk::isValid() const
{
	return m_bundleDir.exists();
}

QString Disk::volumePath() const
{
	return m_bundleDir.filePath("volume");
}

bool Disk::runScript(const QString& scriptName, const QStringList& args, const QString& input)
{
	qDebug() << scriptName << args;

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
		qDebug() << script.readLine();
	}

	if (!(script.exitStatus() == QProcess::NormalExit && script.exitCode() == 0)) {
		qDebug() << "exitStatus:" << script.exitStatus() << "exitCode:" << script.exitCode();
		return false;
	}

	return true;
}

Disk* Disk::createDisk(
		QWidget* parent,
		const QString& dirName,
		const QString& name,
		const QString& password,
		uint64_t size)
{
	QDir dir(dirName);
	QDir diskDir(dir.filePath(name) + ".disk");

	QStringList args;
	args << diskDir.absolutePath();
	args << QString::number(size);

	if (!runScript("create_disk.sh", args, password)) {
		QMessageBox::critical(parent, "SafeDisk", QString("Could not create disk: \"%1\"").arg(name));
		return nullptr;
	}

	QFile::link(diskDir.absolutePath(), systemPath(name));

	return new Disk(name, parent);
}

Disk* Disk::attachDisk(QWidget* parent)
{
	QStringList paths = QStandardPaths::standardLocations(QStandardPaths::DesktopLocation);
	QString dirName = QFileDialog::getExistingDirectory(parent, "Attach SafeDisk", paths[0]);
	if (dirName.isEmpty()) {
		return nullptr;
	}

	QString name = QFileInfo(dirName).baseName();

	if (QDir(systemPath(name)).exists()) {
		QMessageBox::critical(parent, "SafeDisk", QString("A SafeDisk with the name \"%1\" already exists").arg(name));
		return nullptr;
	}

	QFile::link(dirName, systemPath(name));

	Disk* disk = new Disk(name, parent);
	if (!disk->mount()) {
		delete disk;
		QFile::remove(systemPath(name));
		return nullptr;
	}

	return disk;
}

bool Disk::mount()
{
	m_parent->raise();

	QString title = QString("Unlock \"%1\"").arg(m_name);
	bool ok;
	QString password = QInputDialog::getText(m_parent, title, "Password:", QLineEdit::Password, "", &ok);
	if (!ok) {
		return false;
	}

	QStringList args;
	args << m_bundleDir.absolutePath();

	if (!runScript("mount_disk.sh", args, password)) {
		QMessageBox::critical(m_parent, "SafeDisk", QString("Could not unlock disk: \"%1\"").arg(m_name));
		return false;
	}

	revealFolder();
	return true;
}

void Disk::unmount()
{
	QDir appDir(QApplication::applicationDirPath());
	QString scriptPath = appDir.filePath("unmount_disk.sh");
	QStringList args;
	args << volumePath();
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
	QFileInfoList disks = dataPath.entryInfoList(filters, QDir::Dirs | QDir::NoDotAndDotDot | QDir::System, QDir::Name);
	for (auto it = disks.cbegin(); it != disks.cend(); it++) {
		diskList.append(new Disk(it->baseName(), parent));
	}
	return diskList;
}

QString Disk::systemPath(const QString& name)
{
	QStringList paths = QStandardPaths::standardLocations(QStandardPaths::DataLocation);
	return QDir::cleanPath(paths[0] + "/" + name + ".disk");
}

bool Disk::collision(const QString& dirName, const QString& name)
{
	QString fullName = name + ".disk";
	return QDir(dirName).exists(fullName) ||
			QDir(systemPath(name)).exists() ||
			QFileInfo(systemPath(name)).isSymLink();
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
//	m_menu->addAction("Settings...", this, SLOT(displaySettings()));

	updateState();
}

void Disk::updateState()
{
	if (isValid()) {
		if (isMounted()) {
			m_menu->setIcon(QIcon(":/images/glyphicons_204_unlock.png"));
			m_toggleAction->setText("Lock");
//			m_toggleAction->setIcon(QIcon());
			m_revealAction->setEnabled(true);
		}
		else {
			m_menu->setIcon(QIcon(":/images/glyphicons_203_lock.png"));
			m_toggleAction->setText("Unlock");
	//		m_toggleAction->setIcon(QIcon(":/images/glyphicons_179_eject.png"));
			m_revealAction->setEnabled(false);
		}
	}
	else {
		m_menu->setIcon(QIcon(":/images/glyphicons_199_ban.png"));
		m_toggleAction->setText("Find");
//		m_toggleAction->setIcon(QIcon());
		m_revealAction->setEnabled(false);
	}
}

void Disk::toggleMount()
{
	if (isValid()) {
		if (isMounted()) {
			unmount();
		}
		else {
			mount();
		}
	}
	else {
		find();
	}

	updateState();
}

void Disk::find()
{
	QStringList paths = QStandardPaths::standardLocations(QStandardPaths::DesktopLocation);
	QString dirName = QFileDialog::getExistingDirectory(m_parent, "Find SafeDisk", paths[0]);
	if (dirName.isEmpty()) {
		return;
	}

	QFileInfo fi(m_bundleDir.absolutePath());
	QString old = fi.symLinkTarget();

	QFile::remove(m_bundleDir.absolutePath());
	QFile::link(dirName, m_bundleDir.absolutePath());

	if (!mount()) {
		QFile::remove(m_bundleDir.absolutePath());
		QFile::link(old, m_bundleDir.absolutePath());
	}
}

void Disk::displaySettings()
{
	m_parent->raise();
	QMessageBox::information(m_parent, "SafeDisk", "Settings");
}

void Disk::revealFolder()
{
	qDebug() << "reveal" << volumePath();

#if defined(Q_OS_OSX)
	QStringList args;
	args << volumePath();
	QProcess::startDetached("open", args);
#elif defined(Q_OS_WIN)
	QStringList args;
	args << "/select," << volumePath();
	QProcess::startDetached("explorer", args);
#else
#endif
}
