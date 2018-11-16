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

#define CHILD_PROGRAM "/Users/WillJia/Desktop/IOS Lecture/Projects/DeadLockDetection/semDemoC/train"



/************************************************************************
 
 
 PARENT FUNCTIONS
 
 
 *************************************************************************/



/************************************************************************
 
 Function:        read_sequence_file
 
 Description:     Read sequenced list from sequence.txt
 
 *************************************************************************/
string read_sequence_file(string filename){
    string content;
    string line;
    ifstream myfile (filename);
    if (myfile.is_open()) {
        while (getline(myfile,line)) {
            content.append(line);
        }
    }
    myfile.close();
    return content;
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
        sem_unlink(SEM_JUNCTION);
        sem_unlink(SEM_READ_MATRIX);
        sem_unlink(SEM_NORTH);
        sem_unlink(SEM_WEST);
        sem_unlink(SEM_SOUTH);
        sem_unlink(SEM_EAST);
        exit(EXIT_FAILURE);
    }
}

int main(int argc, const char * argv[]) {
    
    string filename = "/Users/WillJia/Desktop/IOS Lecture/Projects/DeadLockDetection/manager/sequence.txt";
    // Read data from sequence.txt file
    string sequence_list = read_sequence_file(filename);
    
    cout << "Number of trains = " << sequence_list.length() << endl;
    
    for (int i = 0; i < sequence_list.length(); i++) {
        cout << sequence_list[i] << endl;
    }
    
    
    return 0;
}
