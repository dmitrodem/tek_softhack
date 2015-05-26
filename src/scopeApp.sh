#!/bin/sh
LD_PRELOAD=/usr/local/bin/override.so
export LD_PRELOAD
exec /usr/local/bin/scopeApp.ppc8xx
