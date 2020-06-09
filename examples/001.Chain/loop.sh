echo "START"
while [ $? -eq 0 ]
do
    for i in {0..7}
    do
    rm -r ./node_$i/
    done
    rm -r ./node_shared/

    node ./prepare.js
done
echo "END"

