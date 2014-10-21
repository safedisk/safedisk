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
	file.open(QFile::ReadOnly);
	return QString(file.readAll()).trimmed();
}

Disk::Disk(const QDir& dir)
	: m_dir(dir)
{
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
	return QFileInfo(m_dir.filePath("disk"));
}

QFileInfo Disk::volumeLink() const
{
	return QFileInfo(m_dir.filePath("volume"));
}

QDir Disk::fuseDir() const
{
	return QDir(m_dir.filePath("fuse"));
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
	return QFileInfo(m_dir.absolutePath()).baseName();
}

DiskState Disk::state() const
{
	if (!m_dir.exists()) {
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

Disk* Disk::create(
		const QDir& dir,
		const QString& name,
		const QString& password,
		uint64_t size)
{
	QDir storageDir(dir.filePath(name) + ".disk");
	QDir systemDir(makeSystemDir());

	QStringList args;
	args << systemDir.absolutePath();
	args << storageDir.absolutePath();
	args << QString::number(size);

	if (!runScript("create_disk.sh", args, password)) {
		systemDir.removeRecursively();
		storageDir.removeRecursively();
	}

	return new Disk(systemDir);
}

Disk* Disk::attach(const QDir& dir)
{
	QString guid = readGuid(dir);
	Disk* disk = new Disk(makeSystemDir(guid));
	disk->link(dir);
	return disk;
}

bool Disk::link(const QDir& dir)
{
	if (readGuid(dir) != guid()) {
		return false;
	}
	QString linkName(diskLink().absoluteFilePath());
	QFile::remove(linkName);
	return QFile::link(dir.absolutePath(), linkName);
}

bool Disk::unlock(const QString& password)
{
	QStringList args;
	args << m_dir.absolutePath();

	if (!runScript("mount_disk.sh", args, password)) {
		return false;
	}

	return true;
}

void Disk::lock()
{
	if (m_pendingProcess) {
		return;
	}

	QDir appDir(QApplication::applicationDirPath());
	QString scriptPath = appDir.filePath("unmount_disk.sh");
	QStringList args;
	args << volumePath();

	m_pendingProcess = new QProcess(this);

	connect(m_pendingProcess,
			static_cast<void (QProcess::*)(int, QProcess::ExitStatus)>(&QProcess::finished),
			[&] (int exitCode, QProcess::ExitStatus exitStatus) {
		qDebug() << "unmount_disk.sh> exitCode:" << exitCode << "exitStatus:" << exitStatus;
		m_pendingProcess->deleteLater();
		m_pendingProcess = nullptr;
		emit locked();
	});

	m_pendingProcess->start(scriptPath, args);
}

void Disk::cancel()
{
	if (m_pendingProcess) {
		m_pendingProcess->terminate();
	}
}

bool Disk::runScript(const QString& scriptName, const QStringList& args, const QString& input)
{
	qDebug() << scriptName << args;

	QDir appDir(QApplication::applicationDirPath());
	QString scriptPath = appDir.filePath(scriptName);

	QProcess script;
	script.setWorkingDirectory(appDir.absolutePath());
	script.setProcessChannelMode(QProcess::ForwardedChannels);
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

	if (!(script.exitStatus() == QProcess::NormalExit && script.exitCode() == 0)) {
		qDebug() << "exitStatus:" << script.exitStatus() << "exitCode:" << script.exitCode();
		return false;
	}

	return true;
}

void Disk::openVolume()
{
	QString url = QString("file://%1").arg(volumePath());
	QDesktopServices::openUrl(QUrl(url, QUrl::TolerantMode));
}

void Disk::revealImage()
{
	revealFile(diskPath());
}

void Disk::remove(bool erase)
{
	if (erase) {
		QDir diskDir(diskPath());
		diskDir.removeRecursively();
	}

	m_dir.removeRecursively();
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
