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

#include "file_processors.h"

#include "debug.h"

#include "nbt.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void process_scoreboard_data(struct config* config) {
  DEBUG(255, "process_scoreboard_data(%p);", config);
  char pathbuf[strlen(config->world_path) + 64];
  if (snprintf(pathbuf, sizeof(pathbuf), "%s/data/scoreboard.dat", config->world_path)) {
    nbt_node* node = nbt_parse_path(pathbuf);
    if (errno == NBT_OK) {
      char* dump = nbt_dump_ascii(node);
      fprintf(stderr, "%s\n", dump);
      free(dump);
      nbt_free(node);
    }
  }
};