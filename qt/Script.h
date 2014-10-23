#pragma once

#include <QObject>
#include <QProcess>

class Script : public QObject
{
	Q_OBJECT
public:
	explicit Script(const QString& program, const QStringList& args, const QString& input);

	void start();
	void terminate();

signals:
	void finished(int exitCode, QProcess::ExitStatus exitStatus);

private slots:
	void onStarted();
	void onFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
	QString m_program;
	QStringList m_args;
	QString m_input;
	QProcess m_process;
};
