POLICY=$1

function verify {
    file1=$1
    file2=$2
    echo -e "\t[DIFFING] $file1 $file2"
    if diff "$file1" "$file2" &> /dev/null ; then
        echo -e "\t\e[1;34m[PASS]\e[0m"
    else
        echo -e "\t\e[1;31m[FAIL]\e[0m"
    fi
}

function testFirstFit {
    echo "[TESTING FIRST FIT]"
    ./memory_test_1 > /dev/null 2>&1
    ./memory_test_6 > /dev/null 2>&1

    verify test1_output.txt ../TestOutputs/test1_output.txt
    verify test6_output.txt ../TestOutputs/test6_output.txt
}

function testBestFit {
    echo "[TESTING BEST FIT]"
    ./memory_test_2 > /dev/null 2>&1
    ./memory_test_7 > /dev/null 2>&1

    verify test2_output.txt ../TestOutputs/test2_output.txt
    verify test7_output.txt ../TestOutputs/test7_output.txt
}

function testWorstFit {
    echo "[TESTING WORST FIT]"
    ./memory_test_3 > /dev/null 2>&1
    ./memory_test_8 > /dev/null 2>&1

    verify test3_output.txt ../TestOutputs/test3_output.txt
    verify test8_output.txt ../TestOutputs/test8_output.txt
}

function testBuddySystem {
    echo "[TESTING BUDDY SYSTEM]"
    ./memory_test_4 > /dev/null 2>&1
    ./memory_test_5 > /dev/null 2>&1

    verify test4_output.txt ../TestOutputs/test4_output.txt
    verify test5_output.txt ../TestOutputs/test5_output.txt
}

./build.sh

if [ "$POLICY" = "all" ]
then
    echo "[TESTING ALL POLICIES]"
    testFirstFit
    testBestFit
    testWorstFit
    testBuddySystem
elif [ "$POLICY" = "0" ]
then
    testFirstFit
elif [ "$POLICY" = "1" ]
then
    testBestFit
elif [ "$POLICY" = "2" ]
then
    testWorstFit
elif [ "$POLICY" = "3" ]
then
    testBuddySystem
fi