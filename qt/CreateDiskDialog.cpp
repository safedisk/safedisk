#include "CreateDiskDialog.h"
#include "Disk.h"

#include <QMessageBox>
#include <QPushButton>

CreateDiskDialog::CreateDiskDialog(QWidget* parent)
	: QDialog(parent)
{
	setupUi(this);
	buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);

	size_t i = 2;
	QString name = "SafeDisk";
	while (Disk::exists(name)) {
		name = QString("SafeDisk %1").arg(i++);
	}
	volumeNameLineEdit->setText(name);
}

QString CreateDiskDialog::volumeName() const
{
	return volumeNameLineEdit->text().trimmed();
}

QString CreateDiskDialog::password() const
{
	return passwordLineEdit->text();
}

uint64_t CreateDiskDialog::size() const
{
	const uint64_t MB = 1000 * 1000;
	return volumeSizeDoubleSpinBox->value() * MB;
}

void CreateDiskDialog::on_passwordLineEdit_textChanged(const QString&)
{
	validate();
}

void CreateDiskDialog::on_repeatLineEdit_textChanged(const QString&)
{
	validate();
}

void CreateDiskDialog::on_volumeNameLineEdit_textChanged(const QString&)
{
	validate();
}

void CreateDiskDialog::validate()
{
	auto button = buttonBox->button(QDialogButtonBox::Ok);
	button->setEnabled(false);
	if (validateName() && validatePassword()) {
		button->setEnabled(true);
	}
}

bool CreateDiskDialog::validateName()
{
	auto name = volumeNameLineEdit->text().trimmed();
	if (name.isEmpty()) {
		volumeNameLineEdit->setStyleSheet("background-color: rgb(255, 125, 125)");
		return false;
	}
	if (Disk::exists(name)) {
		volumeNameLineEdit->setStyleSheet("background-color: rgb(255, 205, 15)");
		return false;
	}
	volumeNameLineEdit->setStyleSheet("");
	return true;
}

bool CreateDiskDialog::validatePassword()
{
	auto password = passwordLineEdit->text();
	auto repeat = repeatLineEdit->text();

	if (password.startsWith(repeat)) {
		if (password == repeat) {
			repeatLineEdit->setStyleSheet("");
			if (!password.isEmpty()) {
				return true;
			}
		}
		else {
			repeatLineEdit->setStyleSheet("background-color: rgb(255, 205, 15)");
		}
	}
	else if (password != repeat) {
		repeatLineEdit->setStyleSheet("background-color: rgb(255, 125, 125)");
	}
	return false;
}
