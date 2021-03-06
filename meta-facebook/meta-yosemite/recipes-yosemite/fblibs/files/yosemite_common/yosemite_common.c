/*
 *
 * Copyright 2015-present Facebook. All Rights Reserved.
 *
 * This file contains code to support IPMI2.0 Specificaton available @
 * http://www.intel.com/content/www/us/en/servers/ipmi/ipmi-specifications.html
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <math.h>
#include <fcntl.h>
#include <errno.h>
#include <syslog.h>
#include <pthread.h>
#include <string.h>
#include "yosemite_common.h"

#define CRASHDUMP_BIN       "/usr/local/bin/dump.sh"
#define CRASHDUMP_FILE      "/mnt/data/crashdump_"

int
yosemite_common_fru_name(uint8_t fru, char *str) {

  switch(fru) {
    case FRU_SLOT1:
      sprintf(str, "slot1");
      break;

    case FRU_SLOT2:
      sprintf(str, "slot2");
      break;

    case FRU_SLOT3:
      sprintf(str, "slot3");
      break;

    case FRU_SLOT4:
      sprintf(str, "slot4");
      break;

    case FRU_SPB:
      sprintf(str, "spb");
      break;

    case FRU_NIC:
      sprintf(str, "nic");
      break;

    default:
#ifdef DEBUG
      syslog(LOG_WARNING, "yosemite_common_fru_id: Wrong fru id");
#endif
    return -1;
  }

  return 0;
}

int
yosemite_common_fru_id(char *str, uint8_t *fru) {

  if (!strcmp(str, "all")) {
    *fru = FRU_ALL;
  } else if (!strcmp(str, "slot1")) {
    *fru = FRU_SLOT1;
  } else if (!strcmp(str, "slot2")) {
    *fru = FRU_SLOT2;
  } else if (!strcmp(str, "slot3")) {
    *fru = FRU_SLOT3;
  } else if (!strcmp(str, "slot4")) {
    *fru = FRU_SLOT4;
  } else if (!strcmp(str, "spb")) {
    *fru = FRU_SPB;
  } else if (!strcmp(str, "nic")) {
    *fru = FRU_NIC;
  } else {
#ifdef DEBUG
    syslog(LOG_WARNING, "yosemite_common_fru_id: Wrong fru id");
#endif
    return -1;
  }

  return 0;
}

void *
generate_dump(void *arg) {

  uint8_t fru = *(uint8_t *) arg;
  char cmd[128];
  char fruname[16];

  yosemite_common_fru_name(fru, fruname);

  // HEADER LINE for the dump
  memset(cmd, 0, 128);
  sprintf(cmd, "%s time > %s%s", CRASHDUMP_BIN, CRASHDUMP_FILE, fruname);
  system(cmd);

  // COREID dump
  memset(cmd, 0, 128);
  sprintf(cmd, "%s %s 48 coreid >> %s%s", CRASHDUMP_BIN, fruname,
      CRASHDUMP_FILE, fruname);
  system(cmd);

  // MSR dump
  memset(cmd, 0, 128);
  sprintf(cmd, "%s %s 48 msr >> %s%s", CRASHDUMP_BIN, fruname,
      CRASHDUMP_FILE, fruname);
  system(cmd);

  syslog(LOG_CRIT, "Crashdump for FRU: %d is generated.", fru);
}

int
yosemite_common_crashdump(uint8_t fru) {

  if (access(CRASHDUMP_BIN, F_OK) == -1) {
    syslog(LOG_CRIT, "Crashdump for FRU: %d failed : "
        "crashdump binary is not preset", fru);
    return 0;
  }

  pthread_t t_dump;

  if (pthread_create(&t_dump, NULL, generate_dump, (void*) &fru) < 0) {
    syslog(LOG_WARNING, "pal_store_crashdump: pthread_create for"
        " FRU %d failed\n", fru);
  }

  syslog(LOG_INFO, "Crashdump for FRU: %d is being generated.", fru);

  return 0;
}
