
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <vector>
#include <queue>
#include <deque>

#include <sstream>
#include <string>
#include <fstream>  //�B�z�ɮ�

using namespace std;

#define INF 2147483647

int n,m;
int s,t;

typedef class edge {   //��Ϫ�edge
public:
	int a, b, cap, flow;
    edge(int _a, int _b, int _cap, int _flow) : a(_a), b(_b), cap(_cap), flow(_flow){}
} edge;

typedef class LG_edge {    //levelgraph��edge
public:
	int a, b;
	//int cap, flow;    //����cap�A�]cap=��Ϫ�cap - flow
    int corr_edge;      //���������edge�bedgelist��index
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
    e.corr_edge = corr_edge;        //levelgraph��edge��������Ϫ�edge
    return e;
}*/

vector<edge> edgelist;  //�s��edge��list
vector<vector<int> > adjlist;       //���Ovector<vector<int> > adjlist; �A�ӬO�sedge�bedgelist��index
vector<vector<LG_edge> > levelgraph;
vector<int> level;

queue<int> bfsqueue;    //�x�sbfs�ݰ��檺�I
void bfs(int x){
    vector<int>::iterator it;// �M�X
    for(it=adjlist[x].begin(); it!=adjlist[x].end(); ++it){
        int index=(*it);
        edge e = edgelist[index];
        if (e.cap > e.flow && level[e.b] > level[x]){ //level[e.b] > level[x]�i��Olevel[e.b]=level[x]+1��level[e.b]=INF
            levelgraph[x].push_back(LG_edge( e.a,e.b, index ,true)); //levelgraph�W�䪺cap����Ϫ�residual cap
            if (level[e.b] == INF)      //����e.b���ƥ[�Jqueue
            {
                bfsqueue.push(e.b);
                //levelgraphNodes++;
            }
            level[e.b]=level[x]+1;
        }
    };

    return;
}

//�C��dfs���@��s-t path �Y�^���^s�I�A�W�a���|�W���y�q
// �blevelgraph�W�j�M�A�䧹blocking flow�����׬OO(nm)
//�]�L�k��Ft��edge��L�|�Q�R���A���h����s-t path���L�{�A�h��dfs�����פ��OO(m)
//�̦hm�� s-t path(�]path����bottleneck arc)�A�C�����path�HO(n)�ɶ��B�z(�M��+�^��)

vector< vector<LG_edge>::iterator > its;// �M�X
long long int arcdo;
int dfs(int x,int bottleneck){
    if (x==t){  //���path
        return bottleneck; //�^�ǭn�W�[��flow��
    }
    //vector<LG_edge>::iterator & it = its[x];// �M�X
    vector<LG_edge>::iterator it;
    for(it=levelgraph[x].begin(); it!=levelgraph[x].end(); ++it){
        if (it->valid==false) continue;
        arcdo++;
        int coEdge =it->corr_edge;
        int residual_cap = edgelist[coEdge].cap - edgelist[coEdge].flow;
        if (residual_cap == 0){ //��wsaturate(�W�@����쪺���|��bottleneck),�R��
            it->valid=false;
            //it=levelgraph[x].erase(it);   //�o�W�C...
            //--it;
            continue;
        }

        int pushes=dfs(it->b,min(bottleneck,residual_cap));
        if (pushes>0){  //�w���path�A���b�^��
            edgelist[coEdge].flow = edgelist[coEdge].flow + pushes; //��Ϫ�edge�]�n�W�[�y�q
            edgelist[(coEdge)^1].flow = edgelist[(coEdge)^1].flow - pushes; //�ϦV��y�q
            return pushes; //�^���A�^�ǭn�W�[��flow��
        }else{  //�S���path�A�R��levelgraph�W����
            it->valid=false;
            //it=levelgraph[x].erase(it);   //�o�W�C...
            //--it;
            continue;
        }

    };
    //�S���s-t path�A�^��0
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
        //�}�l�p��levelgraph
        clear(bfsqueue); //init
        bfsqueue.push(s);
        while(!bfsqueue.empty() && level[t] > level[bfsqueue.front()]){   //�Ylevel[t] = level[bfsqueue.front()�A�N��levelgraph�H�إߧ�t�I���h
            bfs( bfsqueue.front());
            bfsqueue.pop();
        }

        clear(bfsqueue); //init
        cout<< "now levelgraph level = " << level[t] << endl;
        cout<< "now flow = " << flow << endl;
        if (level[t]==INF) break;   //�L�k��Ft

        //�}�l�p��blocking flow
        cout<< "computing blocking flow.. " << endl;
        int dfstimes=0;
        arcdo=0;

        for(int i=0;i<=n;i++){
            its[i]=levelgraph[i].begin();
        }
        while(int pushes= dfs(s,INF)){  //pushes=0�h�w�g�䤣��s-t path
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
        }else if (cha=='p'){  //��l��
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
