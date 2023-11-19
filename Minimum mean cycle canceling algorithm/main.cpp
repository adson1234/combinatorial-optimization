
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <stack>
#include <queue>
#include <deque>
#include <cstring>

#include <sstream>
#include <string>
#include <fstream>  //處理檔案
#include<limits.h> //含LLONG_MAX
#include<climits> //含LLONG_MAX

#include <windows.h>
#include <time.h>


using namespace std;




#define INF 2147483647

int n,m;
int s,t;

typedef class edge {   //原圖的edge
public:
	int a, b, cap, cost, flow;
    edge(int _a, int _b, int _cap, int _cost, int _flow) : a(_a), b(_b), cap(_cap), cost(_cost), flow(_flow){}
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

        //cout<< "generating levelgraph.. " << endl;
        //開始計算levelgraph
        clear(bfsqueue); //init
        bfsqueue.push(s);
        while(!bfsqueue.empty() && level[t] > level[bfsqueue.front()]){   //若level[t] = level[bfsqueue.front()，代表levelgraph已建立完t點那層
            bfs( bfsqueue.front());
            bfsqueue.pop();
        }

        clear(bfsqueue); //init
        //cout<< "now levelgraph level = " << level[t] << endl;
        //cout<< "now flow = " << flow << endl;
        if (level[t]==INF) break;   //無法到達t

        //開始計算blocking flow
        //cout<< "computing blocking flow.. " << endl;
        int dfstimes=0;
        arcdo=0;

        for(int i=0;i<=n;i++){
            its[i]=levelgraph[i].begin();
        }
        while(int pushes= dfs(s,INF)){  //pushes=0則已經找不到s-t path
            flow = flow +pushes;
            dfstimes++;
        }
        //cout<< "dfs times = " << dfstimes << endl;
        //cout<< "arcdo = " << arcdo << endl;
    }

    return flow;
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
    int total_b=0;
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

            cout <<"n = " << n  <<" m = " << m << endl;
            s=n+1;
            t=n+2;
            n=n+2;
            adjlist.resize(n+1);  //新增s(node n+1),t(node n+2)，s連結所有b(v)>0，t連結所有b(v)<0
            levelgraph.resize(n+1);
            level.resize(n+1);
            its.resize(n+1);
        }else if (cha=='n'){
            int _node;
            int _flow;  //b
            infile >> _node >> _flow;

            if (_flow>0){
                total_b=total_b+_flow;

                edgelist.push_back(edge(s,_node,_flow,0,0));
                adjlist[s].push_back(2*arcCount);
                edgelist.push_back(edge(_node,n+1,0,0,0));
                adjlist[_node].push_back(2*arcCount+1);
                arcCount++;
            }else if (_flow <0){
                edgelist.push_back(edge(_node,t,-_flow,0,0));
                adjlist[_node].push_back(2*arcCount);
                edgelist.push_back(edge(t,_node,0,0,0));
                adjlist[t].push_back(2*arcCount+1);
                arcCount++;
            }

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

    cout<<"total positive b[v]="<<total_b<<endl;
    cout <<"read file complete"<< endl;
    return true;
}

const int maxn=30000;
bool visit[maxn+1];
int discover[maxn+1];  //discover Time
int Low[maxn+1];   //smallest value of a subtree rooted at v to which it can climb up by a back edge
int pred[maxn+1];      //parent
int Time;

bool instack[maxn+1];
stack<int> stackA;
vector<int> headnodes;

void DFS_Compute_Low(int v) {   //DFS的遞迴函數，找SCC
    //cout<<v<<" ";
    visit[v]=true;
    Low[v]=discover[v] = ++Time;

    instack[v]=true;
    stackA.push(v);

    for(int i=0;i<adjlist[v].size();i++){    //for each w in Adj(v)
        edge & e=edgelist[adjlist[v][i]];
        int w=e.b;


        if (e.cap>e.flow){
        if (visit[w] == false) {    //在residual network上找

            pred[w]=v;
            DFS_Compute_Low(w);
            // When RDFS_Compute_Low(w) returns,
            // Low[w] stores the
            // lowest value it can climb up
            // for a subtree rooted at w.
            Low[v] = min(Low[v], Low[w]);
        } else{

            if (instack[w]==false){
                // {v, w} is a cross edge
            }else{
                // {v, w} is a back edge
                if (w!=pred[v])            //not parent
                    Low[v] = min(Low[v], discover[w]);
            }
        }
        }

    }


    if (Low[v]==discover[v]){   //v為SCC的head node
        headnodes.push_back(v);

        int temp;
        while(1){
            int temp=stackA.top();stackA.pop();
            instack[temp]=false;
            if (temp==v) break;
        }
    }
}

void Tarjan_SCC(){

    headnodes.clear();
    Time=0;
    for(int i=1;i<=n;i++){
        instack[i]=false;
        visit[i]=false;
    }

    for(int i=1;i<=n;i++){
        if (!visit[i]){
            DFS_Compute_Low(i);
        }
	}
}


int mmc_v;
int mmc_k;

//int prev[n+1][n+1];     //紀錄F[k][i]路徑的前一個node
//int prev_edge[n+1][n+1];     //紀錄F[k][i]路徑的前一個node
vector<vector<int> > pre;
vector<vector<int> > pre_edge;



//可改進的部分:SCC僅是圖的一部份，在其中找MMC時不需要計算全圖
//可建立一個子圖之類的..
double karp(int s){ 	//Karp's algorithm求minimum mean cycle length
    //s為此SCC的headnode，所以肯定有cycle

	//karp's algorithm

	//long long int F[n+1+2][n+1+2];  //F[k][i]=經k邊後到i的最短距離 (自點1開始)
	vector<vector<long long int> > F;
    F.resize(n+1+2);
    for(int i=0; i<n+1+2; ++i){
        F[i].resize(n+1+2);
    }

	for(int k=0;k<=n;k++){	//初始化
		for(int i=1;i<=n;i++)
			F[k][i]=LLONG_MAX/2; //距離無限
	}
	F[0][s]=0;	//s為起始點，距自己0


	for(int k=1;k<=n;k++){  //求F[][]
		for(int j=1;j<=n;j++){
			for(int i=0;i<adjlist[j].size();i++){
                edge e = edgelist[ adjlist[j][i]];
                if (e.flow < e.cap){            //在residual network上找 cycle
                    if (F[k][e.b] > F[k-1][j]+e.cost){
                        F[k][e.b]=F[k-1][j]+e.cost;
                        pre[k][e.b]=j;
                        pre_edge[k][e.b]=adjlist[j][i];
                    }
                }
			}
		}
	}
	//cout<<"123"<<endl;
	//for(int k=0;k<=n;k++){	//測試列印
	//	for(int i=1;i<=n;i++)
	//		printf("%lld ",F[k][i]);
	//	printf("\n");
	//}

	double temp_min=1e18;
	for(int i=1;i<=n;i++){  //求min(max((F[n][i]-F[k][i])/(n-k)))

		double temp_max=-1e18+1;
		double temp;
		int temp_k;
		for(int k=0;k<=n-1;k++){
			temp=(double)(F[n][i]-F[k][i])/(n-k);
			if (temp_max<=temp){	//要取最大的k,所以用<=
				temp_max=temp;
				temp_k=k;
			}
		}

		if (temp_min>temp_max){
			temp_min=temp_max;

			mmc_v=i;
            mmc_k=temp_k;

		}
	}
	//cout<<F[n][mmc_v]<<endl;

	//cout<<endl;
	//cout<<"mmc ml:"<<temp_min<<endl;
	return temp_min;
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


    int flow=Dinic();
    cout <<"Dinic max flow = " << flow << endl;

    n=n-2;  //s,t點已不需要
    for(int j=n+1;j<=n+2;j++){      //連到s,t的邊清除
        for(int i=0;i<adjlist[j].size();i++){
            edge & e = edgelist[adjlist[j][i]];
            e.cap=0;e.cost=0;e.flow=0;      //清除,未刪除
            edge & e2 = edgelist[adjlist[j][i]^1];
            e2.cap=0;e2.cost=0;e2.flow=0;
        }
    }
    cout <<"error test" << endl;
    pre.resize(n+1+2);
    pre_edge.resize(n+1+2);
    for(int i=0; i<n+1; ++i){
        pre[i].resize(n+1+2);
        pre_edge[i].resize(n+1+2);
    }
    bool done=false;
    cout <<"error test" << endl;

    int cycles_cancelled=0;
    while(!done){

        done=true;
        Tarjan_SCC();

        //cout<<headnodes.size()<<" headnodes"<<endl;
        for(int k=0;k<headnodes.size();k++){
            if (karp(headnodes[k]) < 0){
                cycles_cancelled++;
                done=false;
                //以下為找karp演算法的mmc
                //cout<<"headnode "<<headnodes[k]<<" processing"<<endl;
                //cout<<mmc_v<<" "<<pre[n][mmc_v]<<" "<<pre[n-1][pre[n][mmc_v]]<<endl;
                bool mark[n+1]={false}; //開始找出negative cycle的nodes
                int p[n+1]; //the path from s to mmc_v(<=path中的cycle必為MMC)
                p[n]=mmc_v;
                int cycle_start=0;
                for(int i=n;i>=1;i--){
                    if (mark[p[i]]==true){
                        cycle_start=i;
                        break;
                    }
                    mark[p[i]]=true;
                    p[i-1]=pre[i][p[i]];
                }

                int min_cap=INF;
                int next=cycle_start+1;
                while(1){              //find min_cap
                    int index = pre_edge[next][p[next]];
                    edge & e = edgelist[index];
                    min_cap=min(min_cap,e.cap-e.flow);

                    if (p[next]==p[cycle_start]) break;

                    next++;
                }
                next=cycle_start+1;
                while(1){               //increase cycle flow
                    int index = pre_edge[next][p[next]];
                    edgelist[index].flow = edgelist[index].flow + min_cap;
                    edgelist[index^1].flow = edgelist[index^1].flow - min_cap;

                    if (p[next]==p[cycle_start]) break;
                    next++;
                }
            }
        }
    }

    int min_cost=0;
    int arc_total_flow=0;
    for(int j=1;j<=n;j++){
        for(int i=0;i<adjlist[j].size();i++){
            edge & e = edgelist[ adjlist[j][i]];
            if (e.flow > 0 && e.b!=n+2 && e.b!=n+1){    //連到s,t的邊不計算
                min_cost = min_cost + e.flow * e.cost;
                arc_total_flow=arc_total_flow+e.flow;
            }
        }
    }
    cout<< "cycles cancelled=" <<cycles_cancelled<<endl;
    cout<< "min cost=" <<min_cost<<endl;
    //cout<< "arc total flow=" <<arc_total_flow<<endl;
    /*
    for(int i=0;i<edgelist.size();i++){
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
