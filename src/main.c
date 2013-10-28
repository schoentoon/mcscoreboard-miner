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

#include "debug.h"
#include "config.h"

#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <getopt.h>
#include <signal.h>
#include <unistd.h>

#include <event.h>

static const struct option g_LongOpts[] = {
  { "help",       no_argument,       0, 'h' },
  { "help-all",   no_argument,       0, 'H' },
  { "debug",      optional_argument, 0, 'D' },
  { "config",     required_argument, 0, 'c' },
  { 0, 0, 0, 0 }
};

static int usage(char* program) {
  fprintf(stderr, "USAGE: %s [options]\n", program);
  fprintf(stderr, "-h, --help\t\tShow this help message.\n");
  fprintf(stderr, "-H, --help-all\t\tExtended help message explaining all the format options.\n");
  fprintf(stderr, "-D, --debug\t\tIncrease debug level.\n");
  fprintf(stderr, "\t\t\tYou can also directly set a certain debug level with -D5\n");
  fprintf(stderr, "-c, --config [config]\tConfig file.\n");
  return 0;
};

static int extended_help() {
  fprintf(stderr, "The 'worldpath' parameter is required and must be the full path to your world.\n");
  fprintf(stderr, "The following format parameters are optional and can be used multiple times.\n");
  fprintf(stderr, "'scoreboard_format', this will be printed based on scoreboard changes.\n");
  fprintf(stderr, "You can use the following parameters in these formats.\n");
  fprintf(stderr, "  %%name       This will get replaced by the username of which the objective is.\n");
  fprintf(stderr, "  %%objective  The name of the actual objective.\n");
  fprintf(stderr, "  %%score      The score of this objective.\n");
  fprintf(stderr, "'players_format', this will be printed based on changes to the players/*.dat files.\n");
  fprintf(stderr, "  %%username   The username of the player.\n");
  fprintf(stderr, "  %%level      The amount of levels this player has.\n");
  fprintf(stderr, "  %%xp         The amount of xp this player has, this will be something like 0.2341.\n");
  fprintf(stderr, "  %%xplevel    Level + xp, so something like 26.2341.\n");
  fprintf(stderr, "  %%health     The amount of 'hearts' the player has, half a heart is printed as 1 (20 max).\n");
  fprintf(stderr, "  %%food       The amount of drumsticks the player has, just like %%health it is * 2.\n");
  fprintf(stderr, "  %%score      Score of the player, which would be printed upon death.\n");
  fprintf(stderr, "  %%absorption Amount of extra health added by the absorption effect.\n");
  fprintf(stderr, "  %%PosX       X position of the player.\n");
  fprintf(stderr, "  %%PosY       Y position of the player.\n");
  fprintf(stderr, "  %%PosZ       Z position of the player.\n");
  fprintf(stderr, "'level_format', this will be printed based on level.dat changes.\n");
  fprintf(stderr, "  %%seed       The seed of the world, won't really change.\n");
  fprintf(stderr, "  %%time       The number of ticks since the start of the level.\n");
  fprintf(stderr, "  %%daytime    The time of the day, keeps counting.\n");
  fprintf(stderr, "  %%timeday    The time of the day %% 24000, 0 is sunrise, 6000 is mid dag, 12000 sunset, 18000 mid night.\n");
  fprintf(stderr, "  %%raining    true if it's raining, false otherwise.\n");
  fprintf(stderr, "  %%thundering true if thundering (%%rain will also be true), false otherwise.\n");
  return 0;
};

int main(int argc, char** argv) {
  int arg, optindex;
  while ((arg = getopt_long(argc, argv, "hHD::c:", g_LongOpts, &optindex)) != -1) {
    switch (arg) {
    case 'h':
      return usage(argv[0]);
    case 'H':
      return extended_help();
    case 'D':
      if (optarg) {
        errno = 0;
        long tmp = strtol(optarg, NULL, 10);
        if (errno == 0 && tmp < 256)
          debug = (unsigned char) tmp;
        else
          fprintf(stderr, "%ld is an invalid debug level.\n", tmp);
      } else
        debug++;
      break;
    case 'c':
      if (parse_config(optarg) == 0)
        return 1;
      break;
    }
  }
  if (config_is_empty())
    return usage(argv[0]);
  struct event_base* event_base = event_base_new();
  if (dispatch_config(event_base) == 0)
    while (event_base_dispatch(event_base) == 0);
  else
    return 1;
  return 0;
};
