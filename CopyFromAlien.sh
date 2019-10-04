#!/bin/bash

task=$1

file=AnalysisResults.root

# echo $task

# path=$(pwd)

# echo 'task name '$task' is used'
# echo 'path is '$path''

counter=1

touch README.txt 
printf $task > README.txt 

while read; do 
 [[ -z $REPLY ]] && continue
 echo file is: $REPLY
 echo run directory: $(dirname $(dirname $REPLY)) 
 alien_cp alien:$REPLY Outputs_to_merge/AnalysisResults_${counter}.root
 let "counter++"
 echo chunk directory: $(dirname $REPLY)
done < <(alien_find /alice/cern.ch/user/s/skonigst/${task}_WorkingDir/${task}_OutputDir $file)

echo DONE

return 

for a in `alien_ls /alice/cern.ch/user/s/skonigst/$task\_WorkingDir/$task\_OutputDir/`
do 
	for b in `alien_ls /alice/cern.ch/user/s/skonigst/$task\_WorkingDir/$task\_OutputDir/$a/`
	do
	    if [ ! -f $path/$file_$a_$b.root ]; then 	
		    alien_cp alien:/alice/cern.ch/user/s/skonigst/$task\_WorkingDir/$task\_OutputDir/$a/$b/$file.root $path/$file_$a_$b.root
		    echo "copying" $a
	    fi
	done
done
