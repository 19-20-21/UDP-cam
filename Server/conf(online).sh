#!/bin/sh         
#FileName:conf.sh
#Function:配置文件
#Version:V0.1   
#Date:2019-06
if [ "root" != "$LOGNAME" ]
then
	echo "请先root！"
	exit
fi
year=`date +%y`		
month=`date +%m`
day=`date +%d`
hour=`date +%H`
minute=`date +%M`
second=`date +%S`
		echo " "
		echo "             time: 20$year/$month/$day $hour:$minute:$second"
		echo ""
		echo "             ========================================    "
		echo "             *        初  始  化  完  毕       *    "
		echo "             ----------------------------------------    "
		echo "             *   1. 开   始  安   装           *    "
		echo "             *   2. 退   出  系   统           *    "
		echo "             ========================================    "   
		echo "             请选择: "
		stty -echo
		read Keyboard
		stty echo
		case $Keyboard in
			1)	
				sudo apt-get update
				sudo apt-get install libopencv-dev:i386 libopencv-dev
				;;
			2)
				exit
				;;
			*)
				echo "error input!"
				exit
				;;
		esac
		echo"           -----------------------------------------------"
		echo "安装完毕。"
