
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <queue>
#include <deque>
#include <cstring>

#include <sstream>
#include <string>
#include <fstream>  //處理檔案
#include<limits.h> //含LLONG_MAX
#include<climits> //含LLONG_MAX

#include<time.h>

using namespace std;




#define INF 2147483647

int n,m;
int s,t;

vector<int> b;

typedef class edge {   //原圖的edge
public:
	int a, b, cap, cost, flow;
    edge(int _a, int _b, int _cap, int _cost, int _flow) : a(_a), b(_b), cap(_cap), cost(_cost), flow(_flow){}
} edge;

vector<edge> edgelist;  //存放edge的list，edgelist[i]與edgelist[i^1]互為反向邊
vector<vector<int> > adjlist;       //不是vector<vector<int> > adjlist; ，而是存edge在edgelist的index

vector<int> potential;

int total_path_flows;

void BellmanFord(){
    potential.resize(n+1);

    int arcCount=edgelist.size();
    for(int i=1;i<=n;i++){
        edgelist.push_back(edge(0,i,0,0,0));
        adjlist[0].push_back(arcCount);
        arcCount++;
    }

    int dist[n+1];

    dist[0]=0;      //0點為 新增的點
    for(int i=1;i<=n;i++){
        dist[n]=INF;
    }

    for(int i=0;i<=n;i++){
        for(int j=0;j<=n;j++){
            for(int k=0;k< adjlist[j].size();k++){
                edge & e=edgelist[adjlist[j][k]];
                if (e.cap > e.flow || j==0){
                    int t=edgelist[adjlist[j][k]].b;
                    int w=edgelist[adjlist[j][k]].cost;
                    if (dist[t] > dist[j]+w){
                        dist[t] =dist[j]+w;
                    }
                }
            }
        }
    }

        for(int j=0;j<=n;j++){  //判斷有無Negative cycle
            for(int k=0;k< adjlist[j].size();k++){
                edge & e=edgelist[adjlist[j][k]];
                if (e.cap > e.flow || j==0){
                    int t=edgelist[adjlist[j][k]].b;
                    int w=edgelist[adjlist[j][k]].cost;
                    if (dist[t] > dist[j]+w){
                        cout <<"Negative cycle(of costs)"<<endl;
                    }
                }
            }
        }
    //for(int i=1;i<=n;i++){
        //cout << dist[i]<< " ";
    //}
    //cout <<" dist"<<endl;

    //reweight
    for(int j=1;j<=n;j++){
        potential[j]=dist[j];
    }

    for(int i=1;i<=n;i++){
        edgelist.pop_back();
        adjlist[0].pop_back();
        arcCount--;
    }
}

void dijkstra(int x,int y){
    int d[n+1]; //d[i] = dist from x to i
    int p[n+1]; //parent
    int p_e[n+1];  //parent edge

    bool visit[n+1];
    for(int i=1;i<=n;i++) visit[i]=false;
    for(int i=1;i<=n;i++){
        d[i]=INF;
    }

    d[x]=0;
    p[x]=x;

    priority_queue<pair<int,int> , vector<pair<int,int> >, greater< pair<int,int> > > Q;
    Q.push(make_pair(0,x));

    while(!Q.empty()){
        int fix=Q.top().second;
        Q.pop();
        if (visit[fix]==true) continue;

        visit[fix]=true;

        for(int lt: adjlist[fix]){
            edge e=edgelist[lt];
            int reduced_cost=e.cost+potential[e.a]-potential[e.b];
            if (e.cap > e.flow  && !visit[e.b] && d[e.b] > reduced_cost + d[fix]){
                d[e.b] = reduced_cost + d[fix];
                p[e.b]=fix;
                p_e[e.b]=lt;
                Q.push(make_pair(d[e.b],e.b));
            }
        }
        /*if (fix==y){    //進行特殊邊處理，等同於加入特殊邊的效果
            for(int i=1;i<=n;i++){
                //int reduced_cost=potential[y]-potential[i];   //wrong
                int reduced_cost=0;     //correct
                if (!visit[i] && d[i] > reduced_cost + d[y]){
                    d[i] = reduced_cost + d[y];

                    Q.push(make_pair(d[i],i));
                }
            }
        }*/

    }
    //for(int i=1;i<=n;i++){
    //    cout<< d[i]<<" ";
    //}

    if (d[y]==INF) return;  //沒路
    //進行特殊邊處理，等同於加入特殊邊的效果
    for(int i=1;i<=n;i++){
        int reduced_cost=0;     //correct
        if (!visit[i] && d[i] > reduced_cost + d[y]){
            d[i] = reduced_cost + d[y];
        }
    }

    int min_cap=INF;
    int it=y;
    while(it!=x){   //找shortest path上的邊的最小剩餘容量
        min_cap=min(min_cap, edgelist[p_e[it]].cap-edgelist[p_e[it]].flow);
        it=p[it];
    }
    //cout<<min_cap<<endl;
    min_cap=min(min_cap, min(b[x], -b[y]) );
    //cout<<min_cap<<endl;

    it=y;
    while(it!=x){   //增加flow
        edgelist[p_e[it]].flow = edgelist[p_e[it]].flow + min_cap;
        edgelist[p_e[it]^1].flow = edgelist[p_e[it]^1].flow - min_cap;
        it=p[it];
    }
    b[x]=b[x]-min_cap;
    b[y]=b[y]+min_cap;
    total_path_flows=total_path_flows+min_cap;
    //cout<<b[x]<<" "<<b[y]<<endl;
    //cout<<endl;


    //reweight
    for(int j=1;j<=n;j++){
        potential[j]=potential[j]+d[j];
    }

}



bool readfile()
{
    string testdataNO;
    //cin >>  testdataNO;
    //string testdataName="BVZ-tsukuba"+testdataNO+".max";
    string testdataName="transp1.net";

    std::ifstream infile(testdataName.c_str());
    if ( ! infile.is_open() ){
        cout <<testdataName <<" Failed to open" << endl;
        return false;
    }
    char cha;
    int arcCount=0;
    while(!infile.eof()){
        infile >> cha;

        if (cha=='c'){
            string drops;
            getline(infile,drops);
            cout <<drops<<endl;
            continue;
        }else if (cha=='p'){  //初始化
            string drops;
            infile>>drops;
            //cout<<drops;

            infile >> n >> m;

            cout <<"n = " << n <<"m = " << m << endl;
            adjlist.resize(n+1);


            b.resize(n+1);
            b.clear();
        }else if (cha=='n'){
            int _node;
            int _flow;  //b
            infile >> _node >> _flow;
            b[_node]=_flow;

        }else if (cha=='a'){

            int a,b, mcap, Mcap, cost;
            infile >> a >> b >> mcap >> Mcap >> cost;
            edgelist.push_back(edge(a,b,Mcap,cost,0));
            adjlist[a].push_back(2*arcCount);
            edgelist.push_back(edge(b,a,0,-cost,0));
            adjlist[b].push_back(2*arcCount+1);

            arcCount++;
            //cout <<"read "<<arcCount <<" arc"<< endl;
        }

    }

    cout <<"read file complete"<< endl;
    return true;
}


int main()
{
    if (readfile())
    {

    // 儲存時間用的變數
    clock_t start, endtime;
    double cpu_time_used;

    // 計算開始時間
    start = clock();


    total_path_flows=0;
    int paths_added=0;
    //int s=0,t=0;

    BellmanFord();

    bool flag=true;
    while(flag){
        flag=false;
        for(int i=1;i<=n;i++){  //找shortest path(of costs)
            for(int j=1;j<=n;j++){
                if (b[i]<=0) break;
                if (b[j]<0){
                    //cout<<"s="<<i<<" t="<<j<<endl;
                    dijkstra(i,j);
                    paths_added++;
                }
            }

        }
        for(int i=1;i<=n;i++){
            if (b[i]!=0) flag=true;
            //if (b[i]>0){
            //    cout<<b[i]<<" b[i]>0"<<endl;
            //}else if (b[i]<0){
            //    cout<<b[i]<<" b[i]<0"<<endl;
            //}
        }
    }
    /*for(int i=1;i<=n;i++){
        if (b[i]>0){
            cout<<b[i]<<" b[i]>0"<<endl;
        }else if (b[i]<0){
            cout<<b[i]<<" b[i]<0"<<endl;
        }
    }*/
    cout<<"total_path_flows="<<total_path_flows<<endl;

    int min_cost=0;
    int arc_flow=0;
    for(int i=0;i<edgelist.size();i++){
        if (edgelist[i].flow > 0){
            min_cost = min_cost + edgelist[i].flow * edgelist[i].cost;
            arc_flow=arc_flow+edgelist[i].flow;
        }
    }
    cout<< "paths added=" <<paths_added<<endl;
    cout<< "min cost=" <<min_cost<<endl;
    //cout<< "total positive arc flow=" <<arc_flow<<endl;

    /*for(int i=0;i<edgelist.size();i++){
        if (edgelist[i].cap > 0){
            cout<< "flow=" <<edgelist[i].flow<< "cap=" <<edgelist[i].cap<<endl;
        }
    }*/


    // 計算結束時間
    endtime = clock();

    // 計算實際花費時間
    cpu_time_used = ((double) (endtime - start)) / CLOCKS_PER_SEC;

    printf("Time = %f\n", cpu_time_used);
    }
    system("pause");
    return 0;
}
