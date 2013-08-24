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

#include "filereader.h"

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <strings.h>
#include <sys/stat.h>
#include <sys/inotify.h>

#include <event2/bufferevent.h>

static struct config global_config;

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
        size_t slen = strlen(value);
        if (value[slen] == '/')
          value[slen] = '\0';
        global_config.world_path = strdup(value);
      } else if (strcmp(key, "scoreboard_format") == 0) {
        if (global_config.scoreboard_format == NULL) {
          global_config.scoreboard_format = malloc(sizeof(char*) * 2);
          bzero(global_config.scoreboard_format, sizeof(char*) * 2);
          global_config.scoreboard_format[0] = strdup(value);
        } else {
          size_t i = 0;
          while (global_config.scoreboard_format[++i]);
          global_config.scoreboard_format = realloc(global_config.scoreboard_format, sizeof(char*) * (i + 2));
          global_config.scoreboard_format[i] = strdup(value);
          global_config.scoreboard_format[++i] = NULL;
        }
      } else if (strcmp(key, "players_format") == 0) {
        if (global_config.players_format == NULL) {
          global_config.players_format = malloc(sizeof(char*) * 2);
          bzero(global_config.players_format, sizeof(char*) * 2);
          global_config.players_format[0] = strdup(value);
        } else {
          size_t i = 0;
          while (global_config.players_format[++i]);
          global_config.players_format = realloc(global_config.players_format, sizeof(char*) * (i + 2));
          global_config.players_format[i] = strdup(value);
          global_config.players_format[++i] = NULL;
        }
      } else if (strcmp(key, "level_format") == 0) {
        if (global_config.level_format == NULL) {
          global_config.level_format = malloc(sizeof(char*) * 2);
          bzero(global_config.level_format, sizeof(char*) * 2);
          global_config.level_format[0] = strdup(value);
        } else {
          size_t i = 0;
          while (global_config.level_format[++i]);
          global_config.level_format = realloc(global_config.level_format, sizeof(char*) * (i + 2));
          global_config.level_format[i] = strdup(value);
          global_config.level_format[++i] = NULL;
        }
      } else if (strcmp(key, "unbuffered") == 0)
        setvbuf(stdout, NULL, _IONBF, 0);
    }
  }
  return line_count;
};

int dispatch_config(struct event_base* base) {
  int inotifyfd = inotify_init();
  if (inotifyfd == -1)
    return 1;
  struct bufferevent *bufferevent = bufferevent_socket_new(base, inotifyfd, 0);
  bufferevent_setcb(bufferevent, nbt_file_changed_cb, NULL, NULL, &global_config);
  bufferevent_enable(bufferevent, EV_READ);
  char pathbuf[strlen(global_config.world_path) + 32];
  if (global_config.scoreboard_format && snprintf(pathbuf, sizeof(pathbuf), "%s/data", global_config.world_path)) {
    struct stat sb;
    if (stat(pathbuf, &sb) == 0 && S_ISDIR(sb.st_mode)) {
      int wd = inotify_add_watch(inotifyfd, pathbuf, IN_CLOSE_WRITE);
      if (wd == -1) {
        fprintf(stderr, "There was an error adding '%s' to the file observer, error code %d.\n", pathbuf, wd);
        return 1;
      }
      global_config.data_wd = wd;
    } else {
      fprintf(stderr, "There was an error monitoring '%s', error: '%s'\n", pathbuf, strerror(errno));
      return 1;
    }
  }
  if (global_config.players_format && snprintf(pathbuf, sizeof(pathbuf), "%s/players", global_config.world_path)) {
    struct stat sb;
    if (stat(pathbuf, &sb) == 0 && S_ISDIR(sb.st_mode)) { /* They work with temporary files here so IN_MOVED_TO instead */
      int wd = inotify_add_watch(inotifyfd, pathbuf, IN_MOVED_TO); /* of IN_CLOSE_WRITE */
      if (wd == -1) {
        fprintf(stderr, "There was an error adding '%s' to the file observer, error code %d.\n", pathbuf, wd);
        return 1;
      }
      global_config.players_wd = wd;
    } else {
      fprintf(stderr, "There was an error monitoring '%s', error: '%s'\n", pathbuf, strerror(errno));
      return 1;
    }
  }
  if (global_config.level_format && snprintf(pathbuf, sizeof(pathbuf), "%s/level.dat", global_config.world_path)) {
    if (access(pathbuf, F_OK) != -1) { /* They work with temporary files here so IN_MOVED_TO instead */
      int wd = inotify_add_watch(inotifyfd, global_config.world_path, IN_MOVED_TO); /* of IN_CLOSE_WRITE */
      if (wd == -1) {
        fprintf(stderr, "There was an error adding '%s' to the file observer, error code %d.\n", global_config.world_path, wd);
        return 1;
      }
      global_config.level_wd = wd;
    } else {
      fprintf(stderr, "There was an error monitoring '%s', error: '%s'\n", global_config.world_path, strerror(errno));
      return 1;
    }
  }
  return 0;
};