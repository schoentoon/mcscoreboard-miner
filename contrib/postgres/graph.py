#!/usr/bin/python

import psycopg2
import matplotlib.pyplot as plt
from matplotlib.dates import DateFormatter, MinuteLocator
import config

conn = psycopg2.connect(database="schoentoon")
cur = conn.cursor()
cur.execute("""SELECT \"when\", mined
               FROM minedblock
               WHERE block = 1
               AND \"when\" > now() - interval '1 day'""")
times, counter = zip(*cur.fetchall())
cur.close()
conn.close()

plt.plot(times, counter)
plt.grid()
plt.title("Mined stone for the past 24 hours.")
plt.gca().xaxis.set_major_formatter(DateFormatter('%H'))
plt.savefig("stone.png")