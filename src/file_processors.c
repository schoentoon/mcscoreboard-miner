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

#include <jansson.h>

void print_objective_score(nbt_node* nbt, char** format);

void print_player(nbt_node* nbt, char* username, char** formats);

void print_level(nbt_node* nbt, char** formats);

void print_stat(json_t* json, char* username, const char* key, char** formats);

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
#if 0
      char* dump = nbt_dump_ascii(nbt);
      DEBUG(255, "%s", dump);
      free(dump);
#endif
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

void process_level_data(struct config* config) {
  DEBUG(255, "process_level_data(%p);", config);
  char pathbuf[strlen(config->world_path) + 64];
  if (snprintf(pathbuf, sizeof(pathbuf), "%s/level.dat", config->world_path)) {
    nbt_node* nbt = nbt_parse_path(pathbuf);
    if (errno == NBT_OK) {
#if 0
      char* dump = nbt_dump_ascii(nbt);
      DEBUG(255, "%s", dump);
      free(dump);
#endif
      print_level(nbt, config->level_format);
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
}

void process_stats(struct config* config, char* player_file) {
  DEBUG(255, "process_stats(%p, %s);", config, player_file);
  json_t *json;
  json_error_t error;
  char pathbuf[strlen(config->world_path) + strlen(player_file) + 64];
  if (snprintf(pathbuf, sizeof(pathbuf), "%s/stats/%s", config->world_path, player_file)) {
    DEBUG(255, "pathbuf: %s", pathbuf);
    json = json_load_file(pathbuf, 0, &error);
    if (json) {
      const char *key;
      json_t *value;
      size_t i;
      for (i = 0; i < strlen(player_file); i++) {
        if (player_file[i] == '.') {
          player_file[i] = '\0';
          break;
        }
      }
      json_object_foreach(json, key, value) {
        DEBUG(255, "Key: %s", key);
        static const char* USEITEM = "stat.useItem.";
        static const char* MINEBLOCK = "stat.mineBlock.";
        static const char* KILLENTITY = "stat.killEntity.";
        static const char* KILLEDBYENTITY = "stat.entityKilledBy.";
        static const char* CRAFTEDITEM = "stat.craftItem.";
        static const char* BREAKITEM = "stat.breakItem.";
        static const char* STAT = "stat.";
        if (config->stats_useItem_format && string_startsWith((char*) key, (char*) USEITEM))
          print_stat(value, player_file, key, config->stats_useItem_format);
        else if (config->stats_mineBlock_format && string_startsWith((char*) key, (char*) MINEBLOCK))
          print_stat(value, player_file, key, config->stats_mineBlock_format);
        else if (config->stats_killEntity_format && string_startsWith((char*) key, (char*) KILLENTITY))
          print_stat(value, player_file, key, config->stats_killEntity_format);
        else if (config->stats_killedByEntity_format && string_startsWith((char*) key, (char*) KILLEDBYENTITY))
          print_stat(value, player_file, key, config->stats_killedByEntity_format);
        else if (config->stats_craftItem_format && string_startsWith((char*) key, (char*) CRAFTEDITEM))
          print_stat(value, player_file, key, config->stats_craftItem_format);
        else if (config->stats_breakItem_format && string_startsWith((char*) key, (char*) BREAKITEM))
          print_stat(value, player_file, key, config->stats_breakItem_format);
        else if (config->stats_format && string_startsWith((char*) key, (char*) STAT))
          print_stat(value, player_file, key, config->stats_format);
      }
      json_decref(json);
    } else {
      DEBUG(255, "json == NULL :(");
    }
  }
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
            f += 3; // length of name -1 as we already were at the first character
          }
        } else if (string_startsWith(f, objective)) {
          nbt_node* tmp = FIND_NBT_NODE(objective, Objective);
          if (tmp && tmp->type == TAG_STRING) {
            APPEND(tmp->payload.tag_string);
            f += 8; // length of objective -1 as we already were at the first character
          }
        } else if (string_startsWith(f, score)) {
          nbt_node* tmp = FIND_NBT_NODE(score, Score);
          if (tmp && tmp->type == TAG_INT) {
            snprintf(buf, end - buf, "%d", tmp->payload.tag_int);
            while (*buf != '\0')
              buf++;
            f += 4; // length of score -1 as we already were at the first character
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

void print_player(nbt_node* nbt, char* username, char** formats) {
  DEBUG(255, "print_player(%p, %s, %p);", nbt, username, formats);
  static char* USERNAME = "username";
  DEFINE_TAG(level);
  DEFINE_TAG(xp);
  DEFINE_TAG(health);
  DEFINE_TAG(food);
  DEFINE_TAG(score);
  DEFINE_TAG(absorption);
  nbt_node* nbt_pos = NULL;
  static char* POSX = "PosX";
  static char* POSY = "PosY";
  static char* POSZ = "PosZ";
  static char* xplevel = "xplevel";
  size_t i;
  for (i = 0; formats[i]; i++) {
    char b[BUFSIZ];
    char* buf = b;
    char* end = buf + sizeof(b);
    char* f;
    for (f = formats[i]; *f != '\0'; f++) {
      if (*f == '%') {
        f++;
        if (string_startsWith(f, USERNAME)) {
          APPEND(username);
          f += 7;
        } if (string_startsWith(f, xplevel)) {
          nbt_node* level = FIND_NBT_NODE(level, XpLevel);
          nbt_node* xp = FIND_NBT_NODE(xp, XpP);
          if (level && level->type == TAG_INT && xp && xp->type == TAG_FLOAT) {
            float total = level->payload.tag_int + xp->payload.tag_float;
            snprintf(buf, end - buf, "%f", total);
            while (*buf != '\0')
              buf++;
            f += 6; // length of xplevel -1 as we already were at the first character
          }
        } else if (string_startsWith(f, level)) {
          nbt_node* tmp = FIND_NBT_NODE(level, XpLevel);
          if (tmp && tmp->type == TAG_INT) {
            snprintf(buf, end - buf, "%d", tmp->payload.tag_int);
            while (*buf != '\0')
              buf++;
            f += 4; // length of level -1 as we already were at the first character
          }
        } else if (string_startsWith(f, xp)) {
          nbt_node* tmp = FIND_NBT_NODE(xp, XpP);
          if (tmp && tmp->type == TAG_FLOAT) {
            snprintf(buf, end - buf, "%f", tmp->payload.tag_float);
            while (*buf != '\0')
              buf++;
            f += 1; // length of xp -1 as we already were at the first character
          }
        } else if (string_startsWith(f, health)) {
          nbt_node* tmp = FIND_NBT_NODE(health, Health);
          if (tmp && tmp->type == TAG_SHORT) {
            snprintf(buf, end - buf, "%d", tmp->payload.tag_short);
            while (*buf != '\0')
              buf++;
            f += 5; // length of health -1 as we already were at the first character
          }
        } else if (string_startsWith(f, food)) {
          nbt_node* tmp = FIND_NBT_NODE(food, foodLevel);
          if (tmp && tmp->type == TAG_INT) {
            snprintf(buf, end - buf, "%d", tmp->payload.tag_int);
            while (*buf != '\0')
              buf++;
            f += 3; // length of food -1 as we already were at the first character
          }
        } else if (string_startsWith(f, score)) {
          nbt_node* tmp = FIND_NBT_NODE(score, Score);
          if (tmp && tmp->type == TAG_INT) {
            snprintf(buf, end - buf, "%d", tmp->payload.tag_int);
            while (*buf != '\0')
              buf++;
            f += 4; // length of score -1 as we already were at the first character
          }
        } else if (string_startsWith(f, POSX)) {
          nbt_node* tmp = FIND_NBT_NODE(pos, Pos);
          if (tmp && tmp->type == TAG_LIST) {
            nbt_node* pos = nbt_list_item(tmp, 0);
            if (pos && pos->type == TAG_DOUBLE) {
              snprintf(buf, end - buf, "%f", pos->payload.tag_double);
              while (*buf != '\0')
                buf++;
              f+= 3; // length of posx -1 as we already were at the first character
            }
          }
        } else if (string_startsWith(f, POSY)) {
          nbt_node* tmp = FIND_NBT_NODE(pos, Pos);
          if (tmp && tmp->type == TAG_LIST) {
            nbt_node* pos = nbt_list_item(tmp, 1);
            if (pos && pos->type == TAG_DOUBLE) {
              snprintf(buf, end - buf, "%f", pos->payload.tag_double);
              while (*buf != '\0')
                buf++;
              f+= 3; // length of posy -1 as we already were at the first character
            }
          }
        } else if (string_startsWith(f, POSZ)) {
          nbt_node* tmp = FIND_NBT_NODE(pos, Pos);
          if (tmp && tmp->type == TAG_LIST) {
            nbt_node* pos = nbt_list_item(tmp, 2);
            if (pos && pos->type == TAG_DOUBLE) {
              snprintf(buf, end - buf, "%f", pos->payload.tag_double);
              while (*buf != '\0')
                buf++;
              f+= 3; // length of posz -1 as we already were at the first character
            }
          }
        } else if (string_startsWith(f, absorption)) {
          nbt_node* tmp = FIND_NBT_NODE(absorption, AbsorptionAmount);
          if (tmp && tmp->type == TAG_FLOAT) {
            snprintf(buf, end - buf, "%f", tmp->payload.tag_float);
            while (*buf != '\0')
              buf++;
            f += 9; // length of absorption -1 as we already were at the first character
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

void print_stat(json_t* json, char* username, const char* key, char** formats) {
  DEBUG(255, "print_stat(%p, %s, %s, %p);", json, username, key, formats);
  static char* USERNAME = "username";
  static char* ID = "id";
  static char* VALUE = "value";
  size_t i;
  for (i = 0; formats[i]; i++) {
    char b[BUFSIZ];
    char* buf = b;
    char* end = buf + sizeof(b);
    char* f;
    for (f = formats[i]; *f != '\0'; f++) {
      if (*f == '%') {
        f++;
        if (string_startsWith(f, USERNAME)) {
          APPEND(username);
          f += 7;
        } else if (string_startsWith(f, VALUE)) {
          if (json_is_integer(json)) {
            snprintf(buf, end - buf, "%lld", json_integer_value(json));
            while (*buf != '\0')
              buf++;
            f += 4; // length of value -1 as we already were at the first character
          }
        } else if (string_startsWith(f, ID)) {
          size_t dot = 0;
          size_t i = 0;
          for (i = 0; key[i]; i++) {
            if (key[i] == '.')
              dot = i + 1;
          }
          if (dot > 0) {
            snprintf(buf, end - buf, "%s", &key[dot]);
            while (*buf != '\0')
              buf++;
            f += 1; // length of id -1 as we already were at the first character
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

void print_level(nbt_node* nbt, char** formats) {
  static char* timeday = "timeday";
  DEFINE_TAG(seed);
  DEFINE_TAG(time);
  DEFINE_TAG(daytime);
  DEFINE_TAG(raining);
  DEFINE_TAG(thundering);
  size_t i;
  for (i = 0; formats[i]; i++) {
    char b[BUFSIZ];
    char* buf = b;
    char* end = buf + sizeof(b);
    char* f;
    for (f = formats[i]; *f != '\0'; f++) {
      if (*f == '%') {
        f++;
        if (string_startsWith(f, seed)) {
          nbt_node* tmp = FIND_NBT_NODE(seed, RandomSeed);
          if (tmp && tmp->type == TAG_LONG) {
            snprintf(buf, end - buf, "%ld", tmp->payload.tag_long);
            while (*buf != '\0')
              buf++;
            f += 3; // length of seed -1 as we already were at the first character
          }
        } else if (string_startsWith(f, timeday)) {
          nbt_node* tmp = FIND_NBT_NODE(daytime, DayTime);
          if (tmp && tmp->type == TAG_LONG) {
            snprintf(buf, end - buf, "%ld", tmp->payload.tag_long % 24000);
            while (*buf != '\0')
              buf++;
            f += 6; // length of timeday -1 as we already were at the first character
          }
        } else if (string_startsWith(f, time)) {
          nbt_node* tmp = FIND_NBT_NODE(time, Time);
          if (tmp && tmp->type == TAG_LONG) {
            snprintf(buf, end - buf, "%ld", tmp->payload.tag_long);
            while (*buf != '\0')
              buf++;
            f += 3; // length of time -1 as we already were at the first character
          }
        } else if (string_startsWith(f, daytime)) {
          nbt_node* tmp = FIND_NBT_NODE(daytime, DayTime);
          if (tmp && tmp->type == TAG_LONG) {
            snprintf(buf, end - buf, "%ld", tmp->payload.tag_long);
            while (*buf != '\0')
              buf++;
            f += 6; // length of daytime -1 as we already were at the first character
          }
        } else if (string_startsWith(f, raining)) {
          nbt_node* tmp = FIND_NBT_NODE(raining, raining);
          if (tmp && tmp->type == TAG_BYTE) {
            snprintf(buf, end - buf, "%s", (tmp->payload.tag_byte == 1) ? "true" : "false");
            while (*buf != '\0')
              buf++;
            f += 6; // length of raining -1 as we already were at the first character
          }
        } else if (string_startsWith(f, thundering)) {
          nbt_node* tmp = FIND_NBT_NODE(thundering, thundering);
          if (tmp && tmp->type == TAG_BYTE) {
            snprintf(buf, end - buf, "%s", (tmp->payload.tag_byte == 1) ? "true" : "false");
            while (*buf != '\0')
              buf++;
            f += 9; // length of thundering -1 as we already were at the first character
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