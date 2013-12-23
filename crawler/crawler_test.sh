# This is a script used to test crawler.c
# It produces a timestamped log file with the results.

outputfile="crawler_testlog"

make clean >> "$outputfile"
make crawler >> "$outputfile"

if [ $? -ne 0 ] 
    then
        echo "Could not compile" >> "$outputfile"
        exit 1
fi

./crawler http://www.cs.dartmouth.edu logged_files >> "$outputfile"
if [ $? -ne 1 ] 
    then
        echo "# of arguments test FAILED." >> "$outputfile"
        exit 1
fi

./crawler http://www.cs.dartmouth.edu logged_files/pizza 3 >> "$outputfile"
if [ $? -ne 1 ] 
    then
        echo "wrong target directory test FAILED." >> "$outputfile"
        exit 1
fi

./crawler http://www.cs.dartmouth.edu logged_files 10 >> "$outputfile"
if [ $? -ne 1 ] 
    then
        echo "max depth test FAILED." >> "$outputfile"
        exit 1
fi

./crawler http://www.cs.dartmouth.edu data 3 >> "$outputfile"

echo "Crawler testing complete!"
