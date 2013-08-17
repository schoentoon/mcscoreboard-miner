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

void print_objective_score(nbt_node* nbt, char** format);

void print_player(nbt_node* nbt, char* username, char** format);

void process_scoreboard_data(struct config* config) {
  DEBUG(255, "process_scoreboard_data(%p);", config);
  char pathbuf[strlen(config->world_path) + 64];
  if (snprintf(pathbuf, sizeof(pathbuf), "%s/data/scoreboard.dat", config->world_path)) {
    nbt_node* nbt = nbt_parse_path(pathbuf);
    if (errno == NBT_OK) {
      static const char* PLAYERSCORES = "PlayerScores";
      nbt_node* scores = nbt_find_by_name(nbt, PLAYERSCORES);
      if (scores && scores->type == TAG_LIST) {
        struct list_head* pos;
        list_for_each(pos, &scores->payload.tag_list->entry) {
          const struct nbt_list* entry = list_entry(pos, const struct nbt_list, entry);
          print_objective_score(entry->data, config->scoreboard_format);
        }
      }
      nbt_free(nbt);
    }
  }
};

void process_player_data(struct config* config, char* player_file) {
  DEBUG(255, "process_player_data(%p, %s);", config, player_file);
  char pathbuf[strlen(config->world_path) + 64];
  if (snprintf(pathbuf, sizeof(pathbuf), "%s/players/%s", config->world_path, player_file)) {
    nbt_node* nbt = nbt_parse_path(pathbuf);
    if (errno == NBT_OK) {
      char* dump = nbt_dump_ascii(nbt);
      DEBUG(255, "%s", dump);
      free(dump);
      const size_t file_len = strlen(player_file);
      size_t dot;
      for (dot = 0; dot < file_len; dot++) {
        if (player_file[dot] == '.')
          break;
      }
      if (player_file[dot] == '.')
        player_file[dot] = '\0';
      print_player(nbt, player_file, config->players_format);
      if (player_file[dot] == '\0')
        player_file[dot] = '.';
      nbt_free(nbt);
    }
  }
};

int string_startsWith(char* line, char* start) {
  int line_len = strlen(line);
  int start_len = strlen(start);
  int lowest = (line_len < start_len) ? line_len : start_len;
  int i;
  for (i = 0; i < lowest; i++) {
    if (line[i] != start[i])
      return 0;
  }
  return 1;
};

#define APPEND(/*char* */ str) \
  { \
    char* s = str; \
    while (*s && buf < end) \
      *buf++ = *s++; \
  }

#define DEFINE_TAG(str) \
  static char* str = #str; \
  nbt_node* nbt_##str = NULL;

inline nbt_node* find_nbt_node(nbt_node** cached, nbt_node* nbt, char* string) {
  if (*cached)
    return *cached;
  *cached = nbt_find_by_name(nbt, string);
  return *cached;
}

#define FIND_NBT_NODE(str, nbt_name) \
  find_nbt_node(&nbt_##str, nbt, #nbt_name);

void print_objective_score(nbt_node* nbt, char** formats) {
  DEFINE_TAG(name);
  DEFINE_TAG(objective);
  DEFINE_TAG(score);
  size_t i;
  for (i = 0; formats[i]; i++) {
    char b[BUFSIZ];
    char* buf = b;
    char* end = buf + sizeof(b);
    char* f;
    for (f = formats[i]; *f != '\0'; f++) {
      if (*f == '%') {
        f++;
        if (string_startsWith(f, name)) {
          nbt_node* tmp = FIND_NBT_NODE(name, Name);
          if (tmp && tmp->type == TAG_STRING) {
            APPEND(tmp->payload.tag_string);
            f += 3;
          }
        } else if (string_startsWith(f, objective)) {
          nbt_node* tmp = FIND_NBT_NODE(objective, Objective);
          if (tmp && tmp->type == TAG_STRING) {
            APPEND(tmp->payload.tag_string);
            f += 8;
          }
        } else if (string_startsWith(f, score)) {
          nbt_node* tmp = FIND_NBT_NODE(score, Score);
          if (tmp && tmp->type == TAG_INT) {
            snprintf(buf, end - buf, "%d", tmp->payload.tag_int);
            while (*buf != '\0')
              buf++;
            f += 4;
          }
        }
      } else if (buf < end) {
        if (*f == '\\') {
          switch (*(++f)) {
          case 'n':
            *buf++ = '\n';
            break;
          case 'r':
            *buf++ = '\r';
            break;
          case 't':
            *buf++ = '\t';
            break;
          default:
            *buf++ = '\\';
            *buf++ = *f;
            break;
          }
        } else
          *buf++ = *f;
      }
    }
    *buf = '\0';
    printf("%s", b);
  }
};

void print_player(nbt_node* nbt, char* username, char** format) {
  DEBUG(255, "print_player(%p, %s, %p);", nbt, username, format);
};