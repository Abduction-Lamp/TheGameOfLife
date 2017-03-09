#pragma once

#include <Windows.h>



const int MINSIZEWIDTH_MAIN = 840;
const int MINSIZEHEIGHT_MAIN = 590;

const int MINSIZEWIDTH_BOARD = 600;
const int MINSIZEHEIGHT_BOARD = 490;

const int MINSIZEWIDTH_TREE = 200;
const int MINSIZEHEIGHT_TREE = 300;


struct GHWND
{
	HWND main_wnd;

	HWND button_clear;
	HWND button_random;
	HWND button_play_pause;

	HWND track_speed;
	HWND track_size;

	HWND board;
	HWND tree_view;

	HWND label;

	HWND status;
};



enum PlayFlag {PLAY, PAUSE};
