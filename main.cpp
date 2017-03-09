#pragma region Includes and Manifest Dependencies

#define _CRT_SECURE_NO_WARNINGS
#define _CRT_NON_CONFORMING_SWPRINTFS

#include <Windows.h>
#include <tchar.h>
#include <ctime>
#include <cmath>
#include <vector>


#include "Resource.h"
#include "DataGuiElements.h"

#include <Commctrl.h>
#pragma comment(lib, "comctl32.lib")

#pragma comment(linker,"\"/manifestdependency:type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#pragma endregion




LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK BoardProc(HWND, UINT, WPARAM, LPARAM);


void CreateGUI(const HWND &hMainWnd, struct GHWND * const ghWnd);

BOOL ** CreateMatrix(size_t iSize, size_t jSize);
void DeleteMatrix(BOOL **data, size_t iSize);



int APIENTRY _tWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPTSTR cmd, int mode)
{
	HWND		hMainWnd;
	MSG			msg;
	WNDCLASSEX	wcex;

	wcex.hInstance		= hInstance;
	wcex.lpszClassName	= _T("MainFrame");
	wcex.lpfnWndProc	= WndProc;
	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.hIcon			= LoadIcon(hInstance, MAKEINTRESOURCE(IDI_MAIN_ICON));
	wcex.hIconSm		= NULL;
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.lpszMenuName	= MAKEINTRESOURCE(IDRC_MAIN_MENU);
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.cbSize			= sizeof(WNDCLASSEX);
	wcex.hbrBackground	= (HBRUSH)(COLOR_3DFACE + 1);

	if (!RegisterClassEx(&wcex))
	{
		MessageBox(NULL, _T("Не удалось зарегистрировать класс окна"), _T("Ошибка"), MB_OK | MB_ICONERROR);
		return 1;
	}


	hMainWnd = CreateWindowEx(
		NULL,
		_T("MainFrame"),
		_T("Conway's Game of Life"),
		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_THICKFRAME | WS_CLIPCHILDREN,
		CW_USEDEFAULT, CW_USEDEFAULT,
		MINSIZEWIDTH_MAIN, MINSIZEHEIGHT_MAIN,
		HWND_DESKTOP,
		NULL,
		hInstance,
		NULL
	);

	if (!hMainWnd)
	{
		MessageBox(NULL, _T("Не удалось создать окно"), _T("Ошибка"), MB_OK | MB_ICONERROR);
		return 1;
	}


	InitCommonControls();
	ShowWindow(hMainWnd, mode);

	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return msg.wParam;
}



LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static struct GHWND		ghWnd;

	static RECT				defaultClientRect;
	int						deltaX,
							deltaY;

	static int				speed,
							scale;

	TCHAR					str_buffer[MAX_PATH];

	static PlayFlag			playFlag;



	switch (message)
	{
	case WM_CREATE:
		srand((unsigned int)time(NULL));

		CreateGUI(hWnd, &ghWnd);
		GetClientRect(hWnd, &defaultClientRect);

		speed = SendMessage(ghWnd.track_speed, TBM_GETPOS, 0, 0);
		scale = SendMessage(ghWnd.track_size, TBM_GETPOS, 0, 0);
		_stprintf_s(str_buffer, _T("Speed %d\tScale %d"), speed, scale);
		SendMessage(ghWnd.status, SB_SETTEXT, MAKEWPARAM(1, FALSE), reinterpret_cast<LPARAM>(str_buffer));	

		playFlag = PAUSE;
		break;

	case WM_SIZE:
		deltaX = LOWORD(lParam) - defaultClientRect.right;
		deltaY = HIWORD(lParam) - defaultClientRect.bottom;
		SetWindowPos(ghWnd.board, HWND_TOP, 0, 0, (MINSIZEWIDTH_BOARD + deltaX), (MINSIZEHEIGHT_BOARD + deltaY), SWP_NOMOVE | SWP_NOZORDER);
		SetWindowPos(ghWnd.tree_view, HWND_TOP, 0, 0, MINSIZEWIDTH_TREE, (MINSIZEHEIGHT_TREE + deltaY), SWP_NOMOVE | SWP_NOZORDER);

		SendMessage(ghWnd.status, WM_SIZE, wParam, lParam);
		break;

	case WM_GETMINMAXINFO:
		((LPMINMAXINFO)lParam)->ptMinTrackSize.x = MINSIZEWIDTH_MAIN;
		((LPMINMAXINFO)lParam)->ptMinTrackSize.y = MINSIZEHEIGHT_MAIN;
		break;


	case WM_HSCROLL:
		if ((HWND)lParam == ghWnd.track_speed)
		{
			speed = SendMessage(ghWnd.track_speed, TBM_GETPOS, 0, 0);
			if (playFlag == PLAY)
			{
				KillTimer(ghWnd.board, 1);
				SetTimer(ghWnd.board, 1, 410 - 4*speed, NULL);
			}
		}
		if ((HWND)lParam == ghWnd.track_size)
		{
			scale = SendMessage(ghWnd.track_size, TBM_GETPOS, 0, 0);
		}
		_stprintf_s(str_buffer, _T("Speed %d\tScale %d"), speed, scale);
		SendMessage(ghWnd.status, SB_SETTEXT, MAKEWPARAM(1, FALSE), reinterpret_cast<LPARAM>(str_buffer));	

		SendMessage(ghWnd.board, SCALE_MESSAGE, MAKEWPARAM(FALSE, 0), LONG(scale * 10));
		break;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDRC_BUTTON_RANDOM:
			SendMessage(ghWnd.board, RANDOM_MESSAGE, MAKEWPARAM(FALSE, 0), MAKELPARAM(FALSE, 0));
			break;

		case IDRC_BUTTON_CLEAR:
			SendMessage(ghWnd.board, CLEAR_MESSAGE, MAKEWPARAM(FALSE, 0), MAKELPARAM(FALSE, 0));
			break;

		case IDRC_BUTTON_PLAY_PAUSE:
			if (playFlag == PAUSE)
			{
				SetWindowText(ghWnd.button_play_pause, _T("PAUSE"));
				playFlag = PLAY;

				EnableWindow(ghWnd.button_clear, FALSE);
				EnableWindow(ghWnd.button_random, FALSE);

				SetTimer(ghWnd.board, 1, 410 - 4*speed, NULL);
			}
			else
			{
				SetWindowText(ghWnd.button_play_pause, _T("PLAY"));
				playFlag = PAUSE;

				EnableWindow(ghWnd.button_clear, TRUE);
				EnableWindow(ghWnd.button_random, TRUE);

				KillTimer(ghWnd.board, 1);
			}
			break;

		case IDRC_MENU_EXIT:
			SendMessage(hWnd, WM_CLOSE, MAKEWPARAM(FALSE, 0), MAKELPARAM(FALSE, 0));
			break;
		}
		break;

	case WM_CLOSE:
		DestroyWindow(hWnd);
		break;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}

	return 0;
}


void CreateGUI(const HWND &hMainWnd, struct GHWND * const ghWnd)
{
	HINSTANCE hInst;
	hInst = GetModuleHandle(NULL);
	
	
	INITCOMMONCONTROLSEX icex;
	InitCommonControlsEx(&icex);


	WNDCLASSEX wcex		= {0};
	wcex.cbSize			= sizeof(WNDCLASSEX);
	wcex.lpfnWndProc	= BoardProc;
	wcex.lpszClassName	= _T("BoardChildClass");
	//wcex.style			= CS_OWNDC;					// Использовать частный контекст дисплея (используют при высокой интенсивности применение операций рисования)
	wcex.hInstance		= hInst;
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW);

	if (!RegisterClassEx(&wcex))
	{
		MessageBox(NULL, _T("Не удалось зарегистрировать класс окна"), _T("Ошибка"), MB_OK | MB_ICONERROR);
		return;
	}

	ghWnd->board =
		CreateWindowEx(
			WS_EX_CLIENTEDGE,
			_T("BoardChildClass"),
			NULL,
			WS_CHILD | WS_VISIBLE,
			220, 10, 
			MINSIZEWIDTH_BOARD,
			MINSIZEHEIGHT_BOARD,
			hMainWnd, 
			NULL,
			hInst,
			NULL
		);

	if (!ghWnd->board)
	{
		MessageBox(NULL, _T("Не удалось создать дочерние окно"), _T("Ошибка"), MB_OK | MB_ICONERROR);
		return;
	}



	ghWnd->tree_view = 
		CreateWindowEx(
			WS_EX_CLIENTEDGE,
			WC_TREEVIEW,
			_T("Tree View"),
			WS_VISIBLE | WS_CHILD | TVS_HASLINES, 
			10, 200, 
			MINSIZEWIDTH_TREE, 
			MINSIZEHEIGHT_TREE,
			hMainWnd, 
			(HMENU)IDRC_TREEVIEW, 
			hInst, 
			NULL
		);



	ghWnd->button_clear = 
		CreateWindowEx(
			NULL,
			_T("BUTTON"),
			_T("Clear"),
			WS_CHILD | BS_PUSHBUTTON | BS_TEXT | WS_VISIBLE, 
			10, 5, 
			60, 40,
			hMainWnd, 
			(HMENU)IDRC_BUTTON_CLEAR, 
			hInst, 
			NULL
		);
	SendMessage(ghWnd->button_clear, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(FALSE, 0));

	ghWnd->button_random = 
		CreateWindowEx(
			NULL,
			_T("BUTTON"),
			_T("Random"),
			WS_CHILD | BS_PUSHBUTTON | BS_TEXT | WS_VISIBLE, 
			75, 5, 
			60, 40,
			hMainWnd, 
			(HMENU)IDRC_BUTTON_RANDOM, 
			hInst, 
			NULL
		);
	SendMessage(ghWnd->button_random, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(FALSE, 0));

	ghWnd->button_play_pause = 
		CreateWindowEx(
			NULL,
			_T("BUTTON"),
			_T("Play"),
			WS_CHILD | BS_DEFPUSHBUTTON | BS_TEXT | WS_VISIBLE, 
			150, 5, 
			60, 40,
			hMainWnd, 
			(HMENU)IDRC_BUTTON_PLAY_PAUSE, 
			hInst, 
			NULL
		);
	SendMessage(ghWnd->button_play_pause, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(FALSE, 0));



	ghWnd->track_speed =
		CreateWindowEx( 
			NULL,                               
			TRACKBAR_CLASS,                  
			_T("Скорость"),              
			WS_CHILD | WS_VISIBLE | TBS_HORZ | TBS_AUTOTICKS | TBS_BOTH,            
			10, 75,                         
			200, 35,                         
			hMainWnd,                         
			(HMENU)IDRC_TRACK_SPEED,                    
			hInst,                       
			NULL                             
        ); 
	
	ghWnd->track_size =
		CreateWindowEx( 
			NULL,                               
			TRACKBAR_CLASS,                  
			_T("Маштаб"),              
			WS_CHILD | WS_VISIBLE | TBS_AUTOTICKS | TBS_LEFT | TBS_BOTH,            
			10, 140,                         
			200, 35,                         
			hMainWnd,                         
			(HMENU)IDRC_TRACK_SIZE,                    
			hInst,                       
			NULL                             
        );
	SendMessage(ghWnd->track_size, TBM_SETRANGE, (WPARAM)TRUE, (LPARAM)MAKELONG(1, 8));                       
	SendMessage(ghWnd->track_size, TBM_SETPOS, (WPARAM) TRUE, (LPARAM) 1); 


	ghWnd->label = CreateWindowEx(NULL, _T("STATIC"), _T("Slow"), WS_CHILD | WS_VISIBLE, 10, 64, 30, 15, hMainWnd, (HMENU)IDRC_LABEL, hInst, NULL); 
	SendMessage(ghWnd->label, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(FALSE, 0));
	//EnableWindow(ghWnd->label, FALSE);
	ghWnd->label = CreateWindowEx(NULL, _T("STATIC"), _T("Fast"), WS_CHILD | WS_VISIBLE, 185, 64, 30, 15, hMainWnd, (HMENU)IDRC_LABEL, hInst, NULL); 
	SendMessage(ghWnd->label, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(FALSE, 0));
	//EnableWindow(ghWnd->label, FALSE);

	ghWnd->label = CreateWindowEx(NULL, _T("STATIC"), _T("x1"), WS_CHILD | WS_VISIBLE, 20, 129, 20, 15, hMainWnd, (HMENU)IDRC_LABEL, hInst, NULL); 
	SendMessage(ghWnd->label, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(FALSE, 0));
	EnableWindow(ghWnd->label, FALSE);
	ghWnd->label = CreateWindowEx(NULL, _T("STATIC"), _T("x2"), WS_CHILD | WS_VISIBLE, 45, 129, 20, 15, hMainWnd, (HMENU)IDRC_LABEL, hInst, NULL); 
	SendMessage(ghWnd->label, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(FALSE, 0));
	EnableWindow(ghWnd->label, FALSE);
	ghWnd->label = CreateWindowEx(NULL, _T("STATIC"), _T("x3"), WS_CHILD | WS_VISIBLE, 70, 129, 20, 15, hMainWnd, (HMENU)IDRC_LABEL, hInst, NULL); 
	SendMessage(ghWnd->label, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(FALSE, 0));
	EnableWindow(ghWnd->label, FALSE);
	ghWnd->label = CreateWindowEx(NULL, _T("STATIC"), _T("x4"), WS_CHILD | WS_VISIBLE, 95, 129, 20, 15, hMainWnd, (HMENU)IDRC_LABEL, hInst, NULL); 
	SendMessage(ghWnd->label, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(FALSE, 0));
	EnableWindow(ghWnd->label, FALSE);
	ghWnd->label = CreateWindowEx(NULL, _T("STATIC"), _T("x5"), WS_CHILD | WS_VISIBLE, 119, 129, 20, 15, hMainWnd, (HMENU)IDRC_LABEL, hInst, NULL); 
	SendMessage(ghWnd->label, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(FALSE, 0));
	EnableWindow(ghWnd->label, FALSE);
	ghWnd->label = CreateWindowEx(NULL, _T("STATIC"), _T("x6"), WS_CHILD | WS_VISIBLE, 144, 129, 20, 15, hMainWnd, (HMENU)IDRC_LABEL, hInst, NULL); 
	SendMessage(ghWnd->label, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(FALSE, 0));
	EnableWindow(ghWnd->label, FALSE);
	ghWnd->label = CreateWindowEx(NULL, _T("STATIC"), _T("x7"), WS_CHILD | WS_VISIBLE, 169, 129, 20, 15, hMainWnd, (HMENU)IDRC_LABEL, hInst, NULL); 
	SendMessage(ghWnd->label, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(FALSE, 0));
	EnableWindow(ghWnd->label, FALSE);
	ghWnd->label = CreateWindowEx(NULL, _T("STATIC"), _T("x8"), WS_CHILD | WS_VISIBLE, 194, 129, 20, 15, hMainWnd, (HMENU)IDRC_LABEL, hInst, NULL); 
	SendMessage(ghWnd->label, WM_SETFONT, (WPARAM)GetStockObject(DEFAULT_GUI_FONT), MAKELPARAM(FALSE, 0));
	EnableWindow(ghWnd->label, FALSE);


	int statusWidhs[3] = { 215, 555, -1 };
	ghWnd->status =
		CreateStatusWindow(
			WS_CHILD | WS_VISIBLE,
			_T(""),
			hMainWnd,
			IDRC_MAIN_STATUS
		);
	SendMessage(ghWnd->status, SB_SETPARTS, 3, (LPARAM)statusWidhs);
}



LRESULT CALLBACK BoardProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	HDC				hdc;
	PAINTSTRUCT		ps;

	HDC				mhdc;
    HBITMAP			mBitMap;
	HANDLE			hOldHandle;


	static RECT		sizeBoard;
	static int		scale;
	const int		minScale = 10;

	static BOOL		**eMatrixData = NULL;
	BOOL			**tempMatrixData = NULL;

	static int		xMatrixSize,
					yMatrixSize;

	POINT			point;
	RECT			rect;

	
	static HPEN		hOldPen,
					hPenGrid;

	static HBRUSH	hOldBrush,
					hBrushBG,
					hBrushElements;


	int count = 0;

	int iTop = 0,
		iBottom = 0;
	int jLeft = 0,
		jRight = 0;



	switch(message)
	{
	case WM_CREATE:
		hPenGrid = CreatePen(PS_SOLID, 1, RGB(255, 255, 255));

		hBrushBG = CreateSolidBrush(RGB(234, 234, 234));
		hBrushElements = CreateSolidBrush(RGB(4, 153, 197));

		scale = minScale;

		xMatrixSize = yMatrixSize = 1;
		eMatrixData = CreateMatrix(xMatrixSize, yMatrixSize);
		break;

	case WM_SIZE:
		GetClientRect(hWnd, &sizeBoard);
		
		tempMatrixData = CreateMatrix(sizeBoard.right/minScale, sizeBoard.bottom/minScale);
		for (int i = 0; i < sizeBoard.right/minScale; i++)
		{
			for (int j = 0; j < sizeBoard.bottom/minScale; j++)
			{
				if ((i < xMatrixSize) && (j < yMatrixSize))
				{
					if (eMatrixData[i][j] == TRUE)
					{
						tempMatrixData[i][j] = eMatrixData[i][j];
					}
				}
			}
		}

		DeleteMatrix(eMatrixData, xMatrixSize);
		
		eMatrixData = tempMatrixData;
		tempMatrixData = NULL;

		xMatrixSize = sizeBoard.right / minScale;
		yMatrixSize = sizeBoard.bottom / minScale;
		break;

	case WM_ERASEBKGND:
		break; 


	case SCALE_MESSAGE:
		scale = lParam;
		InvalidateRect(hWnd, NULL, FALSE);
		break;

	case RANDOM_MESSAGE:
		for (int i = 0; i < xMatrixSize; i++)
		{
			for (int j = 0; j < yMatrixSize; j++)
			{
				if (rand() % 3)
					eMatrixData[i][j] = FALSE;
				else
					eMatrixData[i][j] = TRUE;	
			}
		}
		InvalidateRect(hWnd, NULL, FALSE);
		break;

	case CLEAR_MESSAGE:
		DeleteMatrix(eMatrixData, xMatrixSize);
		eMatrixData = CreateMatrix(sizeBoard.right/minScale, sizeBoard.bottom/minScale);
		InvalidateRect(hWnd, NULL, FALSE);
		break;



	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		{
			mhdc = CreateCompatibleDC(hdc);
			mBitMap = CreateCompatibleBitmap(hdc, sizeBoard.right, sizeBoard.bottom);
			hOldHandle = SelectObject(mhdc, mBitMap);
			PatBlt(mhdc, 0, 0, sizeBoard.right, sizeBoard.bottom, WHITENESS);
			{
				SetBkMode(mhdc, TRANSPARENT);
				FillRect(mhdc, &sizeBoard, hBrushBG);

				hOldPen = (HPEN)SelectObject(mhdc, hPenGrid);
				for(int i = 0; i < sizeBoard.right; i+=scale)
				{
					MoveToEx(mhdc, i, sizeBoard.top, NULL);
					LineTo(mhdc, i, sizeBoard.bottom);
				}
			
				for (int i = 0; i < sizeBoard.bottom; i+=scale)
				{
					MoveToEx(mhdc, sizeBoard.top, i, NULL);
					LineTo(mhdc, sizeBoard.right, i);
				}

				hOldBrush = (HBRUSH)SelectObject(mhdc, hBrushElements);
		
				for (int i = 0; i < xMatrixSize; i++)
				{
					for (int j = 0; j < yMatrixSize; j++)
					{
						if (eMatrixData[i][j] == TRUE)
						{		
							rect.left = i * scale;
							rect.top = j * scale;
							rect.right = rect.left + scale;
							rect.bottom = rect.top + scale;

							FillRect(mhdc, &rect, hBrushElements);
						}
					}
				}

				SelectObject(mhdc, hOldPen);
				SelectObject(mhdc, hOldBrush);
			}
			BitBlt(hdc, 0, 0, sizeBoard.right, sizeBoard.bottom, mhdc, 0, 0, SRCCOPY);    

			SelectObject(mhdc, hOldHandle);
			DeleteObject(mBitMap);
			DeleteDC(mhdc);
		}
		EndPaint(hWnd, &ps);
		break;

	case WM_LBUTTONDOWN:
		int c;
		point.x = LOWORD(lParam);
		point.y = HIWORD(lParam);

		c = 0;
		while (point.x > scale)
		{
			c++;
			point.x -= scale;
		}
		point.x = c;

		c = 0;
		while (point.y > scale)
		{
			c++;
			point.y -= scale;
		}
		point.y = c;

		if ((point.x < xMatrixSize) && (point.y < yMatrixSize))
		{
			eMatrixData[point.x][point.y] = !eMatrixData[point.x][point.y];
		}

		rect.left = point.x * scale;
		rect.top = point.y * scale;
		rect.right = rect.left + scale;
		rect.bottom = rect.top + scale;
		InvalidateRect(hWnd, &rect, FALSE);
		break;

	case WM_TIMER:
		tempMatrixData = CreateMatrix(xMatrixSize, yMatrixSize);

		for (int i = 0; i < xMatrixSize; i++)
		{
			if (i == 0)
			{
				iTop = xMatrixSize - 1;
				iBottom = i+1;
			}
			else
			{
				if (i == (xMatrixSize - 1))
				{
					iTop = i-1;
					iBottom = 0;
				}
				else
				{
					iTop = i+1;
					iBottom = i-1;
				}
			}
			
			for (int j = 0; j < yMatrixSize; j++)
			{
				if (j == 0)
				{
					jLeft = yMatrixSize - 1;
					jRight = j+1;
				}
				else
				{
					if (j == (yMatrixSize - 1))
					{
						jLeft = j-1;
						jRight = 0;
					}
					else
					{
						jLeft = j-1;
						jRight = j+1;
					}
				}

				count = 0;
				if (eMatrixData[iTop][jLeft] == TRUE) 
					count++;
				if (eMatrixData[iTop][j] == TRUE) 
					count++;
				if (eMatrixData[iTop][jRight] == TRUE) 
					count++;
				if (eMatrixData[i][jLeft] == TRUE) 
					count++;
				if (eMatrixData[i][jRight] == TRUE) 
					count++;
				if (eMatrixData[iBottom][jLeft] == TRUE) 
					count++;
				if (eMatrixData[iBottom][j] == TRUE) 
					count++;
				if (eMatrixData[iBottom][jRight] == TRUE) 
					count++;


				if (eMatrixData[i][j] == TRUE)
				{
					if (count > 1 && count < 4)
						tempMatrixData[i][j] = TRUE;
					else
						tempMatrixData[i][j] = FALSE;
				}
				else
				{
					if (count == 3)
						tempMatrixData[i][j] = TRUE;
					else
						tempMatrixData[i][j] = FALSE;
				}
			}
		}

		DeleteMatrix(eMatrixData, xMatrixSize);
		eMatrixData = tempMatrixData;
		tempMatrixData = NULL;

		InvalidateRect(hWnd, NULL, FALSE);
		break;

	case WM_DESTROY:
		DeleteObject(hPenGrid);

		DeleteObject(hBrushBG);
		DeleteObject(hBrushElements);

		DeleteMatrix(eMatrixData, xMatrixSize);
		break;

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}


BOOL ** CreateMatrix(size_t iSize, size_t jSize)
{
	BOOL **data;

	data = new BOOL * [iSize];
	for (size_t i = 0; i < iSize; i++)
	{
			data[i] = new BOOL [jSize];

			for (size_t j = 0; j < jSize; j++)
				data[i][j] = FALSE;
	}
	return data;
}

void DeleteMatrix(BOOL **data, size_t iSize)
{
	for (size_t i = 0; i < iSize; i++)
		delete [] data[i];
	delete [] data;
}