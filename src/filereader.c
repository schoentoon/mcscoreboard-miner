/*  mcscoreboard-miner
 *  Copyright (C) 2013  Toon Schoenmakers
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "filereader.h"

#include "debug.h"

#include <limits.h>
#include <sys/inotify.h>

#define BUF_LEN (sizeof(struct inotify_event) + NAME_MAX + 1)

void nbt_file_changed_cb(struct bufferevent* bev, void* args) {
  char buf[BUF_LEN];
  size_t numRead;
  while ((numRead = bufferevent_read(bev, buf, BUF_LEN))) {
    struct inotify_event *event = (struct inotify_event*) buf;
    if (event->len > 0) {
      DEBUG(255, "%s changed!", event->name);
    }
  };
};