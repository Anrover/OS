#!/bin/bash

WIN_COMBINATIONS=(0 1 2 3 4 5 6 7 8 0 4 8 2 4 6 0 3 6 1 4 7 2 5 8)
CURSOR_X=0
CURSOR_Y=0
CHAR='X'
PLAYING_CHAR='X'

STEPS=()

function draw_map() {
	tput reset
echo 'Используй стрелки для выбора клетки. 
Нажми Enter, чтобы сделать ход.
Ты играешь' $CHAR
	echo '┏━━━┳━━━┳━━━┓'
	echo '┃   ┃   ┃   ┃'
	echo '┣━━━╋━━━╋━━━┫'
	echo '┃   ┃   ┃   ┃'
	echo '┣━━━╋━━━╋━━━┫'
	echo '┃   ┃   ┃   ┃'
	echo '┗━━━┻━━━┻━━━┛'
}

function move_cursor_up() {
    CURSOR_Y=$(((CURSOR_Y + 2) % 3))
}

function move_cursor_down() {
    CURSOR_Y=$(((CURSOR_Y + 1) % 3))
}

function move_cursor_left() {
    CURSOR_X=$(((CURSOR_X + 2) % 3))
}

function move_cursor_right() {
    CURSOR_X=$(((CURSOR_X + 1) % 3))
}

function put_char() {
	coord=$((CURSOR_Y * 3 + CURSOR_X))
	if [[ -z ${STEPS[$coord]} ]]; then
		echo -n $1
		set_cursor
		STEPS[$coord]=$1
		if [[ $PLAYING_CHAR = $CHAR ]]; then
			echo $CURSOR_Y $CURSOR_X > pipe
			PLAYING_CHAR=`get_enemy_char`
		fi
	fi
}

function make_move() {
    set_cursor
    read -rsn1 key
    case $key in
       'A') move_cursor_up;;
       'B') move_cursor_down;;
       'C') move_cursor_right;;
       'D') move_cursor_left;;
       '') put_char $CHAR;;
    esac
}

function get_enemy_char() {
    if [[ $CHAR = 'X' ]];
        then echo 'O'
        else echo 'X'
    fi
}

function set_cursor() {
	y=$((4+2*CURSOR_Y))
 	x=$((2+4*CURSOR_X))
 	tput cup $y $x;
}

function wait_enemy_move() {
	data=`cat pipe`
	enemy_step=($data)
	CURSOR_Y=${enemy_step[0]}
	CURSOR_X=${enemy_step[1]}
	set_cursor
	put_char `get_enemy_char`
	PLAYING_CHAR=$CHAR
}

function check_on_finished() {
	for (( i = 0; i < ${#WIN_COMBINATIONS[@]}; i+=3)); do
		x=${STEPS[WIN_COMBINATIONS[i]]}
		y=${STEPS[WIN_COMBINATIONS[i+1]]}
		z=${STEPS[WIN_COMBINATIONS[i+2]]}
		if  [[ $x = $y ]] && [[ $y = $z ]] && [[ $x != "" ]]; then
			tput rc
			echo "!!!!"$x "is WINNER!!!!"
			sleep 5
			exit
		elif [[ ${#STEPS[@]} = 9 ]]; then
			tput rc
			echo "..END OF THE GAME.."
			sleep 5
			exit
		fi
	done
}

function make_connect() {
	ch=`timeout 0.5s cat pipe`
	if [[ $ch = "" ]]; then
		echo `get_enemy_char` > pipe;
	else
		CHAR=$ch;
	fi
}

function main() {
    stty -echo
    mknod pipe p &>/dev/null
    trap 'rm pipe; reset' EXIT
    make_connect
    draw_map
    tput sc
    while true; do
        if [[ $PLAYING_CHAR = $CHAR ]]; 
            then make_move
            else wait_enemy_move
        fi

        check_on_finished
    done
}

main
