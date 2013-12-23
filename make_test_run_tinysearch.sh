# Tests the Tiny Search Engine

outputfile="tinysearchengine_testlog"

echo "Testing crawler" >> "$outputfile"

cd crawler
make clean >> "$outputfile"
make crawler >> "$outputfile"

./crawler_test.sh >> "$outputfile"

echo "Testing indexer" >> "$outputfile"

cd ../index
make clean >> "$outputfile"
make indexer >> "$outputfile"

./indexer_test.sh >> "$outputfile"

cd ../queryengine
make clean >> "$outputfile"
make query >> "$outputfile"
make query_test >> "$outputfile"

echo "Unit testing queryengine" >> "$outputfile"

query_test >> "$outputfile"

query ../crawler/data/index.dat ../crawler/data/

echo "Testing complete!" >> "$outputfile"
