#!/usr/bin/python

import sys
import psycopg2
import matplotlib.pyplot as plt
from matplotlib.dates import DateFormatter, MinuteLocator
import argparse

parser = argparse.ArgumentParser()
parser.add_argument("-d", "--database", help="what database to connect to", type=str, required=True)
parser.add_argument("-o", "--output", help="write to this file", type=str, required=True)
parser.add_argument("blocks", nargs="*", help="Block ids", default=[], type=int)
parser.add_argument("--title", type=str)
args = parser.parse_args()

conn = psycopg2.connect(database=args.database)
cur = conn.cursor()

for block in args.blocks:
  cur.execute("""SELECT \"when\", mined
                FROM minedblock
                WHERE block = %d
                AND \"when\" > now() - interval '1 day'""" % (block))
  times, counter = zip(*cur.fetchall())
  plt.plot(times, counter)

cur.close()
conn.close()

plt.grid()
if args.title:
  plt.title(args.title)
plt.gca().xaxis.set_major_formatter(DateFormatter('%H'))
plt.savefig(args.output)