cat logs/* | grep -oP "\K\w+(?=[0-9]* us)" > data.out

SUM=0
NUM_LINES=0
while read p; do
	SUM=$(($SUM+$p))
	NUM_LINES=$(($NUM_LINES+1))
done < data.out
echo $(($SUM / $NUM_LINES))
