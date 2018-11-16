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

#define CHILD_PROGRAM "/Users/WillJia/Desktop/IOS Lecture/Projects/DeadLockDetection/train/train"

// nxm matrix
#define M 4

// nxm matrix
int N = 0;

// output stream
ofstream outfile;

// sequence file name
string sequence_file_path = "/Users/WillJia/Desktop/IOS Lecture/Projects/DeadLockDetection/manager/sequence.txt";

// matrix file name
string matrix_file_path = "/Users/WillJia/Desktop/IOS Lecture/Projects/DeadLockDetection/manager/matrix.txt";


/************************************************************************
 
 
 PARENT FUNCTIONS
 
 
 *************************************************************************/

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
    if (sem_unlink(SEM_JUNCTION)&& sem_unlink(SEM_READ_MATRIX)&&
        sem_unlink(SEM_NORTH)&& sem_unlink(SEM_WEST)&&
        sem_unlink(SEM_SOUTH)&& sem_unlink(SEM_EAST)){
        if(detect_failed)perror("sem_unlink(3) failed");
    }
}

/************************************************************************
 
 Function:        create_name_semaphore
 
 Description:     generate 6 semaphores into file to make sharing semaphore happen
 
 *************************************************************************/

void create_name_semaphore(){
    // initialize basic semaphores
    /* We initialize the semaphore counter to 1 (INITIAL_VALUE) */
    sem_t *sem_junction = sem_open(SEM_JUNCTION, O_CREAT | O_EXCL, SEM_PERMS, INITIAL_VALUE);
    sem_t *sem_matrix = sem_open(SEM_READ_MATRIX, O_CREAT | O_EXCL, SEM_PERMS, INITIAL_VALUE);
    sem_t *sem_north = sem_open(SEM_NORTH, O_CREAT | O_EXCL, SEM_PERMS, INITIAL_VALUE);
    sem_t *sem_west = sem_open(SEM_WEST, O_CREAT | O_EXCL, SEM_PERMS, INITIAL_VALUE);
    sem_t *sem_south = sem_open(SEM_SOUTH, O_CREAT | O_EXCL, SEM_PERMS, INITIAL_VALUE);
    sem_t *sem_east = sem_open(SEM_EAST, O_CREAT | O_EXCL, SEM_PERMS, INITIAL_VALUE);
    
    // check creation status
    if (sem_junction == SEM_FAILED && sem_matrix == SEM_FAILED && sem_north == SEM_FAILED && sem_west == SEM_FAILED && sem_south == SEM_FAILED && sem_east == SEM_FAILED) {
        perror("sem_open(3) error");
        exit(EXIT_FAILURE);
    }
    
    /* Close the semaphore as we won't be using it in the parent process */
    if (sem_close(sem_junction) < 0 && sem_close(sem_matrix) < 0 && sem_close(sem_north) < 0 && sem_close(sem_west) < 0 && sem_close(sem_south) < 0 && sem_close(sem_east) < 0) {
        perror("sem_close(3) failed");
        /* We ignore possible sem_unlink(3) errors here */
        unlink_semaphores(false);
        exit(EXIT_FAILURE);
    }
}




int main(int argc, const char * argv[]) {
    

    
    // Read data from sequence.txt file
    string sequence_list = read_sequence_file(sequence_file_path);
    N = (int)sequence_list.size();
    cout << "Number of trains = " << N << endl;
    
    // output sequence content
    for (int i = 0; i < N ; i++) {
        cout << sequence_list[i] << " ";
    }
    cout << endl;
    
    // initialize matrix file
    initialize_matrix();
    
    
    // start creating child process
    pid_t pids[N];
    int i ;
    
    for (i = 0; i < N ; i++) {
        
        // TODO: check deadlock by probability
        
        
        // ELSE: generate new train process
        
        if ((pids[i] = fork()) < 0) {
            perror("fork(2) failed");
            exit(EXIT_FAILURE);
        }
        // generate pid from 1
        std::string s = std::to_string(i + 1);
        char const *PID = s.c_str();
        
        if (pids[i] == 0) {
            if (execlp(CHILD_PROGRAM, PID, sequence_list[i] ,NULL) < 0) {
                perror("execl(2) failed");
                exit(EXIT_FAILURE);
            }
        }
    }
    
    for (i = 0; i < N ; i++)
        if (waitpid(pids[i], NULL, 0) < 0)
            perror("waitpid(2) failed");
    
    unlink_semaphores(true);
    
    return 0;
}
