#!/bin/bash -x
#
#  SafeDisk
#  Copyright (C) 2014 Frank Laub
#
#  This program is free software: you can redistribute it and/or modify
#  it under the terms of the GNU Affero General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  This program is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU Affero General Public License for more details.
#
#  You should have received a copy of the GNU Affero General Public License
#  along with this program.  If not, see <http://www.gnu.org/licenses/>.

set -e

BREWS="boost osxfuse qt5 openssl"

for i in $BREWS; do
	brew outdated | grep -q $i && brew upgrade $i
done

for i in $BREWS; do
	brew list | grep -q $i || brew install $i
done
