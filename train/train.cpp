//
//  main.cpp
//  child
//
//  Created by Zesheng Jia on 2018-09-24.
//  Copyright © 2018 Zesheng Jia. All rights reserved.
//


#include <iostream>
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
#include <array>
#include <ctime>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <chrono>

using namespace std::chrono;

// Define named semaphore
#define SEM_JUNCTION "/semaphore_junction"
#define SEM_READ_MATRIX "/semaphore_read_matrix"
#define SEM_NORTH "/semaphore_north"
#define SEM_WEST "/semaphore_west"
#define SEM_SOUTH "/semaphore_south"
#define SEM_EAST "/semaphore_east"

using namespace std;

// child PID
int ID ;

// train direction
char direction;
// train right_side_direction
char right_side_direction;

// all direction
char all_dir[4] = {'N' , 'W' , 'S' , 'E'};
// corresponding right side
// (index + 1) % 4
// train info
string train_info[4] = {"North" , "West" , "South" , "East"};
// lock info
string lock_info[4] = {"North_Lock" , "West_Lock" , "South_Lock" , "East_Lock"};
// train index
int index_direction = 0;
int index_right_side_direction = 0;

// get semaphores
sem_t *sem_junction;
sem_t *sem_matrix;
sem_t *sem_direction;
sem_t *sem_right_side_direction;


// matrix file name
string matrix_file_path = "/Users/WillJia/Documents/DeadLockDetection/manager/matrix.txt";


// matrix
vector<vector<int>> matrix ;

// output stream
ofstream outfile;
/************************************************************************
 
 
 CHILDREN FUNCTIONS
 
 
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
 
 Function:        readingArray
 
 Description:     whole procedure of reading array
 
 *************************************************************************/
void readingArray(){
    
    /* reading array */
    matrix = processFile(matrix_file_path);
}

/************************************************************************
 
 Function:        get_right_side_direction
 
 Description:     get ID from **argv
 
 *************************************************************************/
char get_right_side_direction(){
    for (int i = 0; i < 4; i++) {
        if (all_dir[i] == direction) {
            return all_dir[(i + 1) % 4];
        }
    }
    return NULL;
}

/************************************************************************
 
 Function:        getID
 
 Description:     get ID from **argv
 
 *************************************************************************/
void get_arguments(const char * argv[]){
    ID = atoi(argv[0]);
    direction = argv[1][0];
    right_side_direction = get_right_side_direction();
}

/************************************************************************
 
 Function:        get_direction_semaphores
 
 Description:     get_direction_semaphores and index
 
 *************************************************************************/


int get_direction_semaphores(char char_dir){
    
    int index = 0;
    // according to the Argv from parent process to choose which semaphore to use
    switch (char_dir) {
        case 'N':
            index = 0;
            sem_direction = sem_open(SEM_NORTH, O_RDWR);
            break;
        case 'W':
            index = 1;
            sem_direction = sem_open(SEM_WEST, O_RDWR);
            break;
        case 'S':
            index = 2;
            sem_direction = sem_open(SEM_SOUTH, O_RDWR);
            break;
        case 'E':
            index = 3;
            sem_direction = sem_open(SEM_EAST, O_RDWR);
            break;
        default:
            cout << "wrong!!!" << endl;
            break;
    }
    
    return index;
}

/************************************************************************
 
 Function:        get_right_side_direction_semaphores
 
 Description:     get_right_side_direction_semaphores and index
 
 *************************************************************************/

int get_right_side_direction_semaphores( char char_dir){
    
    int index = 0;
    // according to the Argv from parent process to choose which semaphore to use
    switch (char_dir) {
        case 'N':
            index = 0;
            sem_right_side_direction = sem_open(SEM_NORTH, O_RDWR);
            break;
        case 'W':
            index = 1;
            sem_right_side_direction = sem_open(SEM_WEST, O_RDWR);
            break;
        case 'S':
            index = 2;
            sem_right_side_direction = sem_open(SEM_SOUTH, O_RDWR);
            break;
        case 'E':
            index = 3;
            sem_right_side_direction = sem_open(SEM_EAST, O_RDWR);
            break;
        default:
            cout << "wrong!!!" << endl;
            break;
    }
    
    return index;
}

/************************************************************************
 
 Function:        get_semaphores
 
 Description:     get_semaphores
 
 *************************************************************************/
void get_semaphores(){
    // get 4 semaphores
    // get junction and matrix semaphore
//    sem_junction = sem_open(SEM_JUNCTION, O_RDWR);
    sem_junction = sem_open(SEM_JUNCTION, O_RDWR);
    sem_matrix = sem_open(SEM_READ_MATRIX,O_RDWR);
    
    // get specific train semaphore and its right side semaphore
    index_direction = get_direction_semaphores(direction);
    index_right_side_direction = get_right_side_direction_semaphores(right_side_direction);
    
    // check semaphore
    if (sem_junction == SEM_FAILED ){
        perror("Child sem_open(sem_junction) failed");
        exit(EXIT_FAILURE);
        
    }
    if (sem_matrix == SEM_FAILED ){
        perror("Child sem_open(sem_matrix) failed");
        exit(EXIT_FAILURE);
    }
    if (sem_direction == SEM_FAILED ){
        perror("Child sem_open(sem_direction) failed");
        exit(EXIT_FAILURE);
    }
    if (sem_right_side_direction == SEM_FAILED ){
        perror("Child sem_open(sem_right_side_direction) failed");
        exit(EXIT_FAILURE);
    }
    
}
/************************************************************************
 
 Function:        print_matrix
 
 Description:     print_matrix
 
 *************************************************************************/
void print_matrix(vector<vector<int>> matrix){
    // print matrix
    printf("Train<pid%d>:\n" , ID);
    printf("Direction train: %c\n" , direction);
    printf("Right Side Direction train: %c\n", right_side_direction);
    printf("{");
    
    for (int i = 0; i < matrix.size(); i++)
    {
        printf("{");
        for (int j = 0; j < matrix[i].size(); j++)
        {
            printf("%d",  matrix[i][j]);
            if (j < 3) {
                printf(",");
            }
        }
        printf("} , \n");
    }
    printf("};");
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
 
 Function:        update_matrix
 
 Description:     update_matrix
 
 *************************************************************************/

void update_matrix(int i ,int j , int value){
    
    sem_wait(sem_matrix);
    
    readingArray();
    matrix[i][j] = value;
    open_output_file(matrix_file_path);
    for (int i = 0; i < matrix.size(); i++) {
        for (int j = 0; j < matrix[i].size(); j++) {
            outfile << matrix[i][j] << " ";
        }
        outfile << endl;
    }
    
    close_output_file();
    
//    print_matrix(matrix);
    sem_post(sem_matrix);
    
}


/************************************************************************
 
 Function:        main
 
 Description:      main function
 
 *************************************************************************/
int main(int argc, const char * argv[]) {
    

    srand((int)time(NULL));
    
    //Done : change matrix to vector<int>
    readingArray();
    
    // get arguments by argv
    get_arguments(argv);
    
    //get semaphores
    get_semaphores();
    
    // waits direction semaphore

    printf("\nTrain<pid%d> successfully created and %s Train Started\n" , ID, train_info[index_direction].c_str());
    
    // request for direction lock
    printf("\nTrain<pid%d>: requests for %s \n", ID, lock_info[index_direction].c_str());
    
    // upadte matrix
    update_matrix(ID - 1, index_direction, 1);
    
    sem_wait(sem_direction);
    
    // acquire for direction lock
    printf("\nTrain<pid%d>: acquires for %s\n", ID, lock_info[index_direction].c_str());
    
    // upadte matrix
    update_matrix(ID - 1, index_direction, 2);
    

    // request for right side lock
    printf("\nTrain<pid%d>: requests for %s \n", ID, lock_info[index_right_side_direction].c_str());
    
    // upadte matrix
    update_matrix(ID - 1, index_right_side_direction, 1);
    
    // acquire for direction lock
    sem_wait(sem_right_side_direction);
    
    // upadte matrix
    update_matrix(ID - 1, index_right_side_direction, 2);
    printf("vTrain<pid%d>: acquires for %s\n", ID, lock_info[index_right_side_direction].c_str());
    
    // request for junction lock
    printf("\nTrain<pid%d>: requests Junction_Lock\n", ID);
    
    sem_wait(sem_junction);
    printf("\nTrain<pid%d>: Acquires Junction_Lock; Passing Junction\n", ID);
    sleep(2);
    
    
    // release junction lock
    printf("\nTrain<pid%d>: releases Junction_Lock\n", ID);
    sem_post(sem_junction);
    
    // release direction lock
    printf("\nTrain<pid%d>: releases %s\n", ID, lock_info[index_direction].c_str());
    sem_post(sem_direction);
    
    // upadte matrix
    update_matrix(ID - 1, index_direction, 0);

    // release right side direction lock
    printf("\nTrain<pid%d>: releases %s\n", ID, lock_info[index_right_side_direction].c_str());
    sem_post(sem_right_side_direction);
    
    // upadte matrix
    update_matrix(ID - 1, index_right_side_direction, 0);
    
}
