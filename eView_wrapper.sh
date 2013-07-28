#!/bin/sh 
`dirname $0`/eView061_russian_debug >`dirname $0`/eView_log.txt 2>&1 || messagebox "Cannot start eView"
