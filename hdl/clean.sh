#!/bin/bash
#
# @author Tobias Weber <tobias.weber@tum.de>
# @date dec-2020
# @license see 'LICENSE.EUPL' file
#

dirs=( "." "lib" "test" "calc" )

for dir in "${dirs[@]}"; do
	pushd "${dir}"

	rm -fv *.o
	rm -fv testbed-thetester
	rm -fv *.cf
	rm -fv *.vcd

	popd
done
