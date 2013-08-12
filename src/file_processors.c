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

struct objective_score {
  int score;
  char* name;
  char* objective;
};

void print_objective_score(struct objective_score* score, char* format);

void process_scoreboard_data(struct config* config) {
  DEBUG(255, "process_scoreboard_data(%p);", config);
  char pathbuf[strlen(config->world_path) + 64];
  if (snprintf(pathbuf, sizeof(pathbuf), "%s/data/scoreboard.dat", config->world_path)) {
    nbt_node* node = nbt_parse_path(pathbuf);
    if (errno == NBT_OK) {
      static const char* PLAYERSCORES = "PlayerScores";
      nbt_node* scores = nbt_find_by_name(node, PLAYERSCORES);
      if (scores) {
        struct list_head* pos;
        list_for_each(pos, &scores->payload.tag_list->entry) {
          const struct nbt_list* entry = list_entry(pos, const struct nbt_list, entry);
          DEBUG(255, "entry = %p;", entry);
          static const char* NAME = "Name";
          static const char* SCORE = "Score";
          static const char* OBJECTIVE = "Objective";
          nbt_node* name = nbt_find_by_name(entry->data, NAME);
          nbt_node* score = nbt_find_by_name(entry->data, SCORE);
          nbt_node* objective = nbt_find_by_name(entry->data, OBJECTIVE);
          if (name && name->type == TAG_STRING
            && score && score->type == TAG_INT
            && objective && objective->type == TAG_STRING) {
            struct objective_score wrapped_score;
            wrapped_score.score = score->payload.tag_int;
            wrapped_score.name = name->payload.tag_string;
            wrapped_score.objective = objective->payload.tag_string;
            size_t i;
            for (i = 0; config->format[i]; i++)
              print_objective_score(&wrapped_score, config->format[0]);
          }
        }
      }
      nbt_free(node);
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
  while (*str && buf < end) \
    *buf++ = *str++;

void print_objective_score(struct objective_score* score, char* format) {
  char b[BUFSIZ];
  char* buf = b;
  char* s = buf;
  char* end = s + sizeof(b);
  char* f;
  for (f = format; *f != '\0'; f++) {
    if (*f == '%') {
      f++;
      static char* NAME = "name";
      static char* OBJECTIVE = "objective";
      static char* SCORE = "score";
      if (string_startsWith(f, NAME)) {
        APPEND(score->name);
        f += 3;
      } else if (string_startsWith(f, OBJECTIVE)) {
        APPEND(score->objective);
        f += 8;
      } else if (string_startsWith(f, SCORE)) {
        snprintf(buf, end - buf, "%d", score->score);
        while (*buf != '\0')
          buf++;
        f += 4;
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
};