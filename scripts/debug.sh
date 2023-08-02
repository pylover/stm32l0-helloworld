#! /usr/bin/env bash
# Copyright 2023 Vahid Mardani
# 
# This file is part of stm32l0-helloworld.
#  stm32l0-helloworld is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by the
#  Free Software Foundation, either version 3 of the License, or (at your
#  option) any later version.
# 
#  stm32l0-helloworld is distributed in the hope that it will be useful, but
#  WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
#  or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
#  for more details.
# 
#  You should have received a copy of the GNU General Public License along
#  with stm32l0-helloworld. If not, see <https://www.gnu.org/licenses/>.
# 
#  Author: Vahid Mardani <vahid.mardani@gmail.com>
#
# A simple script which automates the make openocd & make gdb


list_descendants() {
  local children=$(ps -o pid= --ppid "$1")

  for pid in $children
  do
    list_descendants "$pid"
  done

  echo "$children"
}

TMPFILE=/tmp/stm32-ocd.out
if [ -f ${TMPFILE} ]; then
  truncate -s0 ${TMPFILE}
else
  touch ${TMPFILE}
fi
make openocd OPENOCD_ARGS="--log_output ${TMPFILE}" &
ocdpid=$!

tail -f ${TMPFILE} | grep -q '\*\* Resetting Target \*\*'

make gdb
exitstatus=$?
echo "Killing pid $ocdpid and all children"
kill $(list_descendants $ocdpid)
exit $exitstatus
