#!/usr/bin/python

import sys
import psycopg2
import matplotlib

matplotlib.use('Agg') # Force it to not use Xwindows so it can be ran headless http://stackoverflow.com/a/3054314

import matplotlib.pyplot as plt
from matplotlib.dates import DateFormatter, MinuteLocator
import argparse

parser = argparse.ArgumentParser()
parser.add_argument("-d", "--database", help="what database to connect to", type=str, required=True)
parser.add_argument("-o", "--output", help="write to this file", type=str, required=True)
parser.add_argument("-mb", "--minedblocks", nargs="*", help="mined block id", default=[], type=int)
parser.add_argument("-ci", "--crafteditem", nargs="*", help="crafted item id", default=[], type=int)
parser.add_argument("-s", "--stat", nargs="*", help="the misc stats", default=[], type=str)
parser.add_argument("-u", "--username", help="only select data from this username (multiplayer)", type=str)
parser.add_argument("--since", help="start timestamp (directly passed to the database so read the PostgreSQL documentation regarding timestamps)", type=str, default="now() - interval '1 day'")
parser.add_argument("--till", help="select until this timestamp, example \"'1 november 2012 00:00'\"", type=str, default="now()")
parser.add_argument("--title", type=str)
args = parser.parse_args()

conn = psycopg2.connect(database=args.database)
cur = conn.cursor()

for block in args.minedblocks:
  try:
    cur.execute("""SELECT \"when\", mined
                  FROM minedblock
                  WHERE block = %d
                  %s
                  AND \"when\" between (%s) and (%s)""" % (block, ("AND name = '%s'" % (args.username) if args.username else ""), args.since, args.till))
    times, counter = zip(*cur.fetchall())
    plt.plot(times, counter)
  except Exception as e:
    print e

for item in args.crafteditem:
  try:
    cur.execute("""SELECT \"when\", times
                  FROM crafteditem
                  WHERE item = %d
                  %s
                  AND \"when\" between (%s) and (%s)""" % (item, ("AND name = '%s'" % (args.username) if args.username else ""), args.since, args.till))
    times, counter = zip(*cur.fetchall())
    plt.plot(times, counter)
  except Exception as e:
    print e

for stat in args.stat:
  try:
    cur.execute("""SELECT \"when\", count
                   FROM stats
                   WHERE stat = '%s'
                   %s
                   AND \"when\" between (%s) and (%s)""" % (stat, ("AND name = '%s'" % (args.username) if args.username else ""), args.since, args.till))
    times, counter = zip(*cur.fetchall())
    plt.plot(times, counter)
  except Exception as e:
    print e

cur.close()
conn.close()

plt.grid()
if args.title:
  plt.title(args.title)
plt.gca().xaxis.set_major_formatter(DateFormatter('%H'))
plt.savefig(args.output)