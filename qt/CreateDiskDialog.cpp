#include "CreateDiskDialog.h"

#include <QMessageBox>
#include <QPushButton>

CreateDiskDialog::CreateDiskDialog(QWidget* parent)
	: QDialog(parent)
{
	setupUi(this);
	buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);
}

QString CreateDiskDialog::volumeName() const
{
	return volumeNameLineEdit->text();
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
	verifyRepeat();
}

void CreateDiskDialog::on_repeatLineEdit_textChanged(const QString&)
{
	verifyRepeat();
}

void CreateDiskDialog::verifyRepeat()
{
	auto password = passwordLineEdit->text();
	auto repeat = repeatLineEdit->text();
	auto button = buttonBox->button(QDialogButtonBox::Ok);

	button->setEnabled(false);

	if (password.startsWith(repeat)) {
		if (password == repeat) {
			repeatLineEdit->setStyleSheet("");
			if (!password.isEmpty()) {
				button->setEnabled(true);
			}
		}
		else {
			repeatLineEdit->setStyleSheet("background-color: rgb(255, 205, 15)");
		}
	}
	else if (password != repeat) {
		repeatLineEdit->setStyleSheet("background-color: rgb(255, 125, 125)");
	}
}
