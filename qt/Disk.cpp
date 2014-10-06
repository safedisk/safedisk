#include "Disk.h"

#include <QDir>
#include <QProcess>
#include <QMessageBox>
#include <QInputDialog>
#include <QStandardPaths>

Disk::Disk(const QString& name, QWidget* parent)
	: QObject(parent)
	, m_name(name)
	, m_parent(parent)
{
	m_rootPath = QStandardPaths::locate(QStandardPaths::DataLocation, name, QStandardPaths::LocateDirectory);
	if (m_rootPath.isEmpty()) {
		QStringList paths = QStandardPaths::standardLocations(QStandardPaths::DataLocation);
		m_rootPath = QDir::cleanPath(paths[0] + "/" + name);
		QDir::root().mkpath(m_rootPath);
	}

	createMenu();
}

QList<Disk*> Disk::listDisks(QWidget* parent)
{
	QList<Disk*> diskList;
	QStringList paths = QStandardPaths::standardLocations(QStandardPaths::DataLocation);
	QDir::root().mkpath(paths[0]);
	QDir dataPath(paths[0]);
	QStringList disks = dataPath.entryList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
	for (auto it = disks.cbegin(); it != disks.cend(); it++) {
		diskList.append(new Disk(*it, parent));
	}
	return diskList;
}

bool Disk::exists(const QString& name)
{
	return !QStandardPaths::locate(QStandardPaths::DataLocation, name, QStandardPaths::LocateDirectory).isEmpty();
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
		if (password != "foo") {
			QMessageBox::critical(m_parent, title, "Incorrect Password");
			return;
		}
		revealFolder();
	}

	m_isLocked = !m_isLocked;
	updateState();
}

void Disk::displaySettings()
{
	m_parent->raise();
	QMessageBox::information(m_parent, "SafeDisk", "Restore Disk");
}

void Disk::revealFolder()
{
#if defined(Q_OS_OSX)
	QStringList args;
	args << m_rootPath;
	QProcess::startDetached("open", args);
#elif defined(Q_OS_WIN)
	QStringList args;
	args << "/select," << m_rootPath;
	QProcess::startDetached("explorer", args);
#else
#endif
}
