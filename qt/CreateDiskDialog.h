#pragma once

#include "ui_CreateDisk.h"

class CreateDiskDialog : public QDialog, private Ui::CreateDisk
{
	Q_OBJECT
public:
	explicit CreateDiskDialog(QWidget* parent = nullptr);

	QString volumeName() const;
	QString password() const;
	uint64_t size() const;

private slots:
	void on_passwordLineEdit_textChanged(const QString &arg1);
	void on_repeatLineEdit_textChanged(const QString &arg1);

private:
	void verifyRepeat();
};
