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

#ifndef _CONFIG_H
#define _CONFIG_H

#include <event2/dns.h>
#include <event2/event.h>
#include <event2/event_struct.h>

struct config {
  char* world_path;
  char** scoreboard_format;
  char** players_format;
  char** level_format;
  char** stats_useItem_format;
  char** stats_mineBlock_format;
  char** stats_killEntity_format;
  char** stats_killedByEntity_format;
  char** stats_craftItem_format;
  char** stats_format;
  int data_wd;
  int players_wd;
  int level_wd;
  int stats_wd;
};

int parse_config(char* filename);

int dispatch_config(struct event_base* base);

int config_is_empty();

#endif //_CONFIG_H