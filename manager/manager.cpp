//
//  main.cpp
//  manager
//
//  Created by Zesheng Jia on 2018-11-15.
//  Copyright © 2018 Zesheng Jia. All rights reserved.
//


#include <iostream>
#include <algorithm>    // std::sort
#include <fstream>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <string.h>
#include <unistd.h>
#include <string.h>
#include <fstream>
#include <vector>
#include <sstream>
#include <cstdlib>
#include <ctime>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <vector>
#include <tuple>
#include <stack>
#include <map>

/************************************************************************************
 
 File:             DeadLock Detection
 
 Description:      Using named semaphore + processes and RAG technique to detect deadlock
 
 Author:           Zesheng Jia A00416452
 
 *************************************************************************************/

using namespace std;

// Define named semaphore
#define SEM_JUNCTION "/semaphore_junction"
#define SEM_READ_MATRIX "/semaphore_read_matrix"
#define SEM_NORTH "/semaphore_north"
#define SEM_WEST "/semaphore_west"
#define SEM_SOUTH "/semaphore_south"
#define SEM_EAST "/semaphore_east"

#define SEM_PERMS (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP)
#define INITIAL_VALUE 1

//string CHILD_PROGRAM = "./train";
string CHILD_PROGRAM = "/Users/WillJia/Documents/DeadLockDetection/manager/train";


// nxm matrix
#define M 4

// nxm matrix
int N = 0;

// output stream
ofstream outfile;

// sequence file name
string sequence_file_path = "/Users/WillJia/Documents/DeadLockDetection/manager/sequence.txt";

// matrix file name
string matrix_file_path = "/Users/WillJia/Documents/DeadLockDetection/manager/matrix.txt";

// declare sem_t
sem_t *sem_junction;
sem_t *sem_matrix;
sem_t *sem_north;
sem_t *sem_west;
sem_t *sem_south;
sem_t *sem_east;

// check deadlock parameters
int V, E;
int n;

//带权有向图
map<int, vector<tuple<int , int , double>>> EWD;

bool marked[100];   // v 是否已经被访问过？
bool onStack[100];  // v 是否在栈里？
tuple<int , int , double> edgeTo[100]; // 到达顶点 v 的最后一条边
stack<tuple<int , int , double>> cycle[100];    // 有向环

// train info
string train_info[4] = {"North" , "West" , "South" , "East"};


/************************************************************************
 
 
 PARENT FUNCTIONS
 
 
 *************************************************************************/
/************************************************************************
 
 Function:        num_2_char
 
 Description:     change number to char for passing value in pipe
 
 *************************************************************************/


char const* num_2_char(int n ){
    std::string s = std::to_string(n);
    char const *pchar = s.c_str();
    return pchar;
}

/************************************************************************
 
 Function:        split
 
 Description:     spilt line into elements
 
 *************************************************************************/


std::vector<std::string> &split(const std::string &s, char delim, std::vector<std::string> &elems) {
    std::stringstream ss(s);
    std::string item;
    while(std::getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}
// https://stackoverflow.com/questions/275404/splitting-strings-in-c


/************************************************************************
 
 Function:        arrayConvert
 
 Description:     convert string to array
 
 *************************************************************************/

vector<int> arrayConvert(string content){
    
    vector<int> array;
    vector<string> numbers ;
    split(content, ' ', numbers);
    int i = 0;
    for(string n : numbers){
        stringstream geek(n);
        geek >> i;
        array.push_back(i);
    }
    return array;
}
//https://www.geeksforgeeks.org/converting-strings-numbers-cc/

/************************************************************************
 
 Function:        readContent
 
 Description:     read content from file
 
 *************************************************************************/

vector<vector<int>> readContent(string filename){
    string content;
    string line;
    ifstream myfile (filename);
    vector<vector<int>> array;
    
    if (myfile.is_open()) {
        while (getline(myfile,line)) {
            array.push_back(arrayConvert(line));
        }
    }
    myfile.close();
    return array;
}


/************************************************************************
 
 Function:        processFile
 
 Description:     read file and store in array
 
 *************************************************************************/

vector<vector<int>> processFile(string filename){
    return readContent(filename);
}

/************************************************************************
 
 Function:        open_output_file
 
 Description:     open output file
 
 *************************************************************************/
void open_output_file(string file_path){
    outfile.open(file_path);
}


/************************************************************************
 
 Function:        close_output_file
 
 Description:     close output file
 
 *************************************************************************/
void close_output_file(){
    outfile.close();
}


/************************************************************************
 
 Function:        read_sequence_file
 
 Description:     Read sequenced list from sequence.txt
 
 *************************************************************************/
string read_sequence_file(string file_path){
    string content;
    string line;
    ifstream myfile (file_path);
    if (myfile.is_open()) {
        while (getline(myfile,line)) {
            content.append(line);
        }
    }
    myfile.close();
    return content;
}


/************************************************************************
 
 Function:        initilize_matrix
 
 Description:     Initialize matrix.txt
 
 *************************************************************************/
void initialize_matrix(){
    
    open_output_file(matrix_file_path);
    for (int i = 0; i < N; i++) {
        for (int j = 0; j < M; j++) {
            outfile << "0" << " ";
        }
        outfile << endl;
    }
    close_output_file();
}

/************************************************************************
 
 Function:        unlink_semaphores
 
 Description:     unlink all semaphores
 
 *************************************************************************/
void unlink_semaphores(bool detect_failed){
    sem_unlink(SEM_JUNCTION);
    sem_unlink(SEM_READ_MATRIX);
    sem_unlink(SEM_NORTH);
    sem_unlink(SEM_WEST) ;
    sem_unlink(SEM_SOUTH) ;
    sem_unlink(SEM_EAST);
    
    if (sem_unlink(SEM_JUNCTION) && sem_unlink(SEM_READ_MATRIX) &&
        sem_unlink(SEM_NORTH) && sem_unlink(SEM_WEST) &&
        sem_unlink(SEM_SOUTH) && sem_unlink(SEM_EAST)){
        if(detect_failed)perror("sem_unlink(3) failed");

    }
    
}

/************************************************************************
 
 Function:        create_name_semaphore
 
 Description:     generate 6 semaphores into file to make sharing semaphore happen
 
 *************************************************************************/



void create_name_semaphore(){
//    unlink_semaphores(true);
    // initialize basic semaphores
    /* We initialize the semaphore counter to 1 (INITIAL_VALUE) */
     sem_junction = sem_open(SEM_JUNCTION, O_CREAT | O_EXCL, SEM_PERMS, INITIAL_VALUE);
     sem_matrix = sem_open(SEM_READ_MATRIX, O_CREAT | O_EXCL, SEM_PERMS, INITIAL_VALUE);
     sem_north = sem_open(SEM_NORTH, O_CREAT | O_EXCL, SEM_PERMS, INITIAL_VALUE);
     sem_west = sem_open(SEM_WEST, O_CREAT | O_EXCL, SEM_PERMS, INITIAL_VALUE);
     sem_south = sem_open(SEM_SOUTH, O_CREAT | O_EXCL, SEM_PERMS, INITIAL_VALUE);
     sem_east = sem_open(SEM_EAST, O_CREAT | O_EXCL, SEM_PERMS, INITIAL_VALUE);
    
    // check creation status
    if (sem_junction == SEM_FAILED && sem_matrix == SEM_FAILED && sem_north == SEM_FAILED && sem_west == SEM_FAILED && sem_south == SEM_FAILED && sem_east == SEM_FAILED) {
        perror("sem_open(3) error");
        exit(EXIT_FAILURE);
    }


}

/************************************************************************
 
 Function:        print_matrix
 
 Description:     print_matrix
 
 *************************************************************************/
void print_matrix(vector<vector<int>> matrix){
    printf("--------------------------------------------------\n");
    printf("            Semaphores          \n");
    printf("             North   West    South   East    \n");
    printf("--------------------------------------------------\n");

    for (int i = 0; i < matrix.size(); i++)
    {
        printf("Train<pid%d>    {" , i + 1);
        for (int j = 0; j < matrix[i].size(); j++)
        {
            printf("%d",  matrix[i][j]);
            if (j < 3) {
                printf(",");
            }
        }
        printf("} , \n");
    }
    printf("\n");
}

/************************************************************************
 
 Function:        dfs
 
 Description:     dfs
 
 *************************************************************************/
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

void show_data() {
    cout << "EdgeWeightedDigraph : " << endl;
    for(int v = 0; v < V; v++)
    {
        cout << v << " : ";
        for(vector<tuple<int, int, double>>::iterator ii = EWD[v].begin(); ii != EWD[v].end(); ii++)
            cout << get<0>(*ii) << "->" << get<1>(*ii) << " " << get<2>(*ii) << " ";
        cout << endl;
    }
}


/************************************************************************
 
 Function:        find_cycle
 
 Description:    find cycle in graph
 *************************************************************************/

void find_cycle() {
    for(int v = 0 ; v < V; v++)
        if(!marked[v]) dfs(v);
}

/************************************************************************
 
 Function:        show_cycle
 
 Description:     show found cycle
 
 *************************************************************************/
void show_cycle() {
    int detect_deadlock_flag = 0;
    for(int i = 0 ; i < n;i++)
    {
        int index = 0;
        string train_info_first;
        int index_first = 0 ;
        if(!cycle[0].empty()) detect_deadlock_flag = 1;
        if (detect_deadlock_flag) {
            detect_deadlock_flag = 1;
             printf("*************************************************\n\n");
            printf(" DeadLock detected!!!!!!!\n\n");
            printf("*************************************************\n\n");

        }
        
        vector<string> string_cycle;
        
        while(!cycle[i].empty()) {
            tuple<int, int, double> f = cycle[i].top();
            
            string first_string, second_string;
            
            
            if (get<0>(f) < 4) {
                first_string = "[ " + train_info[get<0>(f)] + " ] ";
            }else{
                first_string = "( Tran<pid" + to_string(get<0>(f) - 3) +"> )" ;
            }
            
            if (get<1>(f) < 4) {
                second_string = "[ " + train_info[get<1>(f)] + "  ] ";
            }else{
                second_string = "( Tran<pid" + to_string(get<1>(f) - 3) +"> )" ;

            }
            
            string_cycle.push_back(first_string  + "                ->               " + second_string + "\n" );
            
        
            if (get<2>(f) == 2) {
                printf("-Train<pid%d> from %s " , get<1>(f) - 3, train_info[get<0>(f)].c_str());
                
                if (index != 0) {
                    printf("\n");
                }
                if (index == 0) {
                    index_first = get<1>(f) - 3;
                    train_info_first = train_info[get<0>(f)];
                    printf("is waiting for ");
                }
                
                if (index!= 0){
                    printf("-Train<pid%d> from %s " ,  get<1>(f)  - 3, train_info[get<0>(f)].c_str());
                    
                    printf("is waiting for "  );
                }
                index++;
                
            }
            
            cycle[i].pop();
        }
        printf("-Train<pid%d> from %s \n" ,index_first , train_info_first.c_str());
        
        printf("\n\n*************************************************\n\n");
        printf("RAG grahp : \n");
        printf("\n\n*************************************************\n");

        for(string item : string_cycle){
            printf("%s" , item.c_str());
        }
    }
    
    if (detect_deadlock_flag == 1) {
//        printf("DeadLock detected!!!!!!!\n");
        unlink_semaphores(false);
        exit(1);
    }
}

/************************************************************************
 
 Function:        read_data
 
 Description:     read_data from matrix.txt
 
 *************************************************************************/

void read_data() {
    V = N + M;
    
    sem_wait(sem_matrix);
    vector<vector<int>> array = readContent(matrix_file_path);
    print_matrix(array);
   
    
    
    for(int i = 0 ; i < array.size(); i++){
        for (int j = 0;j < array[i].size() ;j++) {
            if (array[i][j] == 2) {
                //                printf("j %d -> i %d\n" , j , i + 4 );
                EWD[j].push_back(make_tuple(j, i+4, 2));
                E++;
            }else if (array[i][j] == 1) {
                //                printf("i %d -> j %d\n" , i + 4 , j );
                EWD[i+4].push_back(make_tuple(i+4, j, 1));
                E++;
            }
        }
    }
    sem_post(sem_matrix);
}


/************************************************************************
 
 Function:        check_deadlock
 
 Description:     check_deadlock
 
 *************************************************************************/
void check_deadlock(){
    read_data();
    show_data();
    find_cycle();
    show_cycle();
    EWD.clear();
    for (int i = 0; i < 100; i++) {
        marked[i] = false;
        onStack[i] = false;
        while (!cycle[i].empty()){
            cycle[i].pop();
        }
    }

}


int main(int argc, const char * argv[]) {
    
//   sem_close(sem_junction) ;
//    sem_close(sem_matrix) ;
//    sem_close(sem_north) ;
//    sem_close(sem_west) ;
//    sem_close(sem_south) ;
//    sem_close(sem_east);

    
    float p = atof(argv[1]);
    
    srand((int)time(NULL));
    
    unlink_semaphores(false);
    
    // Read data from sequence.txt file
    string sequence_list = read_sequence_file(sequence_file_path);
    
    N = (int)sequence_list.size();
    
    printf("Number of trains = %d\n" , N);
    
    // output sequence content
    for (int i = 0; i < N ; i++) {
        printf("%c ", sequence_list[i]);
    }
    printf("\n");
    
    // initialize matrix file
    initialize_matrix();
    
    create_name_semaphore();
    
    // start creating child process
    pid_t pids[N];
    int i = 0;
    
    while(1){
        
        // TODO: check deadlock by probability
        float r = (rand() % 10) / 10.0 ;
        if (r < p) {
            check_deadlock();
        }else{
        
            // ELSE: generate new train process
            
        
            if ((pids[i] = fork()) < 0) {
                perror("fork(2) failed");
                exit(EXIT_FAILURE);
            }
            
            
            // generate pid from 1
            std::string s = std::to_string(i + 1);
            char const *PID = s.c_str();
            
            char d = sequence_list[i];
            char const *direction = &d;

            
            if (pids[i] == 0) {
                if (execl(CHILD_PROGRAM.c_str(), PID, direction, NULL) < 0) {
                    perror("execl(2) failed");
                    exit(EXIT_FAILURE);
                }
            }
            i++;
        }
        if (i >= N) break;
        
    }
    
    int flag = 0;
    
    while (1) {
        check_deadlock();
        if (wait(NULL) > 0)
            flag++;
        if (flag >= N) {
            break;
        }
    }
    
    
    printf("\n\n*************************\n\n");
    printf("All trains Successfully passed !!!");
    printf("\n\n*************************\n\n");
    
    unlink_semaphores(true);
    


    return 0;
}
