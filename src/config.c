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

#include "config.h"

#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>

static struct config {
  char* world_path;
  char** format;
} global_config;

int parse_config(char* filename) {
  FILE* f = fopen(filename, "r");
  if (!f) {
    fprintf(stderr, "Error '%s' while opening '%s'.\n", strerror(errno), filename);
    return 0;
  }
  bzero(&global_config, sizeof(struct config));
  char linebuffer[BUFSIZ];
  unsigned int line_count = 0;
  while (fgets(linebuffer, sizeof(linebuffer), f)) {
    line_count++;
    if (linebuffer[0] == '#' || linebuffer[1] == 1)
      continue;
    char key[BUFSIZ];
    char value[BUFSIZ];
    if (sscanf(linebuffer, "%[a-z_] = %[^\t\n]", key, value) == 2) {
      if (strcmp(key, "worldpath") == 0) {
        free(global_config.world_path);
        global_config.world_path = strdup(value);
      } else if (strcmp(key, "format") == 0) {
        if (global_config.format == NULL) {
          global_config.format = malloc(sizeof(char*) * 2);
          bzero(global_config.format, sizeof(char*) * 2);
          global_config.format[0] = strdup(value);
        } else {
          size_t i = 0;
          while (global_config.format[++i]);
          global_config.format = realloc(global_config.format, sizeof(char*) * (i + 2));
          global_config.format[i] = strdup(value);
          global_config.format[++i] = NULL;
        }
      } else if (strcmp(key, "unbuffered") == 0)
        setvbuf(stdout, NULL, _IONBF, 0);
    }
  }
  return line_count;
};