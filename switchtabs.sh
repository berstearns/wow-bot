#!/bin/bash
# xdotool windowactivate  xdotool search --desktop=0 --name "Window Title"

function switchToWindow(){
	# $1 => $TOID
	# $2 => $POS_X
	# $3 => $POX_Y
	# xdotool windowactivate $TOID;xdotool mousemove $POS_X $POS_Y
	xdotool windowactivate $1;xdotool mousemove $2 $3
	sleep 0.05
	xdotool mousedown 1
	xdotool mouseup 1
}

function selectNearestTarget() {
	sleep 0.2 && xdotool key "p"
	sleep 0.2 && xdotool key "2"
	# sleep 0.2 && xdotool click
	# xdotool mousedown 1
	# sleep 0.2 && xdotool click
	# xdotool mouseup 1
}

echo $1
TOID=$1
TOPOS=$2

case $TOPOS in

  1)
    FROMPOS=2
    POS_X=480
    POS_Y=250
    ;;

  2)
    FROMPOS=1
    POS_X=1400
    POS_Y=250
    ;;

  3)
    POS_X=800
    POS_Y=850
    ;;

  4)
    POS_X=1400
    POS_Y=850
    ;;

  *)
    ;;
esac

myArray=( switchToWindow )#selectNearestTarget )
myParams1=($TOID $POS_X $POS_Y) 
myParams2=(3,4,5)
for func in ${myArray[@]}; do
    $func "${myParams1}"
done
