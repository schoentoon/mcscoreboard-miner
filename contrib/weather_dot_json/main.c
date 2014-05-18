/*  mcscoreboard-miner
 *  Copyright (C) 2013-2014  Toon Schoenmakers
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

#include <stdio.h>
#include <getopt.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>

#include <jansson.h>

static const struct option g_LongOpts[] = {
  { "help",       no_argument,       0, 'h' },
  { "output",     required_argument, 0, 'o' },
  { 0, 0, 0, 0 }
};

static int usage(char* program) {
  return 1;
}

int main(int argc, char** argv) {
  int arg, optindex;
  char* output = NULL;
  while ((arg = getopt_long(argc, argv, "ho:", g_LongOpts, &optindex)) != -1) {
    switch (arg) {
    case 'h':
      return usage(argv[0]);
    case 'o':
      output = optarg;
      break;
    }
  }
  if (output == NULL) {
    fprintf(stderr, "No output specified?\n");
    return 1;
  }
  ssize_t n;
  char buff[BUFSIZ];
  int in = fileno(stdin);
  while ((n = read(in, buff, sizeof(buff))) > 0) {
    long daytime;
    char thunder[6]; // false + '\0' is 6 characters long ;)
    char rain[6];
    if (sscanf(buff, "%ld %5s %5s", &daytime, thunder, rain) == 3) {
      json_t* json = json_object();
      if (json) {
        json_t* src = NULL;
        if (strcmp(thunder, "true") == 0)
          src = json_string((daytime%24000) < 12000 ? "weather/weather_thunder_day.png" : "weather/weather_thunder_night.png");
        else if (strcmp(rain, "true") == 0)
          src = json_string((daytime%24000) < 12000 ? "weather/weather_stormy_day.png" : "weather/weather_stormy_night.png");
        else
          src = json_string((daytime%24000) < 12000 ? "weather/weather_sunny_day.png" : "weather/weather_sunny_night.png");
        if (src && json_object_set(json, "src", src) == 0)
          json_dump_file(json, output, 0);
        json_decref(json);
      }
    }
  }
  return 1;
};