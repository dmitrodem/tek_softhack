#!/bin/sh
$(awk 'BEGIN {printf("md5sum ")} {printf("%s ", $2)} END {printf("\n")}' md5sum.txt) > md5sum.txt
