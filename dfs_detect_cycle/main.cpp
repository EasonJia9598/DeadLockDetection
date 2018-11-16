#include <iostream>
#include <vector>
#include <tuple>
#include <stack>
#include <map>
using namespace std;

int V, E;
int n;

//带权有向图
map<int, vector<tuple<int , int , double>>> EWD;

bool marked[100];   // v 是否已经被访问过？
bool onStack[100];  // v 是否在栈里？
tuple<int , int , double> edgeTo[100]; // 到达顶点 v 的最后一条边
stack<tuple<int , int , double>> cycle[100];    // 有向环


void dfs(int v) {
    onStack[v] = true;
    marked[v] = true;
    
    for(vector<tuple<int, int, double>>::iterator ii = EWD[v].begin(); ii != EWD[v].end(); ii++)
    {
        
        int w = get<1>(*ii);
        
        if(!marked[w]) {    // 遇见没访问过的顶点继续dfs递归
            edgeTo[w] = *ii;
            dfs(w);
        }
        else if(onStack[w]) {   // 遇见一个访问过的顶点，并且该顶点在栈里说明发现了一个新环
            tuple<int, int, double> f = *ii;
            while(get<0>(f) != w) {
                cycle[n].push(f);
                f = edgeTo[get<0>(f)];
            }
            
            cycle[n].push(f);
            n++;
            return ;
            
        }
    }
    
    onStack[v] = false;
}

void findCycle() {
    for(int v = 0 ; v < V; v++)
        if(!marked[v]) dfs(v);
}

void showCycle() {
    cout << "Cycle : " << endl;
    for(int i = 0 ; i < n;i++)
    {
        double weight = 0.0;
        while(!cycle[i].empty()) {
            tuple<int, int, double> f = cycle[i].top();
            cout << get<0>(f) << "->" << get<1>(f) << " " << get<2>(f) << endl;
            weight += get<2>(f);
            cycle[i].pop();
        }
        cout << "weight = " << weight << endl << endl;
    }
}


void readData() {
    V = 16;
    int N = 12, M = 4;
//    int array[4][4] = {
//        {2 , 1 , 0 ,0 } ,
//        {0,2,1,0},
//        {0,0,2,1},
//        {1,0,0,2}
//    };
    
    int array[12][4] = {
        {1,0,0,0},
        {0,1,0,0},
        {0,0,0,1},
        {0,0,1,0},
        {1,0,0,0},
        {0,1,0,0},
        {0,0,0,1},
        {0,0,1,0},
        {1,0,0,0},
        {0,1,0,0},
        {0,0,0,1},
        {0,0,1,0}
        
    };
    
    for(int i = 0 ; i < N ; i++){
        for (int j = 0;j < M ;j++) {
            if (array[i][j] == 2) {
                //                printf("j %d -> i %d\n" , j , i + 4 );
                EWD[j].push_back(make_tuple(j, i+4, 1));
                E++;
            }else if (array[i][j] == 1) {
                //                printf("i %d -> j %d\n" , i + 4 , j );
                EWD[i+4].push_back(make_tuple(i+4, j, 1));
                E++;
            }
        }
    }
    
}

void showData() {
    cout << "EdgeWeightedDigraph : " << endl;
    for(int v = 0; v < V; v++)
    {
        cout << v << " : ";
        for(vector<tuple<int, int, double>>::iterator ii = EWD[v].begin(); ii != EWD[v].end(); ii++)
            cout << get<0>(*ii) << "->" << get<1>(*ii) << " " << get<2>(*ii) << " ";
        cout << endl;
    }
    
    system("pause");
}

int main()
{
    readData();
    showData();
    findCycle();
    showCycle();
    
}
