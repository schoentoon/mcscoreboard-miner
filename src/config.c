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
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <limits.h>
#include <string.h>
#include <signal.h>
#include <strings.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/inotify.h>

#include <event2/bufferevent.h>

#define ADD_TO_ARRAY(array, value) \
  if (array == NULL) { \
    array = malloc(sizeof(char*) * 2); \
    bzero(array, sizeof(char*) * 2); \
    array[0] = strdup(value); \
  } else { \
    size_t i = 0; \
    while (array[++i]); \
    array = realloc(array, sizeof(char*) * (i + 2)); \
    array[i] = strdup(value); \
    array[++i] = NULL; \
  }

static struct config global_config;

unsigned char inited = 0;

int config_is_empty() {
  return !inited;
};

int should_daemonize() {
  return global_config.daemon;
};

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
        ADD_TO_ARRAY(global_config.scoreboard_format, value);
      } else if (strcmp(key, "players_format") == 0) {
        ADD_TO_ARRAY(global_config.players_format, value);
      } else if (strcmp(key, "level_format") == 0) {
        ADD_TO_ARRAY(global_config.level_format, value);
      } else if (strcmp(key, "stats_useitem_format") == 0) {
        ADD_TO_ARRAY(global_config.stats_useItem_format, value);
      } else if (strcmp(key, "stats_mineblock_format") == 0) {
        ADD_TO_ARRAY(global_config.stats_mineBlock_format, value);
      } else if (strcmp(key, "stats_killentity_format") == 0) {
        ADD_TO_ARRAY(global_config.stats_killEntity_format, value);
      } else if (strcmp(key, "stats_killedbyentity_format") == 0) {
        ADD_TO_ARRAY(global_config.stats_killedByEntity_format, value);
      } else if (strcmp(key, "stats_craftitem_format") == 0) {
        ADD_TO_ARRAY(global_config.stats_craftItem_format, value);
      } else if (strcmp(key, "stats_breakitem_format") == 0) {
        ADD_TO_ARRAY(global_config.stats_breakItem_format, value);
      } else if (strcmp(key, "stats_format") == 0) {
        ADD_TO_ARRAY(global_config.stats_format, value);
      } else if (strcmp(key, "exec") == 0) {
        free(global_config.pipe_to);
        global_config.pipe_to = strdup(value);
      } else if (strcmp(key, "pidfile") == 0) {
        free(global_config.pidfile);
        global_config.pidfile = strdup(value);
      }
    } else if (sscanf(linebuffer, "%[a-z_]", key) == 1) {
      if (strcmp(key, "daemon") == 0)
        global_config.daemon = 1;
      else if (strcmp(key, "unbuffered") == 0)
        setvbuf(stdout, NULL, _IONBF, 0);
    }
  }
  inited = 1;
  return line_count;
};

void signal_handler(int signal) {
  kill(global_config.pid_child, signal);
  if (signal == SIGTERM)
    exit(0);
};

int dispatch_config(struct event_base* base) {
  int inotifyfd = inotify_init();
  if (inotifyfd == -1)
    return 1;
  struct bufferevent *bufferevent = bufferevent_socket_new(base, inotifyfd, 0);
  bufferevent_setcb(bufferevent, file_changed_cb, NULL, NULL, &global_config);
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
  if ((global_config.stats_useItem_format || global_config.stats_mineBlock_format || global_config.stats_killEntity_format
    || global_config.stats_killedByEntity_format || global_config.stats_craftItem_format || global_config.stats_format)
    && snprintf(pathbuf, sizeof(pathbuf), "%s/stats", global_config.world_path)) {
    struct stat sb;
    if (stat(pathbuf, &sb) == 0 && S_ISDIR(sb.st_mode)) {
      int wd = inotify_add_watch(inotifyfd, pathbuf, IN_CLOSE_WRITE);
      if (wd == -1) {
        fprintf(stderr, "There was an error adding '%s' to the file observer, error code %d.\n", pathbuf, wd);
        return 1;
      }
      global_config.stats_wd = wd;
    } else {
      fprintf(stderr, "There was an error monitoring '%s', error: '%s'\n", pathbuf, strerror(errno));
      return 1;
    }
  }
  if (global_config.pipe_to) {
    int pipes[2];
    if (pipe(pipes) == -1) {
      fprintf(stderr, "Could not create pipe :( '%s'\n", strerror(errno));
      return 1;
    }
    pid_t pid = fork();
    switch (pid) {
    case -1:
      fprintf(stderr, "Couldn't fork :( '%s'\n", strerror(errno));
      return 1;
    case 0:
      close(fileno(stdin));
      dup(pipes[0]);
      close(pipes[1]);
      close(pipes[0]);
      const char *argv[] = { "sh", "-c", global_config.pipe_to, NULL };
      execvp("/bin/sh", (char * const *) argv);
      fprintf(stderr, "execvp failed\n");
      exit(EXIT_FAILURE);
      break;
    default:
      global_config.pid_child = pid;
      struct sigaction sa;
      sa.sa_flags = 0;
      sigemptyset(&sa.sa_mask);
      sa.sa_handler = signal_handler;
      sigaction(SIGTERM,  &sa, (struct sigaction*) NULL);
      sigaction(SIGHUP,  &sa, (struct sigaction*) NULL);
      sigaction(SIGUSR1, &sa, (struct sigaction*) NULL);
      sigaction(SIGUSR2, &sa, (struct sigaction*) NULL);
      close(fileno(stdout));
      dup(pipes[1]);
      close(pipes[0]);
      close(pipes[1]);
      break;
    }
  }
  if (global_config.pidfile) {
    FILE *f;
    int fd;
    if (((fd = open(global_config.pidfile, O_RDWR|O_CREAT|O_TRUNC, 0644)) == -1) || ((f = fdopen(fd, "r+")) == NULL) ) {
      fprintf(stderr, "Can't open or create %s.\n", global_config.pidfile);
      return 1;
    }

    int pid = getpid();
    if (!fprintf(f,"%d", pid)) {
      fprintf(stderr, "Can't write pid , '%s'.\n", strerror(errno));
      close(fd);
      return 1;
    }
    fclose(f);
    close(fd);
  }
  return 0;
};