#!/usr/bin/env bash

TEST_PATH="/home/jeonghun/SNU/Lecture/ComputerArchitecture/pyrisc/asm/sum100"

/home/jeonghun/SNU/Lecture/ComputerArchitecture/assignments/ca-pa4/snurisc6.py -l 4 "${TEST_PATH}" > ./snurisc6_result.txt
/home/jeonghun/SNU/Lecture/ComputerArchitecture/pyrisc/pipe5/snurisc5.py -l 4 "${TEST_PATH}" > ./snurisc5_result.txt

diff ./snurisc6_result.txt ./snurisc5_result.txt > compare.txt