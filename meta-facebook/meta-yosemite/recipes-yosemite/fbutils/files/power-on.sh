#!/bin/sh
#
# Copyright 2014-present Facebook. All Rights Reserved.
#
# This program file is free software; you can redistribute it and/or modify it
# under the terms of the GNU General Public License as published by the
# Free Software Foundation; version 2 of the License.
#
# This program is distributed in the hope that it will be useful, but WITHOUT
# ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
# FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
# for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program in a file named COPYING; if not, write to the
# Free Software Foundation, Inc.,
# 51 Franklin Street, Fifth Floor,
# Boston, MA 02110-1301 USA

### BEGIN INIT INFO
# Provides:          power-on
# Required-Start:
# Required-Stop:
# Default-Start:     S
# Default-Stop:
# Short-Description: Power on Server
### END INIT INFO
. /usr/local/fbpackages/utils/ast-functions

PATH=/sbin:/bin:/usr/sbin:/usr/bin:/usr/local/bin

# Disable Watch Dog Timer
/usr/local/bin/watchdog_ctrl.sh off


KEYDIR=/mnt/data/kv_store
DEF_PWR_ON=1
TO_PWR_ON=

check_por_config()
{

  TO_PWR_ON=-1

  # Check if the file/key doesn't exist
  if [ ! -f "${KEYDIR}/slot${1}_por_cfg" ]; then
    TO_PWR_ON=$DEF_PWR_ON
  else
    POR=`cat ${KEYDIR}/slot${1}_por_cfg`

    # Case ON
    if [ $POR == "on" ]; then
      TO_PWR_ON=1;

    # Case OFF
    elif [ $POR == "off" ]; then
      TO_PWR_ON=0;

    # Case LPS
    elif [ $POR == "lps" ]; then

      # Check if the file/key doesn't exist
      if [ ! -f "${KEYDIR}/pwr_server${1}_last_state" ]; then
        TO_PWR_ON=$DEF_PWR_ON
      else
        LS=`cat ${KEYDIR}/pwr_server${1}_last_state`
        if [ $LS == "on" ]; then
          TO_PWR_ON=1;
        elif [ $LS == "off" ]; then
          TO_PWR_ON=0;
        fi
      fi
    fi
  fi
}

# Check whether it is fresh power on reset
if [ $(is_bmc_por) -eq 1 ]; then

  check_por_config 1
  if [ $TO_PWR_ON -eq 1 ] && [ $(is_server_prsnt 1) == "1" ] ; then
    power-util slot1 on
  fi

  check_por_config 2
  if [ $TO_PWR_ON -eq 1 ] && [ $(is_server_prsnt 2) == "1" ] ; then
    power-util slot2 on
  fi

  check_por_config 3
  if [ $TO_PWR_ON -eq 1 ] && [ $(is_server_prsnt 3) == "1" ] ; then
    power-util slot3 on
  fi

  check_por_config 4
  if [ $TO_PWR_ON -eq 1 ] && [ $(is_server_prsnt 4) == "1" ] ; then
    power-util slot4 on
  fi
fi
