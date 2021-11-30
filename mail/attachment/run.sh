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

GET "https://umjicanvas.com/api/v1/users/self/favorites/courses?access_token=$token" > /tmp/temp.txt
courses=($(python ./canvas.py xxx -f -d $path | tr -d '[],'))
rm /tmp/temp.txt
# echo ${#courses[@]}
len_course=${#courses[@]}

python ./canvas.py xxx -c -d $path
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
	python ./canvas.py $course_name -d $path
	rm /tmp/$course_name.txt
done

my_activity=0
for (( i = 1; i < ${len_course}; i += 2 ));
do
	course_name=${courses[$(($i + 1))]}
	delta=`python ./canvas.py $course_name -m ${courses[0]} -d $path`
	my_activity=$((my_activity + $delta))
	python ./canvas.py $course_name -r -d $path
done
python ./canvas.py xxx -s $my_activity -d $path
python ./canvas.py xxx -p -d $path