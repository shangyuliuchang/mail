if [ ! -f "/home/pi/project/canvas/con/token.txt" ]; then
	echo "Missing access token. Please include it in ./token.txt"
	exit
fi

token=`cat /home/pi/project/canvas/con/token.txt`
if [ -z $token ]; then
	echo "Missing access token. Please include it in ./token.txt"
	exit
fi

GET "https://umjicanvas.com/api/v1/users/self/favorites/courses?access_token=$token" > /tmp/temp.txt
courses=($(python3 /home/pi/project/canvas/con/canvas.py xxx -f | tr -d '[],'))
rm /tmp/temp.txt
len_course=${#courses[@]}

for (( i = 1; i < ${len_course}; i += 2 ));
do
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
	python3 /home/pi/project/canvas/con/canvas.py $course_name
	echo "Fetching data from $course_name"
	rm /tmp/$course_name.txt
done

my_activity=0
for (( i = 1; i < ${len_course}; i += 2 ));
do
	course_name=${courses[$(($i + 1))]}
	delta=`python3 /home/pi/project/canvas/con/canvas.py $course_name -m ${courses[0]}`
	my_activity=$((my_activity + $delta))
	python3 /home/pi/project/canvas/con/canvas.py $course_name -r
done
python3 /home/pi/project/canvas/con/canvas.py xxx -s $my_activity
python3 /home/pi/project/canvas/con/canvas.py xxx -p