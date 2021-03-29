#!/bin/bash
#
# protected mode test
# @author Tobias Weber
# @date mar-21
# @license: see 'LICENSE.GPL' file
#


USE_EMULATOR=0


case ${USE_EMULATOR} in
	0)
		if ! qemu-system-x86_64 -drive format=raw,file=lm.x86,index=0; then
			echo -e "Error running qemu."
			exit -1
		fi
	;;

	1)
		if ! bochs "floppya: 1_44=lm.x86, status=inserted" "boot:floppy"; then
			echo -e "Error running bochs."
			exit -1
		fi
	;;

	*)
		echo -e "Unknown emulator selected."
		exit -1
	;;
esac
