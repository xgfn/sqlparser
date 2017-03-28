#!/bin/bash
FILENAME=$1
i=0
STATEMENT=`basename -s .txt $FILENAME`
printf "\033[0;33m{Test $STATEMENT SQL}\033[0m\n"

while read line
do 
    i=`expr $i + 1`;
    #printf "\n\033[0;33mTEST$i\033[0m\n"
    echo "${line}" | xargs -0 ./analyzescript
    if [ $? != 0 ]; then
	printf "\033[0;31mTEST$i [FAIL] \033[0m\n\n"
    else
	printf "\033[0;32mTEST$i [SUCCESS] \033[0m\n\n"
    fi
done < $FILENAME
