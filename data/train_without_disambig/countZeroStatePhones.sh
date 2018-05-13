phones=`cat X_sorted | cut -f3`
declare -A arr
for phone in $phones; do
	i=1
	while read -r line; do
		x=`echo $line | cut -d' ' -f2-`		
		for p in $x; do
			echo "Phone:$phone-Line:$i-TrainPhone:$p"
			if [ "$phone" == "$p" ]; then
				((arr[$phone]++))
			fi
		done
		((i++))
	done < text
done

for key1 in "${!arr[@]}"; do
	echo $key1" "${arr[$key1]}
	echo $key1" "${arr[$key1]} >> count
done