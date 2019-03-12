#!/usr/bin/env bash

gdbDummy=$1
testScript=$2

echo $gdbDummy
echo $testScript

$gdbDummy &
pidGdbDummy=$1

$testScript &
pidExpectScript=$!

wait $pidGdbDummy $pidExpectScript

