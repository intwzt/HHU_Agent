#include <iostream>
#include <fstream>
#include <cstring>
#include <cstdio>
#include <vector>
#include <cmath>
#include <ctime>
#include <queue>
#include <string>
#include <algorithm>
#include <cstring>
#include <windows.h>

using namespace std;

const int MAX = 1000;
const int maxint = 999999;



std::string City_code;
std::string weather;
std::string temp;


int temperature;//int温度
int weatherlike;//1表示可能有雨，2表示天气晴朗,3表示雨转晴，4表示晴转雨



//改用临界表表示道路和点的连通关系
int matrix[MAX][MAX];
vector<int> vec[MAX];


int g_ixs = 0,g_iys = 0;//起点坐标
int g_ixe = 0,g_iye = 0;//终点坐标

int perx = 0,pery = 0;//寻路方式鼠标坐标


int sn = 0,en = 0;//起点和终点

int perfer = 0;//寻路方式


int flag_over = 0;




class Point
{
public:
	int n;
	double x;
	double y;
	Point();
	Point(int n,double a,double b):n(n),x(a),y(b) {};
};


class Road
{
public:
	double ax,ay,bx,by;
	double len;
	int sight;
	int crowd;
	int sun;//1表示路偏东 2表示路偏西
	int water;//雨天路面是否积水或泥泞,1表示积水，0表示不积水，但路面积水无法行走时，将参数乘以一个非常大的值以确保不选择该路
	int inroom;//道路室内或室外，1表示室外，0表示室内或阴凉,当温度高于30度且天气晴朗时，必须相应选择室内道路,将参数改成负数
	double curdata;//当前寻路权值
};

class Current
{
public:
	char month[10];
	char day[10];
	char week[20];
	char hour[10];
	void getData();//获取系统时间数据
};
Current curtime;

void Current::getData()
{
	time_t t = time(0);
	strftime(month, sizeof(month), "%m",localtime(&t));
	strftime(day, sizeof(day), "%d",localtime(&t));
	strftime(week, sizeof(week), "%A",localtime(&t));
	strftime(hour, sizeof(hour), "%H",localtime(&t));
}


//根据不同的寻路需求更改系统道路权值
class TimeIn
{
public:
	int hourvalue;
	int gethourvalue();//获取道路拥挤数据
	int sun();//获取太阳光照射角度数据
};

int TimeIn::gethourvalue()
{
	Current cu;
	cu.getData();
	int curh = 0;
	curh = cu.hour[0]-'0';
	curh = curh*10;
	curh = curh + cu.hour[1] - '0';
	hourvalue = curh;
	if((curh >= 7 && curh <= 8)
		||(curh >= 11 && curh <= 13)
		||(curh >= 14 && curh <= 16)
		||(curh >= 21 && curh <= 22))return 1;
	else return 0;
}

int TimeIn::sun()
{
	Current cu;
	cu.getData();
	int curh = 0;
	curh = cu.hour[0]-'0';
	curh = curh*10;
	curh = curh + cu.hour[1] - '0';
	//1表示太阳在东边 2表示太阳在西边 3表示正午或无太阳
	if(curh <= 11 && curh >= 8)return 1;
	else if(curh >= 12 && curh <= 18)return 2;
	else return 3;
}


//道路类（绘图）

class paintRoad
{
public:
	double sx;
	double sy;
	double ex;
	double ey;
	paintRoad();
	paintRoad(double a,double b,double c,double d):sx(a),sy(b),ex(c),ey(d){}
	bool operator == (paintRoad p)
	{
		if(this->sx == p.sx && this->sy == p.sy && this->ex == p.ex && this->ey == p.ey)return true;
		else return false;
	}
};




//交点和道路的容器
vector<Point> point;
vector<Road> road;


double dist[MAX];     // 表示当前点到源点的最短路径长度
int myprev[MAX];     // 记录当前点的前一个结点
double c[MAX][MAX];   // 记录图的两点间路径长度

int que[MAX];//保存路径的数组
int quetmp[MAX];//原始路径数组


int Nnode;
int Nedge;


void choosePoint();//鼠标坐标转换成点的修正函数
int chooseperfer();//鼠标寻路方式选择



//初始化函数
void init()
{

	srand((unsigned)time(NULL));

	ifstream f("data.txt");
	memset(matrix,0,sizeof(matrix));
	memset(c,0,sizeof(c));

	int n;
	double x,y;
	//调整游标
	Point fp(0,0,0);
	point.push_back(fp);
	Road fr;
	road.push_back(fr);

	Nnode = 78;
	Nedge = 116;
	for(int i = 1; i <= Nnode; i++)
	{
		f>>n>>x>>y;
		Point p(n,x*50,y*45);
		point.push_back(p);
	}
	int p1,p2,sight,cr,wa,inr;
	for(int i = 1; i <= Nedge; i++)
	{
		f>>p1>>p2>>sight>>cr>>wa>>inr;

		//随机生成路径景观指数和道路拥挤指数
		sight = rand()%10 + 1;

		Road r;
		r.ax = point[p1].x;
		r.ay = point[p1].y;
		r.bx = point[p2].x;
		r.by = point[p2].y;
		r.crowd = cr;
		r.sight = sight;
		r.water = wa;
		r.inroom = inr;
		double tx = fabs(r.ax - r.bx);
		double ty = fabs(r.ay - r.by);
		r.len = sqrt(tx * tx + ty * ty);
		double tw = fabs(r.ax + r.bx);
		//获取路的方位（偏向东面还是西面）
		if(tw/(2.0) > (10.0*50))r.sun = 1;//偏东
		else r.sun = 2;//偏西
		//建立临界矩阵
		matrix[p1][p2] = road.size();
		matrix[p2][p1] = road.size();
		//建立临界表
		vec[p1].push_back(p2);
		vec[p2].push_back(p1);
		road.push_back(r);
	}
	f.close();
}



//进行天气评估和温度转换
void convertWearher()
{

	//天气评估
	char str[100];
	int len = weather.length();
	for(int i = 0;i < len;i++)
	{
		str[i] = weather[i];
	}
	
	char subr[] = "雨";
	char subs[] = "晴";

	int flagr = 0;
	int flags = 0;
	
	char *s = strstr(str,subr);
	if(s != NULL)flagr = 1;
	s = strstr(str,subs);
	if(s != NULL)flags = 1;

	if(!(flags && flagr))
	{
		if(flagr)weatherlike = 1;
		else weatherlike = 2;
	}
	else //判断是晴转雨还是雨转晴
	{
		s = strstr(str,subr);
		s = strstr(s,subs);
		if(s != NULL)weatherlike = 3;
		else weatherlike = 4;
	}

	//温度转换
	int n = 0;
	len = temp.length();
	for(int i = 0;i < len;i++)
	{
		n += (str[i] - '0');
		n *= 10;
	}
	temperature = n/10;

}






//权值寻路算法
void Dijkstra(int v, int n)
{
	for(int i = 1; i < MAX; i++)
	{
		for(int j = 1; j < MAX; j++)
		{
			if(matrix[i][j])
			{
				//填写计算好的路径权值
				c[i][j] = road[matrix[i][j]].curdata;
			}
			else c[i][j] = maxint;
		}
	}
	bool s[MAX];    // 判断是否已存入该点到S集合中
	for(int i=1; i<=n; ++i)
	{
		dist[i] = c[v][i];
		s[i] = 0;     // 初始都未用过该点
		if(dist[i] == maxint)
			myprev[i] = 0;
		else
			myprev[i] = v;
	}
	dist[v] = 0;
	s[v] = 1;

	// 依次将未放入S集合的结点中，取dist[]最小值的结点，放入结合S中
	// 一旦S包含了所有V中顶点，dist就记录了从源点到所有其他顶点之间的最短路径长度
	// 注意是从第二个节点开始，第一个为源点
	for(int i=2; i<=n; ++i)
	{
		double tmp = maxint;
		int u = v;
		// 找出当前未使用的点j的dist[j]最小值
		for(int j=1; j<=n; ++j)
			if((!s[j]) && dist[j]<=tmp)
			{
				u = j;              // u保存当前邻接点中距离最小的点的号码
				tmp = dist[j];
			}
			s[u] = 1;    // 表示u点已存入S集合中

			// 更新dist
			for(int j=1; j<=n; ++j)
				if((!s[j]) && c[u][j]<=maxint)
				{
					double newdist = dist[u] + c[u][j];
					if(newdist <= dist[j])
					{
						dist[j] = newdist;
						myprev[j] = u;
					}
				}
	}
}



// 查找从起点v到终点u的路径
int searchPath(int v, int u)
{
	memset(que,0,sizeof(que));
	int tot = 1;
	que[tot] = u;
	tot++;
	int tmp = myprev[u];
	while(tmp != v)
	{
		que[tot] = tmp;
		tot++;
		tmp = myprev[tmp];
	}
	que[tot] = v;
	flag_over = 1;
	return tot;
}

//计算寻路权值参数,每次寻路前都需更新权值
void calValue(int n,int f)
{
	//将权值清零
	for(vector<Road>::iterator iter = road.begin()+1;iter != road.end();++iter)
	{
		iter->curdata = 0;
	}

	//如果考虑天气和气温的影响
	if(f)
	{
		for(vector<Road>::iterator iter = road.begin()+1;iter != road.end();++iter)
		{
			if(weatherlike == 1 && iter->water == 1)
			{
				iter->curdata =  iter->len + MAX;
			}
			else if(weatherlike == 2 && temperature >= 30 && temperature <= 34 && !iter->inroom)
			{
				iter->curdata = -iter->len + iter->len;
			}
			else if(weatherlike == 2 && temperature >= 35 && !iter->inroom)
			{
				iter->curdata = -iter->len * 10;
			}
			else;
		}
	}

	//单一因素寻路

	if(1 == n)//最短路寻路
	{
		for(vector<Road>::iterator iter = road.begin()+1;iter != road.end();++iter)
		{
			iter->curdata += iter->len;//将权值加和
		}
	}

	if(2 == n)
	{
		for(vector<Road>::iterator iter = road.begin()+1;iter != road.end();++iter)
		{
			srand((unsigned)time(NULL));
			int tmp = rand()%2 -1;
			iter->curdata += tmp * iter->sight;
		}
	}


	if(3 == n)//不拥挤寻路
	{
		for(vector<Road>::iterator iter = road.begin()+1;iter != road.end();++iter)
		{
			iter->curdata += iter->len + iter->len * iter->crowd;
		}
	}

	if(4 == n)//避开阳光寻路
	{
		TimeIn timein;
		int flag = timein.sun();
		for(vector<Road>::iterator iter = road.begin()+1;iter != road.end();++iter)
		{
			//考虑单一因素的情况下
			if(flag == iter->sun)
			{
				iter->curdata += iter->len + iter->len * 2.0;
			}
			else iter->curdata += iter->len + iter->len * 0;
		}
	}
}


void choosePoint()
{
	int tmpmin1 = maxint;
	int tmpmin2 = maxint;
	for(vector<Point>::iterator iter = point.begin()+1;iter != point.end();++iter)
	{
		//挑选起点
		if(fabs(g_ixs - iter->x) + fabs(g_iys - iter->y) < tmpmin1)
		{
			tmpmin1 = (int)fabs(g_ixs - iter->x) + (int)fabs(g_iys - iter->y);
			sn = iter->n;
		}

		//挑选终点
		if(fabs(g_ixe - iter->x) + fabs(g_iye - iter->y) < tmpmin2)
		{
			tmpmin2 = (int)fabs(g_ixe - iter->x) + (int)fabs(g_iye - iter->y);
			en = iter->n;
		}
	}
	
}

int chooseperfer()
{
	if(perx >= 1150 && perx <= 1150 + 6 * 25 && pery >= 450 && pery <= 475)return  1;
	else if(perx >= 1150 && perx <= 1150 + 6 * 25 && pery >= 500 && pery <= 525)return 2;
	else if(perx >= 1150 && perx <= 1150 + 6 * 25 && pery >= 550 && pery <= 575)return 3;
	else if(perx >= 1150 && perx <= 1150 + 6 * 25 && pery >= 600 && pery <= 625)return 4;
	else if(perx >= 1100 && perx <= 1100 + 5 * 25 && pery >= 50 && pery <= 75)return 5;
	return 6;
}




//最短路径寻路
int ShortesPath(int s,int e,int f)
{
	calValue(1,f);
	Dijkstra(s,Nnode);
	return searchPath(s,e);
}
//随便逛逛
int SightPath(int s,int e,int f)
{
	calValue(2,f);
	Dijkstra(s,Nnode);
	return searchPath(s,e);
}
//避开人多拥挤寻路
int NoneCrowdPath(int s,int e,int f)
{
	calValue(3,f);
	//先获取时间参数，如果处在人流高峰期，尽量避开人流，反之按最短路径寻路
	TimeIn timein;
	int flag = timein.gethourvalue();
	//参数3表示人流高峰期
	if(flag)Dijkstra(s,Nnode);
	else Dijkstra(s,Nnode);
	return searchPath(s,e);
}
//避开太阳寻路
int VoidSunpath(int s,int e,int f)
{
	calValue(4,f);
	Dijkstra(s,Nnode);
	return searchPath(s,e);
}
