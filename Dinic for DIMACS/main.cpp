
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <queue>
#include <deque>

#include <sstream>
#include <string>
#include <fstream>  //處理檔案

using namespace std;

#define INF 2147483647

int n,m;
int s,t;

typedef class edge {   //原圖的edge
public:
	int a, b, cap, flow;
    edge(int _a, int _b, int _cap, int _flow) : a(_a), b(_b), cap(_cap), flow(_flow){}
} edge;

typedef class LG_edge {    //levelgraph的edge
public:
	int a, b;
	//int cap, flow;    //不須cap，因cap=原圖的cap - flow
    int corr_edge;      //對應的原圖edge在edgelist的index
    bool valid;
    LG_edge(int _a, int _b, int _corr_edge, bool _valid) : a(_a), b(_b), corr_edge(_corr_edge), valid(_valid){}
} LG_edge;

/*edge newEdge(int a, int b, int cap, int flow){
    edge e();
    e.a=a;e.b=b;e.cap=cap;e.flow=flow;
    return e;
}

LG_edge newLG_edge(int a, int b, int corr_edge){
    LG_edge e();
    e.a=a;e.b=b;
    //e.cap=cap;e.flow=flow;
    e.corr_edge = corr_edge;        //levelgraph的edge對應的原圖的edge
    return e;
}*/

vector<edge> edgelist;  //存放edge的list
vector<vector<int> > adjlist;       //不是vector<vector<int> > adjlist; ，而是存edge在edgelist的index
vector<vector<LG_edge> > levelgraph;
vector<int> level;

queue<int> bfsqueue;    //儲存bfs待執行的點
void bfs(int x){
    vector<int>::iterator it;// 尋訪
    for(it=adjlist[x].begin(); it!=adjlist[x].end(); ++it){
        int index=(*it);
        edge e = edgelist[index];
        if (e.cap > e.flow && level[e.b] > level[x]){ //level[e.b] > level[x]可能是level[e.b]=level[x]+1或level[e.b]=INF
            levelgraph[x].push_back(LG_edge( e.a,e.b, index ,true)); //levelgraph上邊的cap為原圖的residual cap
            if (level[e.b] == INF)      //不讓e.b重複加入queue
            {
                bfsqueue.push(e.b);
                //levelgraphNodes++;
            }
            level[e.b]=level[x]+1;
        }
    };

    return;
}

//每次dfs找到一條s-t path 即回溯回s點，增家路徑上的流量
// 在levelgraph上搜尋，找完blocking flow複雜度是O(nm)
//因無法到達t的edge找過會被刪除，除去能找到s-t path的過程，多次dfs複雜度仍是O(m)
//最多m個 s-t path(因path都有bottleneck arc)，每次找到path以O(n)時間處理(尋找+回溯)

vector< vector<LG_edge>::iterator > its;// 尋訪
long long int arcdo;
int dfs(int x,int bottleneck){
    if (x==t){  //找到path
        return bottleneck; //回傳要增加的flow值
    }
    //vector<LG_edge>::iterator & it = its[x];// 尋訪
    vector<LG_edge>::iterator it;
    for(it=levelgraph[x].begin(); it!=levelgraph[x].end(); ++it){
        if (it->valid==false) continue;
        arcdo++;
        int coEdge =it->corr_edge;
        int residual_cap = edgelist[coEdge].cap - edgelist[coEdge].flow;
        if (residual_cap == 0){ //邊已saturate(上一次找到的路徑的bottleneck),刪除
            it->valid=false;
            //it=levelgraph[x].erase(it);   //這超慢...
            //--it;
            continue;
        }

        int pushes=dfs(it->b,min(bottleneck,residual_cap));
        if (pushes>0){  //已找到path，正在回溯
            edgelist[coEdge].flow = edgelist[coEdge].flow + pushes; //原圖的edge也要增加流量
            edgelist[(coEdge)^1].flow = edgelist[(coEdge)^1].flow - pushes; //反向邊流量
            return pushes; //回溯，回傳要增加的flow值
        }else{  //沒找到path，刪除levelgraph上的邊
            it->valid=false;
            //it=levelgraph[x].erase(it);   //這超慢...
            //--it;
            continue;
        }

    };
    //沒找到s-t path，回傳0
    return 0;
}
void clear( std::vector<LG_edge> &q )    //clear a queue
{
   std::vector<LG_edge> empty;
   std::swap( q, empty );
}
void levelgraphClear(){
    for(int i=0;i<=n;i++){  //release memory
        //levelgraph[i].clear();
        clear(levelgraph[i]);
    }

}
void clear( std::queue<int> &q )    //clear a queue
{
   std::queue<int> empty;
   std::swap( q, empty );
}

int Dinic(){

    int flow=0;
    while(1){
        levelgraphClear();
        for(int i=0;i<=n;i++){
            level[i]=INF;
        }
        level[s]=1;

        cout<< "generating levelgraph.. " << endl;
        //開始計算levelgraph
        clear(bfsqueue); //init
        bfsqueue.push(s);
        while(!bfsqueue.empty() && level[t] > level[bfsqueue.front()]){   //若level[t] = level[bfsqueue.front()，代表levelgraph以建立完t點那層
            bfs( bfsqueue.front());
            bfsqueue.pop();
        }

        clear(bfsqueue); //init
        cout<< "now levelgraph level = " << level[t] << endl;
        cout<< "now flow = " << flow << endl;
        if (level[t]==INF) break;   //無法到達t

        //開始計算blocking flow
        cout<< "computing blocking flow.. " << endl;
        int dfstimes=0;
        arcdo=0;

        for(int i=0;i<=n;i++){
            its[i]=levelgraph[i].begin();
        }
        while(int pushes= dfs(s,INF)){  //pushes=0則已經找不到s-t path
            flow = flow +pushes;
            dfstimes++;
        }
        cout<< "dfs times = " << dfstimes << endl;
        cout<< "arcdo = " << arcdo << endl;
    }

    return flow;
}

bool readfile()
{
    string testdataNO;
    cin >>  testdataNO;
    string testdataName="BVZ-tsukuba"+testdataNO+".max";
    //string testdataName="104_0412_16bins.max";
    //string testdataName="graph3x3.txt";
    //string testdataName="graph5x5.txt";

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
            levelgraph.resize(n+1);
            level.resize(n+1);
            its.resize(n+1);

        }else if (cha=='n'){
            int _node;
            char _b;
            infile >> _node >> _b;
            if (_b=='s'){
                s=(int)_node;
            }else if (_b=='t'){
                t=(int)_node;
            }
        }else if (cha=='a'){

            int a,b, cap;
            infile >> a >> b >> cap;
            edgelist.push_back(edge(a,b,cap,0));
            adjlist[a].push_back(2*arcCount);
            edgelist.push_back(edge(b,a,0,0));
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





    int flow=Dinic();
    cout <<"flow = " << flow << endl;

    /*for(int i=0;i<m;i++){
        if (edgelist[i*2].flow > 0)
            cout<< edgelist[i*2].a <<" "<<edgelist[i*2].b <<" " << edgelist[i*2].flow << endl;
        else
            cout<< edgelist[i*2+1].a <<" "<<edgelist[i*2+1].b <<" " << edgelist[i*2+1].flow << endl;
    }*/

    }
    system("pause");
    return 0;
}
