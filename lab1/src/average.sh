#!/usr/bin/bash
average=0
amount=0
while read number
do
amount=$(($amount+1))
average=$(($average+$number))
done
average=$(($average/$amount))
echo "Число аргументов равно $amount"
echo "Среднее арифметическое равно $average"
