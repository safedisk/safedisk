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
	void on_volumeNameLineEdit_textChanged(const QString &arg1);

private:
	void validate();

	bool validateName();
	bool validatePassword();
};
