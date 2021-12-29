if [ -z $1 ]; then
	path="./"
else
	path=$1
fi

token=`cat "${path}token.txt"`
if [ -z $token ]; then
	echo "Missing access token. Please include it in ${path}token.txt"
	exit
fi

py_version=`python -c 'import sys; print(sys.version_info.major)'`
if [[ py_version -eq 3 ]]; then
	py_command="python"
fi
if [[ py_version -eq 2 ]]; then
	py_version=`python3 -c 'import sys; print(sys.version_info.major)'`
	py_command="python3"
fi
if [[ -z $py_version ]]; then
	echo "Please install latest python 3."
	exit
fi
# echo $py_version
GET "https://umjicanvas.com/api/v1/users/self/favorites/courses?access_token=$token" > /tmp/temp.txt
courses=($($py_command /home/pi/project/canvas2/contri/canvas.py xxx -f -d $path | tr -d '[],'))
rm /tmp/temp.txt
# echo ${#courses[@]}
len_course=${#courses[@]}
# echo ${courses[@]}
if [[ ${courses[0]} -eq -1 ]]; then
	echo "ID not exists, or something went wrong with your token, please check."
	exit
fi
if [[ ${courses[1]} -eq -1 ]]; then
	echo "You do not have any course this semester, Bye!"
	exit
fi

$py_command  /home/pi/project/canvas2/contri/canvas.py xxx -c -d $path
for (( i = 1; i < ${len_course}; i += 2 ));
do
	echo "Fetching data from ${courses[$(($i + 1))]}"
	for j in {1..5}
	do
		course_id=${courses[$i]}
		course_name=${courses[$(($i + 1))]}
		var=`GET "https://umjicanvas.com/api/v1/courses/$course_id/users?access_token=$token&include[]=email&include[]=enrollments&per_page=50&page=$j"`
		len=`echo -n $var | wc -m`
		if [ $len -gt 100 ]; then
			echo $var >> /tmp/$course_name.txt
		else
			break
		fi
	done
	$py_command  /home/pi/project/canvas2/contri/canvas.py $course_name -d $path
	rm /tmp/$course_name.txt
done

my_activity=0
for (( i = 1; i < ${len_course}; i += 2 ));
do
	course_name=${courses[$(($i + 1))]}
	delta=`$py_command  /home/pi/project/canvas2/contri/canvas.py $course_name -m ${courses[0]} -d $path`
	my_activity=$((my_activity + $delta))
	$py_command  /home/pi/project/canvas2/contri/canvas.py $course_name -r -d $path
done
$py_command  /home/pi/project/canvas2/contri/canvas.py xxx -s $my_activity -d $path
$py_command  /home/pi/project/canvas2/contri/canvas.py xxx -p -d $path
