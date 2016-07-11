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


int temperature;//int�¶�
int weatherlike;//1��ʾ�������꣬2��ʾ��������,3��ʾ��ת�磬4��ʾ��ת��



//�����ٽ���ʾ��·�͵����ͨ��ϵ
int matrix[MAX][MAX];
vector<int> vec[MAX];


int g_ixs = 0,g_iys = 0;//�������
int g_ixe = 0,g_iye = 0;//�յ�����

int perx = 0,pery = 0;//Ѱ·��ʽ�������


int sn = 0,en = 0;//�����յ�

int perfer = 0;//Ѱ·��ʽ


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
	int sun;//1��ʾ·ƫ�� 2��ʾ·ƫ��
	int water;//����·���Ƿ��ˮ����Ţ,1��ʾ��ˮ��0��ʾ����ˮ����·���ˮ�޷�����ʱ������������һ���ǳ����ֵ��ȷ����ѡ���·
	int inroom;//��·���ڻ����⣬1��ʾ���⣬0��ʾ���ڻ�����,���¶ȸ���30������������ʱ��������Ӧѡ�����ڵ�·,�������ĳɸ���
	double curdata;//��ǰѰ·Ȩֵ
};

class Current
{
public:
	char month[10];
	char day[10];
	char week[20];
	char hour[10];
	void getData();//��ȡϵͳʱ������
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


//���ݲ�ͬ��Ѱ·�������ϵͳ��·Ȩֵ
class TimeIn
{
public:
	int hourvalue;
	int gethourvalue();//��ȡ��·ӵ������
	int sun();//��ȡ̫��������Ƕ�����
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
	//1��ʾ̫���ڶ��� 2��ʾ̫�������� 3��ʾ�������̫��
	if(curh <= 11 && curh >= 8)return 1;
	else if(curh >= 12 && curh <= 18)return 2;
	else return 3;
}


//��·�ࣨ��ͼ��

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




//����͵�·������
vector<Point> point;
vector<Road> road;


double dist[MAX];     // ��ʾ��ǰ�㵽Դ������·������
int myprev[MAX];     // ��¼��ǰ���ǰһ�����
double c[MAX][MAX];   // ��¼ͼ�������·������

int que[MAX];//����·��������
int quetmp[MAX];//ԭʼ·������


int Nnode;
int Nedge;


void choosePoint();//�������ת���ɵ����������
int chooseperfer();//���Ѱ·��ʽѡ��



//��ʼ������
void init()
{

	srand((unsigned)time(NULL));

	ifstream f("data.txt");
	memset(matrix,0,sizeof(matrix));
	memset(c,0,sizeof(c));

	int n;
	double x,y;
	//�����α�
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

		//�������·������ָ���͵�·ӵ��ָ��
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
		//��ȡ·�ķ�λ��ƫ���滹�����棩
		if(tw/(2.0) > (10.0*50))r.sun = 1;//ƫ��
		else r.sun = 2;//ƫ��
		//�����ٽ����
		matrix[p1][p2] = road.size();
		matrix[p2][p1] = road.size();
		//�����ٽ��
		vec[p1].push_back(p2);
		vec[p2].push_back(p1);
		road.push_back(r);
	}
	f.close();
}



//���������������¶�ת��
void convertWearher()
{

	//��������
	char str[100];
	int len = weather.length();
	for(int i = 0;i < len;i++)
	{
		str[i] = weather[i];
	}
	
	char subr[] = "��";
	char subs[] = "��";

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
	else //�ж�����ת�껹����ת��
	{
		s = strstr(str,subr);
		s = strstr(s,subs);
		if(s != NULL)weatherlike = 3;
		else weatherlike = 4;
	}

	//�¶�ת��
	int n = 0;
	len = temp.length();
	for(int i = 0;i < len;i++)
	{
		n += (str[i] - '0');
		n *= 10;
	}
	temperature = n/10;

}






//ȨֵѰ·�㷨
void Dijkstra(int v, int n)
{
	for(int i = 1; i < MAX; i++)
	{
		for(int j = 1; j < MAX; j++)
		{
			if(matrix[i][j])
			{
				//��д����õ�·��Ȩֵ
				c[i][j] = road[matrix[i][j]].curdata;
			}
			else c[i][j] = maxint;
		}
	}
	bool s[MAX];    // �ж��Ƿ��Ѵ���õ㵽S������
	for(int i=1; i<=n; ++i)
	{
		dist[i] = c[v][i];
		s[i] = 0;     // ��ʼ��δ�ù��õ�
		if(dist[i] == maxint)
			myprev[i] = 0;
		else
			myprev[i] = v;
	}
	dist[v] = 0;
	s[v] = 1;

	// ���ν�δ����S���ϵĽ���У�ȡdist[]��Сֵ�Ľ�㣬������S��
	// һ��S����������V�ж��㣬dist�ͼ�¼�˴�Դ�㵽������������֮������·������
	// ע���Ǵӵڶ����ڵ㿪ʼ����һ��ΪԴ��
	for(int i=2; i<=n; ++i)
	{
		double tmp = maxint;
		int u = v;
		// �ҳ���ǰδʹ�õĵ�j��dist[j]��Сֵ
		for(int j=1; j<=n; ++j)
			if((!s[j]) && dist[j]<=tmp)
			{
				u = j;              // u���浱ǰ�ڽӵ��о�����С�ĵ�ĺ���
				tmp = dist[j];
			}
			s[u] = 1;    // ��ʾu���Ѵ���S������

			// ����dist
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



// ���Ҵ����v���յ�u��·��
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

//����Ѱ·Ȩֵ����,ÿ��Ѱ·ǰ�������Ȩֵ
void calValue(int n,int f)
{
	//��Ȩֵ����
	for(vector<Road>::iterator iter = road.begin()+1;iter != road.end();++iter)
	{
		iter->curdata = 0;
	}

	//����������������µ�Ӱ��
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

	//��һ����Ѱ·

	if(1 == n)//���·Ѱ·
	{
		for(vector<Road>::iterator iter = road.begin()+1;iter != road.end();++iter)
		{
			iter->curdata += iter->len;//��Ȩֵ�Ӻ�
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


	if(3 == n)//��ӵ��Ѱ·
	{
		for(vector<Road>::iterator iter = road.begin()+1;iter != road.end();++iter)
		{
			iter->curdata += iter->len + iter->len * iter->crowd;
		}
	}

	if(4 == n)//�ܿ�����Ѱ·
	{
		TimeIn timein;
		int flag = timein.sun();
		for(vector<Road>::iterator iter = road.begin()+1;iter != road.end();++iter)
		{
			//���ǵ�һ���ص������
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
		//��ѡ���
		if(fabs(g_ixs - iter->x) + fabs(g_iys - iter->y) < tmpmin1)
		{
			tmpmin1 = (int)fabs(g_ixs - iter->x) + (int)fabs(g_iys - iter->y);
			sn = iter->n;
		}

		//��ѡ�յ�
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




//���·��Ѱ·
int ShortesPath(int s,int e,int f)
{
	calValue(1,f);
	Dijkstra(s,Nnode);
	return searchPath(s,e);
}
//�����
int SightPath(int s,int e,int f)
{
	calValue(2,f);
	Dijkstra(s,Nnode);
	return searchPath(s,e);
}
//�ܿ��˶�ӵ��Ѱ·
int NoneCrowdPath(int s,int e,int f)
{
	calValue(3,f);
	//�Ȼ�ȡʱ�������������������߷��ڣ������ܿ���������֮�����·��Ѱ·
	TimeIn timein;
	int flag = timein.gethourvalue();
	//����3��ʾ�����߷���
	if(flag)Dijkstra(s,Nnode);
	else Dijkstra(s,Nnode);
	return searchPath(s,e);
}
//�ܿ�̫��Ѱ·
int VoidSunpath(int s,int e,int f)
{
	calValue(4,f);
	Dijkstra(s,Nnode);
	return searchPath(s,e);
}
