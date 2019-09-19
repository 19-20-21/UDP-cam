echo "请输入服务器地址"
read seveip
echo "请输入服务器端口"
read seveport
echo "请输入rstp地址"
read rstpip

./build/Client ${seveip} ${seveport} ${rstpip}
