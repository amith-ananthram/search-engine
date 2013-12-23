# This is a script used to test indexer.c
# It produces a timestamped log file with the results.

outputfile="indexer_testlog"

make clean >> "$outputfile"
make indexer >> "$outputfile"

if [ $? -ne 0 ] 
    then
        echo "Could not compile" >> "$outputfile"
        exit 1
fi

echo "Compiled correctly!" >> "$outputfile"

echo "# of arguments test!" >> "$outputfile"

./indexer ../crawler/data index.dat pizza >> "$outputfile"
if [ $? -ne 1 ] 
    then
        echo "# of arguments test FAILED." >> "$outputfile"
        exit 1
fi

echo "# of arguments test passed!" >> "$outputfile"

echo "Bad target directory test!" >> "$outputfile"

./indexer ../../this/does/not/exist index.dat >> "$outputfile"
if [ $? -ne 1 ] 
    then
        echo "wrong target directory test FAILED." >> "$outputfile"
        exit 1
fi

echo "Bad target directory test passed!" >> "$outputfile"

echo "Testing regular functionality after crawling at depth 3" >> "$outputfile"

./indexer ../crawler/data index.dat >> "$outputfile"

echo "Test complete.  For results, look at file 'index.dat'" >> "$outputfile"

rm -f ../crawler/data/index.dat

echo "Testing ability to read in an index file, store in data structures, and output it again." >> "$outputfile"

./indexer ../crawler/data index.dat index.dat testindex.dat >> "$outputfile"

echo "The following is the results of diff index.dat testindex.dat" >> "$outputfile"

diff ../crawler/data/index.dat ../crawler/data/testindex.dat >> "$outputfile"

echo "Indexer testing complete!"
