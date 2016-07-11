#define _CRT_SECURE_NO_WARNINGS
#include <afxinet.h> 
#include <iostream>
#include <locale>  
#include <codecvt>  
#include <ctime>
#include <string>
#include <cstring>
#include <cstdlib>
#include <sstream>
#include <fstream>
#include <sqlite3.h>
#include <windows.h>
#include <winsock.h>
#include "json.h"
#include "core.h"
#pragma comment(lib,"sqlite3.lib")
#pragma comment(lib,"json_vc71_libmtd.lib")
using namespace std;





CString urlcur = "http://www.weather.com.cn/data/sk/101190101.html";
CString urlw = "http://www.weather.com.cn/data/cityinfo/101190101.html ";





#define  WINDOW_WIDTH  1366 //窗口宽度宏
#define  WINDOW_HEIGHT  768 //窗口高度宏
#define  WINDOW_TITLE L"HHU" //窗口标题宏

HDC g_hdc = NULL,g_mdc = NULL;
HPEN g_hPen = NULL;

HBITMAP g_hbitmap = NULL;
HBITMAP g_point = NULL;



int flag = 0;//系统状态

int flags = 0;//是否重绘过窗口



int sqlite3_exec_callback(void *data, int n_columns, char **col_values, char **col_names)
{
	City_code = col_values[0];
	return 0;
}

void ConvertUTF8ToANSI()  //运用c++11来进行UTF8和ANSI的类型转化
{     
	auto LocUtf8 = std::locale(std::locale(""), new std::codecvt_utf8<wchar_t>);  
	std::wifstream wfin("temp.json");
	std::wstring wstr, content;
	wfin.imbue(LocUtf8);
	while(getline(wfin, wstr))
	{
		content += wstr; 
	}
	wfin.close();  
	//system("del temp.json");

	std::locale::global(std::locale("Chinese-simplified"));
	std::wofstream wfout("weather.json");
	wfout << content;
	wfout.close();
}  

void jsonDate(string strValue,int mode) 
{

	Json::Value value;
	Json::Reader reader;
	reader.parse(strValue, value);
	if(mode == 2)weather = value["weatherinfo"]["weather"].asString();
	else temp = value["weatherinfo"]["temp"].asString();

}





void GetWeather(CString url)
{
	CFile file((TEXT("temp.json")), CFile::modeCreate|CFile::modeWrite);

	CString content,content1;
	CString data,data1;
	DWORD dwStatusCode;
	CInternetSession session(_T("HttpClient"));
	CHttpFile* pfile = (CHttpFile *)session.OpenURL(url);

	pfile -> QueryInfoStatusCode(dwStatusCode);

	if(dwStatusCode == HTTP_STATUS_OK)
	{ 
		while (pfile -> ReadString(data) != false || data.GetLength() != 0)
		{
			content  += data + "\r\n";
		}

	}
	else MessageBox(NULL,L"天气信息获取失败！",L"error",MB_OK);

	pfile -> Close();
	delete pfile;
	session.Close();
	//content.TrimRight(); 
	//printf(" %s\n " ,(LPCTSTR)content); 
	file.Write(content, content.GetLength()*2);  
	file.Close();
}


void ShowWeather(CString url,int n)
{
	fstream fin("weather.json");
	string strValue, str;
	sqlite3 *db = NULL;
	char *err_msg = NULL;
	if(SQLITE_OK != sqlite3_open("weather.db",&db))
	{
		MessageBox(NULL,L"can't open the database.",L"error",MB_OK);
	}



	std::string sql = "select city_code from weather where city = ";
	sql.append("\"");
	sql.append("\"");
	sqlite3_exec(db, sql.c_str(), &sqlite3_exec_callback, 0, &err_msg);


	if(n == 1)GetWeather(urlcur);
	else GetWeather(urlw);
	ConvertUTF8ToANSI();
	while (getline(fin, str))
	{
		strValue += str;
		str.clear();
	}
	jsonDate(strValue,n);
	City_code.clear();
	sqlite3_close(db);
	strValue.clear();
	fin.close();
}

void getWeatherInfo()
{
	ShowWeather(urlw,1);
	ShowWeather(urlcur,2);
}


LPCWSTR stringToLPCWSTR(std::string orig)
{
	size_t origsize = orig.length() + 1;
	const size_t newsize = 100;
	size_t convertedChars = 0;
	wchar_t *wcstring = (wchar_t *)malloc(sizeof(wchar_t)*(orig.length()-1));
	mbstowcs_s(&convertedChars, wcstring, origsize, orig.c_str(), _TRUNCATE);
	return wcstring;
}



//窗口过程函数声明
LRESULT CALLBACK WndProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam);


BOOL Game_Init(HWND hwnd);
VOID Game_Paint(HWND hwnd);
BOOL Game_CleanUp(HWND hwnd);

//windows程序入口函数
int WINAPI WinMain( __in HINSTANCE hInstance, __in_opt HINSTANCE hPrevInstance, __in LPSTR lpCmdLine, __in int nShowCmd )
{
	//初始化地图原始数据
	init();

	//设计一个完整的窗口类
	WNDCLASSEX wndClass = {0};//定义一个窗口类并初始化
	wndClass.cbSize = sizeof(WNDCLASSEX);
	wndClass.style = CS_HREDRAW | CS_VREDRAW;
	wndClass.lpfnWndProc = WndProc;//窗口过程函数
	wndClass.cbClsExtra = 0;
	wndClass.cbWndExtra = 0;
	wndClass.hInstance = hInstance;
	wndClass.hIcon = (HICON)::LoadImage(NULL,L"icon.ico",IMAGE_ICON,0,0,LR_DEFAULTSIZE|LR_LOADFROMFILE);
	wndClass.hCursor = LoadCursor(NULL,IDC_ARROW);
	wndClass.hbrBackground = (HBRUSH)GetStockObject(LTGRAY_BRUSH);
	wndClass.lpszMenuName = NULL;
	wndClass.lpszClassName = L"HHU";

	//注册窗口
	if(!RegisterClassEx(&wndClass))
	{
		MessageBox(NULL,L"窗口注册失败",L"ERROR",0);
		return -1;
	}

	//创建窗口
	HWND hwnd = CreateWindow(L"HHU",WINDOW_TITLE,WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,CW_USEDEFAULT,WINDOW_WIDTH,WINDOW_HEIGHT,
		NULL,NULL,hInstance,NULL);

	//窗口的移动，显示与更新
	MoveWindow(hwnd,250,80,WINDOW_WIDTH,WINDOW_HEIGHT,true);
	ShowWindow(hwnd,nShowCmd);
	UpdateWindow(hwnd);

	//游戏初始化失败，返回停止
	if(!Game_Init(hwnd))
	{
		MessageBox(hwnd,L"资源初始化失败！",L"消息窗口",0);
		return false;
	}
	//PlaySound(L"AIR - 夏影.wav",NULL,SND_FILENAME | SND_ASYNC|SND_LOOP);

	//消息循环过程

	MSG msg = {0};
	while(msg.message != WM_QUIT)
	{
		if(PeekMessage(&msg,0,0,0,PM_REMOVE))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else 
		{
			if(flag > 0 && !flags)
			{
				Game_Init(hwnd);
				Game_Paint(hwnd);
				flags = 1;
			}
		}
	}

	//窗口注销
	UnregisterClass(L"HHU",wndClass.hInstance);

	return 0;
}

//消息处理函数
LRESULT CALLBACK WndProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam)
{
	PAINTSTRUCT paintStuct;//定义一个PAINTSTRUCT结构体来记录绘制信息


	switch(message)
	{
	case WM_PAINT ://重绘显示
		g_hdc = BeginPaint(hwnd,&paintStuct);
		Game_Paint(hwnd);
		EndPaint(hwnd,&paintStuct);
		ValidateRect(hwnd,NULL);
		break;

	case WM_LBUTTONDOWN:
		if(wParam & MK_LBUTTON)
		{
			if((wParam & MK_CONTROL)&& !flag)
			{
				g_ixs = LOWORD(lParam);
				g_iys = HIWORD(lParam);
				flag = 1;
				flags = 0;
			}
			else if((wParam & MK_CONTROL)&& flag == 1)
			{
				g_ixe = LOWORD(lParam);
				g_iye = HIWORD(lParam);
				flag = 2;
				flags = 0;
			}
		}
		break;

		//寻路方式选择
	case WM_RBUTTONDOWN:
		if(wParam & MK_RBUTTON)
		{
			perx = LOWORD(lParam);
			pery = HIWORD(lParam);
			int tmp = chooseperfer();
			int tmpn = 0;
			if(1 == tmp)tmpn = MessageBox(hwnd,L"你想走最短路？",L"确认信息",MB_OKCANCEL);
			else if(2 == tmp)tmpn = MessageBox(hwnd,L"你想随便逛逛？",L"确认信息",MB_OKCANCEL);
			else if(3 == tmp)tmpn = MessageBox(hwnd,L"你不喜欢拥挤？",L"确认信息",MB_OKCANCEL);
			else if(4 == tmp)tmpn = MessageBox(hwnd,L"你好害怕被晒？",L"确认信息",MB_OKCANCEL);
			else if(5 == tmp)tmpn = MessageBox(hwnd,L"重置？",L"确认信息",MB_OKCANCEL);
			if(1 == tmpn)perfer = tmp;
			flags = 0;
			flag = 3;

			if(perfer == 5)
			{
				flag = 0;
				flag_over = 0;
				memset(que,0,sizeof(que));
				memset(quetmp,0,sizeof(quetmp));
				Game_Init(hwnd);
				Game_Paint(hwnd);
			}
		}
		break;



	case WM_KEYDOWN://按下的是ESC消息就销毁窗口
		if(wParam == VK_ESCAPE)DestroyWindow(hwnd);
		break;


	case WM_DESTROY://窗口销毁
		Game_CleanUp(hwnd);
		PostQuitMessage(0);
		break;


	default://默认窗口函数
		return DefWindowProc(hwnd,message,wParam,lParam);
	}
	return 0;//正常退出
}

BOOL Game_Init(HWND hwnd)
{
	g_hdc = GetDC(hwnd);
	g_hbitmap = (HBITMAP)LoadImage(NULL,L"Naruto.bmp",IMAGE_BITMAP,0,0,LR_LOADFROMFILE);
	g_point = (HBITMAP)LoadImage(NULL,L"point.bmp",IMAGE_BITMAP,35,35,LR_LOADFROMFILE);
	g_mdc = CreateCompatibleDC(g_hdc);


	COLORREF color = RGB(0,0,0);
	CreatePen(PS_SOLID,1000,color);

	Game_Paint(hwnd);
	ReleaseDC(hwnd,g_hdc);
	return true;
}

VOID Game_Paint(HWND hwnd)
{

	HFONT hFont = CreateFont(30,0,0,0,0,0,0,0,GB2312_CHARSET,0,0,0,0,L"微软雅黑");
	SelectObject(g_hdc,hFont);
	SetBkMode(g_hdc,TRANSPARENT);


	wchar_t text[] = L"重置点这里！";

	wchar_t text1[] = L"请选择起点！";
	wchar_t text2[] = L"请选择终点！";
	wchar_t text3[] = L"请选择寻路模式！";


	wchar_t text11[] = L"我想走最短路";
	wchar_t text22[] = L"我想随便逛逛";
	wchar_t text33[] = L"我不喜欢拥挤";
	wchar_t text44[] = L"我好害怕被晒";

	//绘制原始路线
	SelectObject(g_mdc,g_hbitmap);
	BitBlt(g_hdc,0,0,WINDOW_WIDTH,WINDOW_HEIGHT,g_mdc,0,0,SRCCOPY);
	
	//绘制地图
	for(vector<Road>::iterator iter = road.begin()+1;iter != road.end();++iter)
	{
		SelectObject(g_hdc,g_hPen);
		MoveToEx(g_hdc,(int)iter->ax,(int)iter->ay,NULL);
		LineTo(g_hdc,(int)iter->bx,(int)iter->by);
	}

	//获取天气
	getWeatherInfo();

	//进行天气转换
	convertWearher();

	weather = "今日天气：" + weather;
	temp = "实时气温： " + temp + "摄氏度";

	LPCWSTR wea_temp = stringToLPCWSTR(weather);
	LPCWSTR temp_temp = stringToLPCWSTR(temp);


	SetTextColor(g_hdc,RGB(255,0,0));
	TextOut(g_hdc,1100,50,text,wcslen(text));

	SetTextColor(g_hdc,RGB(255,0,0));
	TextOut(g_hdc,1100,100,wea_temp,wcslen(wea_temp));
	SetTextColor(g_hdc,RGB(255,0,0));
	TextOut(g_hdc,1100,150,temp_temp,wcslen(temp_temp));



	SetTextColor(g_hdc,RGB(255,0,0));
	TextOut(g_hdc,1150,200,text1,wcslen(text1));
	
	


	//绘制起点图标
	if(flag >= 1)
	{
		SelectObject(g_mdc,g_point);
		BitBlt(g_hdc,g_ixs-5,g_iys-5,WINDOW_WIDTH,WINDOW_WIDTH,g_mdc,0,0,SRCCOPY);
		SetTextColor(g_hdc,RGB(255,0,0));
		TextOut(g_hdc,1150,250,text2,wcslen(text2));
	}

	if(flag >= 2)
	{
		SelectObject(g_mdc,g_point);
		BitBlt(g_hdc,g_ixe-5,g_iye-5,WINDOW_WIDTH,WINDOW_WIDTH,g_mdc,0,0,SRCCOPY);
	}


	if(flag >= 2)
	{
		SetTextColor(g_hdc,RGB(255,0,0));
		TextOut(g_hdc,1150,350,text3,wcslen(text3));


		SetTextColor(g_hdc,RGB(0,0,255));
		TextOut(g_hdc,1150,450,text11,wcslen(text11));
		SetTextColor(g_hdc,RGB(0,0,255));
		TextOut(g_hdc,1150,500,text22,wcslen(text22));
		SetTextColor(g_hdc,RGB(0,0,255));
		TextOut(g_hdc,1150,550,text33,wcslen(text33));
		SetTextColor(g_hdc,RGB(0,0,255));
		TextOut(g_hdc,1150,600,text44,wcslen(text44));
	}

	

	if(flag >= 3)
	{
		

		//得到请求的起点和终点
		choosePoint();

		vector<paintRoad> vecroad1;
		vector<paintRoad> vecroad2;
		vector<paintRoad> vecroad;

		


		//第一次绘制请求路线，绘制考虑天气因素在内的路径规划结果
		double sx,sy;

		int tmp = 0;
		if(1 == perfer)tmp = ShortesPath(sn,en,0);
		else if(2 == perfer)tmp = SightPath(sn,en,0);
		else if(3 == perfer)tmp = NoneCrowdPath(sn,en,0);
		else if(4 == perfer)tmp = VoidSunpath(sn,en,0);





		for(vector<Point>::iterator iter = point.begin();iter != point.end();++iter)
		{
			if(que[tmp] == iter->n)
			{
				sx = iter->x;
				sy = iter->y;
				break;
			}
		}

		for(int i = tmp-1; i >= 1; --i)
		{
			double tmpx,tmpy;
			for(vector<Point>::iterator iter = point.begin();iter != point.end();++iter)
			{
				if(que[i] == iter->n)
				{
					tmpx = iter->x;
					tmpy = iter->y;
				}
			}

			//保存路径
			paintRoad p(sx,sy,tmpx,tmpy);
			vecroad1.push_back(p);

			sx = tmpx;
			sy = tmpy;
			
		}

		//修正选择点和起点终点之间的显示

		if(flag_over)
		{
			double tempsx = point[sn].x;
			double tempsy = point[sn].y;
			double tempex = point[en].x;
			double tempey = point[en].y;

			COLORREF colorred = RGB(0,0,255);
			HPEN bluepen = CreatePen(PS_SOLID,4,colorred);

			SelectObject(g_hdc,bluepen);
			MoveToEx(g_hdc,(int)g_ixs,(int)g_iys,NULL);
			LineTo(g_hdc,(int)tempsx,(int)tempsy);

			MoveToEx(g_hdc,(int)tempex,(int)tempey,NULL);
			LineTo(g_hdc,(int)g_ixe,(int)g_iye);
		}




		//第二次绘制请求路线，绘制不考虑天气因素在内的路径规划结果
		tmp = 0;

		//重置结果数组
		memset(c,0,sizeof(c));
		memset(dist,0,sizeof(dist));
		memset(myprev,0,sizeof(myprev));

		//得到第一次规划得到的路径
		for(int i = 0;i < MAX;i++)
		{
			quetmp[i] = que[i];
		}
		memset(que,0,sizeof(que));


		if(1 == perfer)tmp = ShortesPath(sn,en,1);
		else if(2 == perfer)tmp = SightPath(sn,en,1);
		else if(3 == perfer)tmp = NoneCrowdPath(sn,en,1);
		else if(4 == perfer)tmp = VoidSunpath(sn,en,1);


		for(vector<Point>::iterator iter = point.begin();iter != point.end();++iter)
		{
			if(que[tmp] == iter->n)
			{
				sx = iter->x;
				sy = iter->y;
				break;
			}
		}

		for(int i = tmp-1; i >= 1; --i)
		{
			double tmpx,tmpy;
			for(vector<Point>::iterator iter = point.begin();iter != point.end();++iter)
			{
				if(que[i] == iter->n)
				{
					tmpx = iter->x;
					tmpy = iter->y;
				}
			}

			//保存路径
			paintRoad p(sx,sy,tmpx,tmpy);
			vecroad2.push_back(p);
			sx = tmpx;
			sy = tmpy;
		}

		

		//整理两次规划的路径

		//找出重复的部分
		for(vector<paintRoad>::iterator iter1 = vecroad1.begin();iter1 != vecroad1.end();++iter1)
		{
			for(vector<paintRoad>::iterator iter2 = vecroad2.begin();iter2 != vecroad2.end();++iter2)
			{
				if(*iter1 == *iter2)
				{
					paintRoad p(iter1->sx,iter1->sy,iter1->ex,iter1->ey);
					vecroad.push_back(p);
				}
			}

		}


		//绘制路线,最后绘制重叠部分，这样就可以免去重叠部分的去除工作
		

		COLORREF colorred = RGB(255,0,0);
		HPEN redpen = CreatePen(PS_SOLID,4,colorred);

		

		for(vector<paintRoad>::iterator iter = vecroad1.begin();iter != vecroad1.end();++iter)
		{
			SelectObject(g_hdc,redpen);
			MoveToEx(g_hdc,(int)iter->sx,(int)iter->sy,NULL);
			LineTo(g_hdc,(int)iter->ex,(int)iter->ey);
		}

		
		colorred = RGB(0,255,0);
		HPEN greenpen = CreatePen(PS_SOLID,4,colorred);


		for(vector<paintRoad>::iterator iter = vecroad2.begin();iter != vecroad2.end();++iter)
		{
			SelectObject(g_hdc,greenpen);
			MoveToEx(g_hdc,(int)iter->sx,(int)iter->sy,NULL);
			LineTo(g_hdc,(int)iter->ex,(int)iter->ey);
		}



		colorred = RGB(0,0,255);
		HPEN bluepen = CreatePen(PS_SOLID,4,colorred);




		for(vector<paintRoad>::iterator iter = vecroad.begin();iter != vecroad.end();++iter)
		{
			SelectObject(g_hdc,bluepen);
			MoveToEx(g_hdc,(int)iter->sx,(int)iter->sy,NULL);
			LineTo(g_hdc,(int)iter->ex,(int)iter->ey);
		}
		
	}

	DeleteObject(hFont);
}


BOOL Game_CleanUp(HWND hwnd)
{
	DeleteObject(g_hbitmap);
	DeleteDC(g_mdc);
	return true;
}