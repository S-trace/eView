#!/bin/sh

root=/mnt/onboard

for file in `find $root/.kobo/fmon -name 'fmon_*.sh' -print`
do
    echo "Executing fmon script: $file"
    . $file
done

