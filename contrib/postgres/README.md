mcscoreboard-miner + postgresql
===============================

First of all import postgresql.schema into your postgresql database. Then modify sample.config (from the contrib/postgres directory) to your likes. Then simply run mcscoreboard-miner like this:
```
$ ./mcscoreboard-miner -c sample.config | psql -f -
```
You might have to tweak both commands depending on your setup.
