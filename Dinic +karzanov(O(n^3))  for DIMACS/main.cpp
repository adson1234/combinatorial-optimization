
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
	int cap, flow;    //cap=原圖的cap - flow
    int corr_edge;      //對應的原圖edge在edgelist的index
    int redge;      //對應的rLG的index
    LG_edge(int _a, int _b,int _cap,int _flow, int _corr_edge,int _redge) : a(_a), b(_b),cap(_cap), flow(_flow), corr_edge(_corr_edge),redge(_redge){}
} LG_edge;


vector<edge> edgelist;  //存放edge的list
vector<vector<int> > adjlist;       //不是vector<vector<int> > adjlist; ，而是存edge在edgelist的index
vector<vector<LG_edge> > levelgraph;
vector<vector<LG_edge> > rlevelgraph;   //reverse levelgraph

vector<int> level;


vector<bool> blocked;
vector<int> excess;
vector<int> Torder; //topological order


queue<int> bfsqueue;    //儲存bfs待執行的點
void bfs(int x){
    vector<int>::iterator it;// 尋訪
    for(it=adjlist[x].begin(); it!=adjlist[x].end(); ++it){
        int index=(*it);
        edge e = edgelist[index];
        if (e.cap > e.flow && level[e.b] > level[x]){ //level[e.b] > level[x]可能是level[e.b]=level[x]+1或level[e.b]=INF
            levelgraph[x].push_back(LG_edge( e.a,e.b,e.cap-e.flow,0, index ,rlevelgraph[e.b].size())); //levelgraph上邊的cap為原圖的residual cap
            rlevelgraph[e.b].push_back(LG_edge( e.b,e.a,e.flow,0, index^1 ,levelgraph[x].size()-1));
            if (level[e.b] == INF)      //不讓e.b重複加入queue
            {
                bfsqueue.push(e.b);
                Torder.push_back(e.b);
                //levelgraphNodes++;
            }
            level[e.b]=level[x]+1;
        }
    };

    return;
}



vector< vector<LG_edge>::iterator > LG_it; //levelgraph it
bool IncreaseFlow()
{
    bool needDecreaseFlow=false;
    vector<int>::iterator it;// 尋訪
    vector<LG_edge>::iterator it2;// 尋訪

    for(it=Torder.begin(); it!=Torder.end(); ++it){
        int x=(*it);
        //cout<< x<<" "<<endl;
        if (blocked[x]==false && excess[x]>0 && x!=t){
            //vector<LG_edge>::iterator it2;
            for(std::vector<LG_edge>::size_type i = 0; (i != levelgraph[x].size() && excess[x]>0 ); i++) {

                int coEdge =levelgraph[x][i].corr_edge;
                int residual_cap = levelgraph[x][i].cap-levelgraph[x][i].flow;        //在levelgraph流
                int y=levelgraph[x][i].b;
                if (residual_cap > 0 && blocked[y]==false ){
                    int pushed = min(excess[x],residual_cap);
                    excess[x]-=pushed;
                    excess[y]+=pushed;
                    levelgraph[x][i].flow+=pushed;
                    rlevelgraph[y][levelgraph[x][i].redge].flow-=pushed;
                    edgelist[coEdge].flow +=pushed;
                    edgelist[coEdge^1].flow -=pushed;
                }
            }
            if (excess[x]>0){
                blocked[x]=true;
                //cout<< x<<" ";
                if (x!=s && x!=t) needDecreaseFlow=true;
            }
        }
    }
    return needDecreaseFlow;
}

vector< vector<LG_edge>::iterator > rLG_it; //rlevelgraph it
bool DecreaseFlow()
{
    bool needIncreaseFlow=false;
    vector<int>::reverse_iterator rit;// 尋訪
    vector<LG_edge>::iterator it2;// 尋訪 rlevelgraph

    for(rit=Torder.rbegin(); rit!=Torder.rend(); ++rit){   //reverse topological order
        int x=(*rit);

        //cout <<blocked[1017] << excess[1017] <<endl;
        if (blocked[x]==true && (excess[x]>0) && x!=s ){
            //vector<LG_edge>::iterator it2;
            for(std::vector<LG_edge>::size_type i = 0; (i != rlevelgraph[x].size() && excess[x]>0 ); i++) {
                int coEdge =rlevelgraph[x][i].corr_edge;     //reverse edge: x->edgelist[coEdge].b
                int w = rlevelgraph[x][i].b;     //reverse edge: x->w
                int pushedflow = - rlevelgraph[x][i].flow;
                if ( pushedflow > 0 ){

                    int pushback = min(excess[x], pushedflow );
                    excess[x]-=pushback;
                    excess[w]+=pushback;
                    rlevelgraph[x][i].flow +=pushback;
                    levelgraph[w][rlevelgraph[x][i].redge].flow -=pushback;
                    edgelist[coEdge].flow +=pushback;
                    edgelist[coEdge^1].flow -=pushback;
                }
                if (excess[w]>0 && blocked[w]==false && w!=t){
                    needIncreaseFlow=true;
                }
            }
        }
    }
    return needIncreaseFlow;
}
void clear( std::vector<LG_edge> &q )    //clear a vector
{
   std::vector<LG_edge> empty;
   std::swap( q, empty );
}
void levelgraphClear(){
    for(int i=0;i<=n;i++){  //release memory
        //levelgraph[i].clear();
        clear(levelgraph[i]);
        clear(rlevelgraph[i]);
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
        Torder.clear(); //init
        bfsqueue.push(s);
        //Torder.push_back(s); //dont need
        while(!bfsqueue.empty() && level[t] > level[bfsqueue.front()]){   //若level[t] = level[bfsqueue.front()，代表levelgraph以建立完t點那層
            bfs( bfsqueue.front());
            bfsqueue.pop();
        }

        cout<< "now levelgraph level = " << level[t] << endl;
        cout<< "now flow = " << flow << endl;
        if (level[t]==INF) break;   //無法到達t

        //開始計算blocking flow
        cout<< "computing blocking flow.. " << endl;
        int IncreasedFlowtimes=0;
        int DecreasedFlowtimes=0;

        for(int i=0;i<=n;i++){
            excess[i]=0;
            blocked[i]=false;
        }
        blocked[s]=true;
        int sum_pushed=0;
        vector<LG_edge>::iterator it2;  //init
        for(it2=levelgraph[s].begin();it2!=levelgraph[s].end() ; ++it2){
            int coEdge =it2->corr_edge;
            int residual_cap = edgelist[coEdge].cap - edgelist[coEdge].flow;

            int pushed = residual_cap;
            excess[edgelist[coEdge].b]+=pushed;
            it2->flow =it2->flow + pushed;
            rlevelgraph[it2->b][it2->redge].flow =rlevelgraph[it2->b][it2->redge].flow - pushed;
            edgelist[coEdge].flow +=pushed;
            edgelist[coEdge^1].flow -=pushed;
            sum_pushed+=pushed;

        }

        while(1){
            //if (IncreaseFlow()==false) break;
            IncreaseFlow();
            IncreasedFlowtimes++;
            //if (DecreaseFlow()==false) break;
            DecreaseFlow();
            DecreasedFlowtimes++;
            if (sum_pushed==(excess[t]+excess[s])) break;
            //cout<<sum_pushed<<" "<<excess[t]<<" "<<excess[s]<<endl;

        }
        for(int i=0;i<=n;i++){
            if (excess[i]>0){
                cout<<i<<" "<<blocked[i]<<" "<<endl;
            }

        }
        flow =flow + excess[t];
        cout<<"excess[s]="<< excess[s] <<" excess[t]="<<excess[t]<<endl;
        cout<< "IncreasedFlowtimes = " << IncreasedFlowtimes << endl;
        cout<< "DecreasedFlowtimes = " << DecreasedFlowtimes << endl;
        //cout<< "arcdo = " << arcdo << endl;
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
            rlevelgraph.resize(n+1);
            excess.resize(n+1);
            blocked.resize(n+1);
            LG_it.resize(n+1);
            rLG_it.resize(n+1);
            level.resize(n+1);

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
