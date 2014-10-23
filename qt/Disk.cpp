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

#include <QUrl>
#include <QUuid>
#include <QDebug>
#include <QMultiMap>
#include <QMessageBox>
#include <QTextStream>
#include <QApplication>
#include <QStandardPaths>
#include <QDesktopServices>

// SafeDisk system structure:
// ${UserLocation}
//   ${DiskName}.disk
//     meta
//     file_N
// ${DataLocation}
//   ${GUID}
//     disk -> ${UserLocation}/${DiskName}.disk
//     fuse
//     volume -> /Volumes/${VolumeName}

static
QString readGuid(const QDir& dir)
{
	QFile file(dir.filePath("guid"));
	if (!file.open(QFile::ReadOnly)) {
		return "";
	}
	return QString(file.readAll()).trimmed();
}

Disk::Disk(const QDir& dir)
	: m_dir(new QDir(dir))
{
}

Disk::~Disk()
{
	delete m_dir;
}

QDir Disk::systemRoot()
{
	QStringList paths = QStandardPaths::standardLocations(QStandardPaths::DataLocation);
	QDir::root().mkpath(paths[0]);
	return QDir(paths[0]);
}

QDir Disk::makeSystemDir(QString guid)
{
	if (guid.isEmpty()) {
		guid = QUuid::createUuid().toString();
	}
	systemRoot().mkpath(guid);
	return QDir(systemRoot().filePath(guid));
}

QFileInfo Disk::diskLink() const
{
	if (!m_dir) {
		return QFileInfo();
	}
	return QFileInfo(m_dir->filePath("disk"));
}

QFileInfo Disk::volumeLink() const
{
	if (!m_dir) {
		return QFileInfo();
	}
	return QFileInfo(m_dir->filePath("volume"));
}

QDir Disk::fuseDir() const
{
	if (!m_dir) {
		return QDir();
	}
	return QDir(m_dir->filePath("fuse"));
}

QString Disk::diskPath() const
{
	return diskLink().symLinkTarget();
}

QString Disk::volumePath() const
{
	return volumeLink().symLinkTarget();
}

QString Disk::name() const
{
	return QFileInfo(diskPath()).baseName();
}

QString Disk::guid() const
{
	return QFileInfo(m_dir->absolutePath()).baseName();
}

DiskState Disk::state() const
{
	if (!m_dir) {
		return DiskState::Invalid;
	}
	if (!diskLink().exists()) {
		return DiskState::Missing;
	}
	if (!volumeLink().exists()) {
		return DiskState::Locked;
	}
	return DiskState::Unlocked;
}

bool Disk::collision(const QString& dirName, const QString& name)
{
	QDir storageDir(QDir(dirName).filePath(name) + ".disk");
	return storageDir.exists();
}

QList<Disk*> Disk::fetch()
{
	QMultiMap<QString, Disk*> temp;
	QStringList filters;
	QFileInfoList disks = systemRoot().entryInfoList(filters, QDir::Dirs | QDir::NoDotAndDotDot);
	for (auto it = disks.cbegin(); it != disks.cend(); it++) {
		Disk* disk = new Disk(it->absoluteFilePath());
		temp.insert(disk->name(), disk);
	}
	return temp.values();
}

void Disk::create(
		const QDir& dir,
		const QString& name,
		const QString& password,
		uint64_t size)
{
	m_dir = new QDir(makeSystemDir());
	QDir storageDir(dir.filePath(name) + ".disk");

	QStringList args;
	args << m_dir->absolutePath();
	args << storageDir.absolutePath();
	args << QString::number(size);

	link(storageDir);

	m_pendingScript = new Script("create_disk.sh", args, password);
	connect(m_pendingScript, &Script::finished, [this] (int exitCode, QProcess::ExitStatus exitStatus) {
		m_pendingScript->deleteLater();
		m_pendingScript = nullptr;

		if (exitStatus == QProcess::NormalExit && exitCode == 0) {
			emit created();
			return;
		}

		QDir storageDir(diskPath());
		storageDir.removeRecursively();
		m_dir->removeRecursively();
		delete m_dir;
		m_dir = nullptr;

		emit error(exitCode);
	});

	m_pendingScript->start();
}

void Disk::attach(const QDir& dir)
{
	QString guid = readGuid(dir);
	m_dir = new QDir(makeSystemDir(guid));
	link(dir);
}

bool Disk::match(const QDir& dir)
{
	if (!m_dir) {
		return false;
	}
	return readGuid(dir) == guid();
}

bool Disk::link(const QDir& dir)
{
	if (!m_dir) {
		return false;
	}

	QString linkName(diskLink().absoluteFilePath());
	QFile::remove(linkName);
	return QFile::link(dir.absolutePath(), linkName);
}

void Disk::unlock(const QString& password)
{
	if (!m_dir) {
		return;
	}

	if (m_pendingScript) {
		return;
	}

	QStringList args;
	args << m_dir->absolutePath();

	m_pendingScript = new Script("mount_disk.sh", args, password);
	connect(m_pendingScript, &Script::finished, [this] (int exitCode, QProcess::ExitStatus exitStatus) {
		m_pendingScript->deleteLater();
		m_pendingScript = nullptr;

		if (exitStatus == QProcess::NormalExit && exitCode == 0) {
			emit unlocked();
		}
		else {
			emit error(exitCode);
		}
	});

	m_pendingScript->start();
}

void Disk::lock()
{
	if (!m_dir) {
		return;
	}

	if (m_pendingScript) {
		return;
	}

	QDir appDir(QApplication::applicationDirPath());
	QString scriptPath = appDir.filePath("unmount_disk.sh");
	QStringList args;
	args << volumePath();

	m_pendingScript = new Script("unmount_disk.sh", args, "");
	connect(m_pendingScript, &Script::finished, [this] (int exitCode, QProcess::ExitStatus exitStatus) {
		m_pendingScript->deleteLater();
		m_pendingScript = nullptr;

		if (exitStatus == QProcess::NormalExit && exitCode == 0) {
			emit locked();
		}
		else {
			emit error(exitCode);
		}
	});

	m_pendingScript->start();
}

void Disk::cancel()
{
	if (!m_dir) {
		return;
	}

	if (m_pendingScript) {
		m_pendingScript->terminate();
	}
}

void Disk::openVolume()
{
	if (!m_dir) {
		return;
	}

	QString url = QString("file://%1").arg(volumePath());
	QDesktopServices::openUrl(QUrl(url, QUrl::TolerantMode));
}

void Disk::revealImage()
{
	revealFile(diskPath());
}

void Disk::remove(bool erase)
{
	if (!m_dir) {
		return;
	}

	if (erase) {
		QDir storageDir(diskPath());
		storageDir.removeRecursively();
	}

	m_dir->removeRecursively();
}

void Disk::revealFile(const QString& pathIn)
{
	QStringList args;
	// Mac, Windows support folder or file.
#if defined(Q_OS_WIN)
	QString param;
	if (!QFileInfo(pathIn).isDir()) {
		param = QLatin1String("/select,");
	}
	param += QDir::toNativeSeparators(pathIn);
	args << param;
	QProcess::startDetached("explorer", args);
#elif defined(Q_OS_MAC)
	args << QLatin1String("-e")
		 << QString::fromLatin1("tell application \"Finder\" to reveal POSIX file \"%1\"")
			.arg(pathIn);
	QProcess::execute(QLatin1String("/usr/bin/osascript"), args);
	args.clear();
	args << QLatin1String("-e")
		 << QLatin1String("tell application \"Finder\" to activate");
	QProcess::execute("/usr/bin/osascript", args);
#else
	Q_UNUSED(pathIn)
	// we cannot select a file here, because no file browser really supports it...
	QMessageBox::warning(nullptr, "SafeDisk", "Reveal not supported on this system");
	//	const QFileInfo fileInfo(pathIn);
	//	const QString folder = fileInfo.absoluteFilePath();
	//	const QString app = Utils::UnixUtils::fileBrowser(Core::ICore::instance()->settings());
	//	QProcess browser;
	//	const QString browserArgs = Utils::UnixUtils::substituteFileBrowserParameters(app, folder);
	//	qDebug() << browserArgs;
	//	bool success = browserProc.startDetached(browserArgs);
	//	const QString error = QString::fromLocal8Bit(browserProc.readAllStandardError());
	//	success = success && error.isEmpty();
	//	if (!success) {
	//		QMessageBox::warning(nullptr, "SafeDisk", "Reveal not supported on this system");
	//	}
#endif
}
