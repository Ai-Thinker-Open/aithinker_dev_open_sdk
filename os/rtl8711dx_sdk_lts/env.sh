#!/bin/bash

./patch.sh
source sdk/ameba-rtos/ameba.sh
export CMAKE_COLOR_DIAGNOSTICS=ON
if [ $? -ne 0 ]
then
	echo -e "\e[31mpathc error\e[0m"
	return 1
fi

return 0

