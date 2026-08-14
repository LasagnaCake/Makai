#! usr/bin/bash

mkdir output

echo "" > output/result.txt

rm -rf output/result.*.txt

{ python python test.perf.01.loops.py > output/result.01.py.txt }&
{ lua test.perf.01.loops.lua > output/result.01.lua.txt }&
{ ../../../output/bin/art test.perf.01.loops.bv -S -C > output/result.01.bv.txt }&

while ! test -f "output/result.01.bv.txt"; do sleep 1 done
while ! test -f "output/result.01.lua.txt"; do sleep 1 done
while ! test -f "output/result.01.py.txt"; do sleep 1 done

match = "(?:total-time:\w*)([0-9._]+)"

echo "breve [" >> output/result.txt
grep "$match" "output/result.01.bv.txt" | echo >> output/result.flow
echo "]" >> output/result.txt

echo "lua [" >> output/result.txt
grep "$match" "output/result.01.lua.txt" | echo  >> output/result.flow
echo "]" >> output/result.txt

echo "python [" >> output/result.txt
grep "$match" "output/result.01.py.txt" | echo >> output/result.flow
echo "]" >> output/result.txt