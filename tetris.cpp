#include <windows.h>
#include <time.h>
#include <stdlib.h>

#define W 14						  //游戏区域宽度
#define H 20						  //游戏区域高度
#define W1 6						  //右边状态栏宽度
#define BSIZE 25					  //游戏方格边长
#define Y1 6						  //放置照片底端的纵坐标
#define Y2 12						  //分数显示栏顶端纵坐标
#define Y3 15						  //等级显示栏顶端纵坐标
#define Y4 7						  //帮助栏顶端纵坐标
#define Cur_x W / 2 - 1				  //游戏方块初始状态左上角横坐标
#define Cur_y 1						  //初始状态左上角纵坐标
#define BgColor RGB(0xF5, 0xF5, 0xDC) //米色
#define FgColor RGB(255, 153, 204)	  //粉红
#define RED RGB(255, 0, 0)
#define ORANGE RGB(255, 97, 0)
#define YELLOW RGB(255, 153, 18)
#define GREEN RGB(34, 139, 34)
#define CYAN RGB(128, 42, 42)
#define LIGHT_BLUE RGB(65, 105, 225) //天蓝色
#define PURPLE RGB(255, 0, 255)
#define MS_NEWBLOCK WM_USER + 1 // 消息ID，产生新的【方块】

int score = 0, level = 0, level_step = 100; //分数等级以及每等级所需分数的定义及初始化
int top = H - 1;							//最顶端的纵坐标
int x, y;									//方块当前位置的横坐标及纵坐标
int cur_boxnum, next_boxnum;				//cur_boxnum是当前方块编号，next_boxnum是下一个方块编号

struct BOARD
{
	int var;   //状态，1代表已被占用，0代表未被占用
	int color; //颜色
} board[H][W]; //定义游戏主板，H行N列

struct BLOCK
{
	int a[4][2]; //定义方块形状的数组，每行为方块左上角坐标
	int color;	 //方块颜色
	int next;	 //下一个方块的号码
};

struct BLOCK block[19] =
	{
		//初始化各个游戏方块
		{1, 1, 1, 2, 1, 3, 2, 3, RED, 1},
		{0, 2, 1, 2, 2, 2, 0, 3, RED, 2},
		{0, 1, 1, 1, 1, 2, 1, 3, RED, 3},
		{2, 1, 0, 2, 1, 2, 2, 2, RED, 0},
		{1, 1, 1, 2, 0, 3, 1, 3, ORANGE, 5},
		{0, 1, 0, 2, 1, 2, 2, 2, ORANGE, 6},
		{1, 1, 2, 1, 1, 2, 1, 3, ORANGE, 7},
		{0, 2, 1, 2, 2, 2, 2, 3, ORANGE, 4},
		{1, 1, 0, 2, 1, 2, 2, 2, YELLOW, 9},
		{1, 1, 1, 2, 2, 2, 1, 3, YELLOW, 10},
		{0, 2, 1, 2, 2, 2, 1, 3, YELLOW, 11},
		{1, 1, 0, 2, 1, 2, 1, 3, YELLOW, 8},
		{1, 1, 1, 2, 2, 2, 2, 3, GREEN, 13},
		{1, 2, 2, 2, 0, 3, 1, 3, GREEN, 12},
		{2, 1, 1, 2, 2, 2, 1, 3, CYAN, 15},
		{0, 2, 1, 2, 1, 3, 2, 3, CYAN, 14},
		{1, 0, 1, 1, 1, 2, 1, 3, LIGHT_BLUE, 17},
		{0, 2, 1, 2, 2, 2, 3, 2, LIGHT_BLUE, 16},
		{1, 1, 2, 1, 1, 2, 2, 2, PURPLE, 18},
};

void Paint(HDC hdc, HPEN hpen) //此函数用于初始化界面
{
	int i, j;
	HPEN hpen1;								   //定义画笔，用于绘制分隔线
	HBRUSH hbrush = CreateSolidBrush(BgColor); //定义画刷并赋初值，画刷颜色采用背景色

	hpen1 = CreatePen(PS_DASHDOTDOT, 3, FgColor); //给画笔赋初值，颜色为前景色，线宽为3，双点划线
	SelectObject(hdc, hpen1);					  //选择画笔
	MoveToEx(hdc, W * BSIZE, 0, NULL);			  //将光标移动到(W*BSIZE,0)处
	LineTo(hdc, W * BSIZE, H * BSIZE);			  //从光标所在位置画线到(W*BSIZE,H*BSIZE)处
	DeleteObject(hpen1);						  //删除之前所选用的画笔
	SelectObject(hdc, hpen);					  //重新选择画笔
	SelectObject(hdc, hbrush);					  //选择画刷

	for (i = 1; i < H - 1; i++) //绘制游戏区域方格线
		for (j = 1; j < W - 1; j++)
			Rectangle(hdc, j * BSIZE, i * BSIZE, (j + 1) * BSIZE, (i + 1) * BSIZE);

	for (i = 1; i < 5; i++) //绘制右边状态栏游戏预览区域方格线
		for (j = W + 1; j < W + W1 - 1; j++)
			Rectangle(hdc, j * BSIZE, i * BSIZE, (j + 1) * BSIZE, (i + 1) * BSIZE);

	Rectangle(hdc, (W + 1) * BSIZE, Y2 * BSIZE, (W + W1 - 1) * BSIZE, (Y2 + 2) * BSIZE); //绘制分数栏方格线
	Rectangle(hdc, (W + 1) * BSIZE, Y3 * BSIZE, (W + W1 - 1) * BSIZE, (Y3 + 2) * BSIZE); //绘制等级栏方格线
	Rectangle(hdc, (W + 1) * BSIZE, Y4 * BSIZE, (W + W1 - 1) * BSIZE, (Y4 + 4) * BSIZE); //绘制帮助栏方格线

	TextOut(hdc, (W + 2) * BSIZE, (Y2 + 0.2) * BSIZE, TEXT("分     数:"), 8); //输出文字
	TextOut(hdc, (W + 2) * BSIZE, (Y3 + 0.2) * BSIZE, TEXT("难     度:"), 8); //同上

	DeleteObject(hpen);	  //删除画笔
	DeleteObject(hbrush); //删除画刷
}

void ShowScore(HDC hdc) //显示分数的函数
{
	TCHAR score_str[4];												   //定义字符串用于保存分数值
	wsprintf(score_str, TEXT("%3d"), score);						   //将数字score转换成字符串后保存到score_str之中
	TextOut(hdc, (W + 2.5) * BSIZE, (Y2 + 1.2) * BSIZE, score_str, 3); //在游戏板上显示分数
}

void ShowLevel(HDC hdc) //显示等级的，具体同上
{
	TCHAR level_str[4];
	wsprintf(level_str, TEXT("%3d"), level);
	TextOut(hdc, (W + 2.5) * BSIZE, (Y3 + 1.2) * BSIZE, level_str, 3);
}

void ShowHelp(HDC hdc) //显示帮助的，该函数只在初始化界面时调用
{
	TCHAR help1[] = TEXT("↑ - 旋 转!!"),
		  help2[] = TEXT("↓ - 下 降!!"),
		  help3[] = TEXT("← - 左 移!!"),
		  help4[] = TEXT("→ - 右 移!!");
	TextOut(hdc, (W + 1.8) * BSIZE, (Y4 + 0.2) * BSIZE, help1, 9);
	TextOut(hdc, (W + 1.8) * BSIZE, (Y4 + 1.2) * BSIZE, help2, 9);
	TextOut(hdc, (W + 1.8) * BSIZE, (Y4 + 2.2) * BSIZE, help3, 9);
	TextOut(hdc, (W + 1.8) * BSIZE, (Y4 + 3.2) * BSIZE, help4, 9);
}

void EraseBox(HDC hdc, HPEN hpen, int x, int y, int num) //清除(x,y)处编号为num，的方块
{
	int i;
	HBRUSH hbrush = CreateSolidBrush(BgColor);
	SelectObject(hdc, hpen);
	SelectObject(hdc, hbrush);

	for (i = 0; i < 4; i++) //用背景色填充方块所在区域，使方块隐藏
		Rectangle(hdc,
				  (x + block[num].a[i][0]) * BSIZE,
				  (y + block[num].a[i][1]) * BSIZE,
				  (x + block[num].a[i][0] + 1) * BSIZE,
				  (y + block[num].a[i][1] + 1) * BSIZE);

	DeleteObject(hpen);
	DeleteObject(hbrush);
}

void ShowBox(HDC hdc, HPEN hpen, int x, int y, int num) //显示(x,y)处编号为num，的方块
{
	int i;
	HBRUSH hbrush;

	hbrush = CreateSolidBrush(block[num].color); //创建画刷，颜色和方块颜色相同
	SelectObject(hdc, hpen);
	SelectObject(hdc, hbrush);

	for (i = 0; i < 4; i++) //显示方块的过程
		Rectangle(hdc,
				  (x + block[num].a[i][0]) * BSIZE,
				  (y + block[num].a[i][1]) * BSIZE,
				  (x + block[num].a[i][0] + 1) * BSIZE,
				  (y + block[num].a[i][1] + 1) * BSIZE);

	DeleteObject(hpen);
	DeleteObject(hbrush);
}

void SetFullRow(HDC hdc, HPEN hpen) //满行处理函数
{
	int i, ii, j;
	int org_top = top;
	int flag = 0;
	HBRUSH hbrush;
	SelectObject(hdc, hpen);
	for (i = y; i < y + 4; i++) //从y行开始，从上到下遍历游戏区域
	{
		if (i <= 0 || i >= H - 1)
			continue; //越界了，就跳出本次循环

		for (j = 1; j < W - 1; j++)
			if (!board[i][j].var)
				break; //一旦该行有一个为空，即跳出

		if (j == W - 1) //找到满行了
		{
			for (ii = i; ii >= top; ii--) //重置游戏区域各个方格的状态，top为最顶端，i为找到的满行
				for (j = 1; j < W - 1; j++)
					board[ii][j] = board[ii - 1][j];

			top++;
			score += 10; //分数加10
			flag = 1;	 //标志符
		}
	}

	if (flag) //如果有满行，则重绘主板
	{
		for (i = org_top; i < y + 4; i++) //原来的最顶端
		{
			if (i <= 0 || i >= H - 1)
				continue; //越界了，就跳出本次循环
			for (j = 1; j < W - 1; j++)
			{ //注意这里绘制主板时，每次都要选择不同的画刷，用完后一定要删除
				hbrush = CreateSolidBrush(board[i][j].color);
				SelectObject(hdc, hbrush);
				Rectangle(hdc, j * BSIZE, i * BSIZE, (j + 1) * BSIZE, (i + 1) * BSIZE);
				DeleteObject(hbrush);
			}
		}
		if (level != score / level_step) //这里是程序优化部分，也可省略
			level = score / level_step;
		ShowScore(hdc); //更新分数
		ShowLevel(hdc); //更新等级
	}
	DeleteObject(hpen);
}

void ChangeVar(void) //改变游戏主板的状态
{
	int i;
	for (i = 0; i < 4; i++)
	{
		board[y + block[cur_boxnum].a[i][1]][x + block[cur_boxnum].a[i][0]].var = 1; //状态置1,表示有方块填充
		board[y + block[cur_boxnum].a[i][1]][x + block[cur_boxnum].a[i][0]].color = block[cur_boxnum].color;
	}
}

BOOL CanMove(void) //判断方块是否能移动
{
	int i;
	for (i = 0; i < 4; i++)
		if (board[y + block[cur_boxnum].a[i][1]][x + block[cur_boxnum].a[i][0]].var) //如果该位置以及有方块填充，则不能移动
			return FALSE;
	return TRUE;
}

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);										   //窗口过程函数的声明
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hprevInstance, PSTR szCmdLine, int iCmdShow) //入口函数，即主函数
{																							   //各个形参所代表的意思请自行查阅资料
	int screenwide, screenhight;															   //定义变量来保存屏幕宽度和高度
	TCHAR AppName[] = TEXT("Tetris");														   //定义并初始化窗口类名

	HWND hwnd;			 //定义窗口句柄
	MSG msg;			 //定义消息结构体
	WNDCLASSEX wndclass; //定义窗口类

	wndclass.cbSize = sizeof(wndclass);					//窗口类大小
	wndclass.style = CS_HREDRAW | CS_VREDRAW;			//窗口类风格
	wndclass.lpfnWndProc = WndProc;						//窗口过程函数为WndProc
	wndclass.cbClsExtra = 0;							//窗口类无扩展
	wndclass.cbWndExtra = 0;							//窗口实例无扩展
	wndclass.hInstance = hInstance;						//当前实例句柄为hInstance
	wndclass.hIcon = LoadIcon(NULL, IDI_APPLICATION);	//采用默认图标
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);		//默认光标，光标进入窗口区域时，将显示为箭头
	wndclass.hbrBackground = CreateSolidBrush(BgColor); //窗口背景色
	wndclass.lpszMenuName = NULL;						//窗口类无菜单
	wndclass.lpszClassName = AppName;					//窗口类名
	wndclass.hIconSm = LoadIcon(NULL, IDI_APPLICATION); //采用默认小图标

	if (!RegisterClassEx(&wndclass)) //窗口类的注册
	{
		MessageBeep(0);
		return FALSE;
	}

	screenwide = GetSystemMetrics(SM_CXFULLSCREEN);	 //获取屏幕宽度，即横向分辨率
	screenhight = GetSystemMetrics(SM_CYFULLSCREEN); //获取屏幕高度

	hwnd = CreateWindow(				  //创建窗口
		AppName,						  //窗口类名
		TEXT("Tetris @DylanC(Cll66.CN)"), //窗口实例标题名
		WS_OVERLAPPEDWINDOW,
		0,				  //窗口左上角横坐标
		0,				  //左上角纵坐标
		(W + W1) * BSIZE, //窗口宽度
		(H + 1) * BSIZE,  //窗口高度，注意包含了标题栏
		NULL,			  //窗口无父窗口
		NULL,			  //窗口无住菜单
		hInstance,		  //当前应用程序实例句柄
		NULL			  //这个值一般置NULL，具体请自行查询
	);

	if (!hwnd)
		return FALSE; //创建失败则退出程序

	ShowWindow(hwnd, iCmdShow);																					 //显示窗口
	UpdateWindow(hwnd);																							 //刷新窗口
	MessageBox(hwnd, TEXT(" 准备好来一场惊险刺激的俄罗斯方块了吗？"), TEXT("Tetris @DylanC (Cll66.CN)"), MB_OK); //弹出框
	SetTimer(hwnd, 1, 800, NULL);																				 //设置一个500毫秒触发一次的定时器，具体参数所代表意义请自行查询

	while (GetMessage(&msg, NULL, 0, 0)) //消息循环，用于从消息队列中获取消息
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	return msg.wParam; //注意这一步不能少
}

LRESULT CALLBACK WndProc(HWND hwnd, UINT iMsg, WPARAM wParam, LPARAM lParam)
{ //定义窗口过程函数
	int i, j;
	int old_boxnum;								 //用于保存之前的方块号
	HDC hdc;									 //定义DC句柄，DC里包含绘图默认的一些属性
	PAINTSTRUCT ps;								 //定义绘图结构，此结构包含的信息应用程序使用来绘制一个拥有该应用窗口客户区
	HPEN hpen = CreatePen(PS_SOLID, 1, FgColor); //选择画刷，用背景色作画刷
	switch (iMsg)								 //消息处理的过程
	{
	case WM_CREATE: ////当一个应用程序使用函数CreateWindow或CreateWindowEx来创建一个窗口时，
		//系统将发送该消息给此新建窗口过程。该消息将在创建窗口之后，显示窗口
		//之前发送该消息，该消息将在CreateWindow或CreateWindowEx函数返回之前发送。
		for (i = 1; i < H; i++) //初始化状态，讲两竖边状态初始化为1
			board[i][0].var = board[i][W - 1].var = 1;
		for (j = 1; j < W - 1; j++) //将最底部状态置1
			board[H - 1][j].var = 1;
		for (i = 1; i < H - 1; i++) //将游戏区域状态置0
			for (j = 1; j < W - 1; j++)
			{
				board[i][j].var = 0;
				board[i][j].color = BgColor;
			}
		srand((unsigned)time(NULL));			   //初始化随机数发生器
		cur_boxnum = rand() % 19;				   //赋初值
		next_boxnum = rand() % 19;				   //赋初值
		x = Cur_x;								   //初始化方块横坐标
		y = Cur_y;								   //初始化方块纵坐标
		return 0;								   //直接退出窗口过程函数
	case WM_PAINT:								   //绘制界面，当应用程序适用UpdateWindow刷新窗口时，第一次发送该消息
		hdc = BeginPaint(hwnd, &ps);			   //给DC句柄赋初值
		Paint(hdc, hpen);						   //调用Paint函数绘制界面
		ShowScore(hdc);							   //显示分数
		ShowLevel(hdc);							   //显示等级
		ShowHelp(hdc);							   //显示帮助栏
		ShowBox(hdc, hpen, x, y, cur_boxnum);	   //显示游戏区域中的游戏方块
		ShowBox(hdc, hpen, W + 1, 1, next_boxnum); //显示右边状态栏的游戏方块
		EndPaint(hwnd, &ps);					   //EndPaint函数标记指定窗口的绘画过程结束；这个函数在每次调用BeginPaint函数之后被请求，但仅仅在绘画完成以后。
		return 0;
	case WM_TIMER:		   //定时器消息，每0.5秒接受到一次
		hdc = GetDC(hwnd); //该函数检索一指定窗口的客户区域或整个屏幕的显示设备上下文环境的句柄，以后可以在GDI函数中使用该句柄来在设备上下文环境中绘图。
		y++;			   //y增1
		if (CanMove())	   //如果能移动，则擦除原来位置方块，显示新位置方块，相当于是方块下落
		{
			EraseBox(hdc, hpen, x, y - 1, cur_boxnum);
			ShowBox(hdc, hpen, x, y, cur_boxnum);
		}
		else //如果不能移动，则到底了，y恢复之前的值，并发送MS_NEWBLOCK产生新的方块
		{
			y--;
			SendMessage(hwnd, MS_NEWBLOCK, 0, 0);
		}

		ReleaseDC(hwnd, hdc); //数释放设备上下文环境（DC）供其他应用程序使用
		return 0;
	case WM_KEYDOWN: //当按下键时，会发送该消息
		hdc = GetDC(hwnd);
		switch ((int)wParam) //判断具体按下的键
		{
		case VK_UP:
			old_boxnum = cur_boxnum;			 //保存当前方块号
			cur_boxnum = block[cur_boxnum].next; //方块号变为下一个方块号
			if (CanMove())
			{
				EraseBox(hdc, hpen, x, y, old_boxnum);
				ShowBox(hdc, hpen, x, y, cur_boxnum);
			}
			else
				cur_boxnum = old_boxnum; //恢复之前的值
			break;
		case VK_DOWN:
			y++;
			if (CanMove())
			{
				EraseBox(hdc, hpen, x, y - 1, cur_boxnum);
				ShowBox(hdc, hpen, x, y, cur_boxnum);
			}
			else
			{
				y--;
				SendMessage(hwnd, MS_NEWBLOCK, 0, 0); //不能下移，就到底了，产生新的方块
			}
			break;
		case VK_LEFT:
			x--;		   //横坐标减小1
			if (CanMove()) //如果能移动，则下移，不能移动则恢复之前坐标
			{
				EraseBox(hdc, hpen, x + 1, y, cur_boxnum);
				ShowBox(hdc, hpen, x, y, cur_boxnum);
			}
			else
				x++;
			break;
		case VK_RIGHT: //同上
			x++;
			if (CanMove())
			{
				EraseBox(hdc, hpen, x - 1, y, cur_boxnum);
				ShowBox(hdc, hpen, x, y, cur_boxnum);
			}
			else
				x--;
			break;
		}

		ReleaseDC(hwnd, hdc);
		return 0;
	case MS_NEWBLOCK:
		hdc = GetDC(hwnd);
		if (top > y + block[cur_boxnum].a[0][1])
			top = y + block[cur_boxnum].a[0][1]; //确定最高点
		ChangeVar();							 //改变游戏主板状态
		SetFullRow(hdc, hpen);					 //满行处理
		cur_boxnum = next_boxnum;
		x = Cur_x; //重置方块坐标
		y = Cur_y;
		hpen = CreatePen(PS_SOLID, 1, FgColor);
		srand((unsigned)time(NULL)); //初始化随机数发生器
		next_boxnum = rand() % 19;
		EraseBox(hdc, hpen, W + 1, 1, cur_boxnum);	  //清除右边状态栏的方块
		ShowBox(hdc, hpen, W + 1, 1, next_boxnum);	  //显示右边状态栏的方块
		ShowBox(hdc, hpen, Cur_x, Cur_y, cur_boxnum); //显示游戏主板顶部方块

		if (!CanMove()) //刚移开始就不能移动，则结束程序退出
		{
			KillTimer(hwnd, 1);
			MessageBox(hwnd, TEXT("  退出游戏"), TEXT("退出"), MB_OK);
			PostQuitMessage(0);
		}

		ReleaseDC(hwnd, hdc);
		return 0;
	case WM_DESTROY: //退出游戏
		KillTimer(hwnd, 1);
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hwnd, iMsg, wParam, lParam); //窗口默认处理，当消息处理函数未处理消息时就调用该函数进行处理
}
