#!/bin/bash
./test 2>&1 | tee test_output.txt
echo "Test exit code: $?"