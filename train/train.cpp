//
//  main.cpp
//  child
//
//  Created by WillJia on 2018-09-24.
//  Copyright © 2018 WillJia. All rights reserved.
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

// get semaphores
sem_t *sem_junction;
sem_t *sem_matrix;
sem_t *sem_direction;
sem_t *sem_right_side_direction;

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
 
 Function:        readingArray
 
 Description:     whole procedure of reading array
 
 *************************************************************************/
vector<vector<int>>  readingArray(){
    
    /* reading array */

    vector<vector<int>> array = processFile("/Users/WillJia/Desktop/IOS Lecture/Projects/DeadLockDetection/manager/matrix.txt");
    
    return array;
}

void get_direction_semaphores(sem_t *sem_dir , char char_dir){
    switch (char_dir) {
        case 'N':
            sem_dir = sem_open(SEM_NORTH, O_RDWR);
            break;
        case 'W':
            sem_dir = sem_open(SEM_WEST, O_RDWR);
            break;
        case 'S':
            sem_dir = sem_open(SEM_SOUTH, O_RDWR);
            break;
        case 'E':
            sem_dir = sem_open(SEM_EAST, O_RDWR);
            break;
    }
    
}
/************************************************************************
 
 Function:        get_semaphores
 
 Description:     get_semaphores
 
 *************************************************************************/
void get_semaphores(){
    // get 4 semaphores
    // get junction and matrix semaphore
    sem_junction = sem_open(SEM_JUNCTION, O_RDWR);
    sem_matrix = sem_open(SEM_READ_MATRIX,O_RDWR);
    // get specific train semaphore and its right side semaphore
    get_direction_semaphores(sem_direction , direction);
    get_direction_semaphores(sem_right_side_direction , right_side_direction);
    
    if (sem_junction == SEM_FAILED && sem_matrix == SEM_FAILED &&
        sem_direction == SEM_FAILED && sem_right_side_direction == SEM_FAILED) {
        perror("sem_open(3) failed");
        exit(EXIT_FAILURE);
    }
}
/************************************************************************
 
 Function:        print_matrix
 
 Description:     print_matrix
 
 *************************************************************************/
void print_matrix(vector<vector<int>> matrix){
    
    for (int i = 0; i < matrix.size(); i++)
    {
        for (int j = 0; j < matrix[i].size(); j++)
        {
            cout << matrix[i][j];
        }
    }
}

/************************************************************************
 
 Function:        main
 
 Description:     get_semaphores
 
 *************************************************************************/
int main(int argc, const char * argv[]) {
    
    
    srand((int)time(NULL));
    
    //Done : change matrix to vector<int>
    vector<vector<int>> matrix = readingArray();
    
    print_matrix(matrix);
    
    get_arguments(argv);
    
    //get semaphores
    get_semaphores();
    
    
    
}
