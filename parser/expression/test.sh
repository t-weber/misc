#!/bin/bash
#
# simple LR(1) operator precedence expression test.
#
# @author Tobias Weber
# @date 14-dec-19
# @license see 'LICENSE.EUPL' file
#

PROG1=./lr1_opprec
PROG2=./ll1_expr
PROG3=julia

EXPRESSIONS=( "2+(3*4)-5"
	"-2+(3*4)-5"
	"-(2+3)^(2*7-10)+500"
	"(1+2+3+4+5)-(6+7+8+9)*2^3"
)


for EXPR in ${EXPRESSIONS[@]}; do
	echo "Testing ${EXPR}"
	OUT1=$(echo ${EXPR} | $PROG1)
	OUT2=$(echo ${EXPR} | $PROG2)
	OUT3=$(echo ${EXPR} | $PROG3)

	echo "Output 1: ${OUT1}, Output 2: ${OUT2}, Output 3: ${OUT3}"
	if [ ${OUT1} != ${OUT3} ] || [ ${OUT2} != ${OUT3} ]; then
		echo -e "\e[1mInvalid result!\e[25m"
	fi

	echo -e ""
done
