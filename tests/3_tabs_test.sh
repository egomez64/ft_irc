#!/usr/bin/bash

# Open 2 new terminal tabs
# tab01:
# 	connect immediately
# 	send a big message
# tab02:
# 	connect immediately
# 	send a medium message
# 	wait 1, then send a small message
# tab03:
# 	wait 1, then connect
# 	wait 1, then send a small message

wait_time=1
small_msg=""
medium_msg=""
big_msg=""

function netcat_connect {
	nc $1 $2
}

function tab01 {
	netcat_connect $1 $2
	$big_msg
}

function tab02 {
	netcat_connect $1 $2
	$medium_msg
	sleep $wait_time
	$small_msg
}

function tab03 {
	sleep $wait_time
	netcat_connect $1 $2
	sleep $wait_time
	$small_msg
}

gnome-terminal --tab -e tab02 $1 $2
gnome-terminal --tab -e tab03 $1 $2
tab01 $1 $2