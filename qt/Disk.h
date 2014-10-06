#pragma once

#include <QObject>
#include <QMenu>

class Disk : public QObject
{
	Q_OBJECT
public:
	explicit Disk(const QString& name, QWidget* parent);

	QMenu* menu() const;

	static
	QList<Disk*> listDisks(QWidget* parent);

	static
	bool exists(const QString& name);

signals:

private slots:
	void toggleMount();
	void revealFolder();
	void displaySettings();

private:
	void createMenu();
	void updateState();

private:
	QString m_name;
	QWidget* m_parent;
	QMenu* m_menu = nullptr;
	QAction* m_toggleAction = nullptr;
	QAction* m_revealAction = nullptr;
	bool m_isLocked = true;
	QString m_rootPath;
};
