#!/bin/sh

filesdir=$1
searchstr=$2

if [ $# -ne 2 ]
then
	echo "usage: $0 <filesdir> <searchstr>"
	exit 1
fi

if  [ ! -d $filesdir ]
then
	echo "$filesdir not found - please enter a valid path"
	exit 1
fi
	
#nr_files=`ls -1 | wc -l`
nr_files=0
nr_matches=0

# Loop through files in the search directory
for file in `find $filesdir -type f`
### Use 'find' with a 'while' loop to handle spaces in filenames safely
###find "$filesdir" -type f | while read -r file
	do
		if [ -f "$file" ]
		then
			nr_files=$((nr_files+1))
			# echo "$file"
			matches=$(grep "$searchstr" "$file" | wc -l)
			# echo $matches
			nr_matches=$((nr_matches+matches))
		fi
	done

echo "The number of files are $nr_files and the number of matching lines are $nr_matches"

###  Y die Anzahl der übereinstimmenden Zeilen in den jeweiligen Dateien ist, wobei sich eine 
### übereinstimmende Zeile auf eine Zeile bezieht, die searchstr  enthält (und auch zusätzlichen 
### Inhalt enthalten kann).

