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

#include "CreateDiskDialog.h"
#include "Disk.h"

#include <QMessageBox>
#include <QPushButton>
#include <QFileDialog>
#include <QStandardPaths>

CreateDiskDialog::CreateDiskDialog(QWidget* parent)
	: QDialog(parent)
{
	setupUi(this);

	buttonBox->button(QDialogButtonBox::Ok)->setEnabled(false);

	QStringList paths = QStandardPaths::standardLocations(QStandardPaths::DesktopLocation);
	storagePathText->setText(paths[0]);

	size_t i = 2;
	QString name = "SafeDisk";
	while (Disk::collision(storagePath(), name)) {
		name = QString("SafeDisk %1").arg(i++);
	}
	volumeNameLineEdit->setText(name);
}

QString CreateDiskDialog::storagePath() const
{
	return storagePathText->text().trimmed();
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
	return volumeSizeDoubleSpinBox->value();
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
	if (Disk::collision(storagePath(), name)) {
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

void CreateDiskDialog::on_choosePushButton_clicked()
{
	QString dirName = QFileDialog::getExistingDirectory(this, "Choose Storage Directory", storagePath());
	if (dirName.isEmpty()) {
		return;
	}

	storagePathText->setText(dirName);
	validate();
}
