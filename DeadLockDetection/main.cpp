//
//  main.cpp
//  median
//
//  Created by WillJia on 2018-09-23.
//  Copyright © 2018 WillJia. All rights reserved.
//

/************************************************************************************
 
 File:             median
 
 Description:      Using pipe technique to find median in 5 files by 5 child process.
 
 Author:           Zesheng Jia A00416452
 
 *************************************************************************************/



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


using namespace std;

//pipe
#define READ_END 0
#define WRITE_END 1

//signal
#define REQUEST 100         //REQUEST
#define PIVOT 200           //PIVOT
#define LARGE 300           //LARGE
#define SMALL 400           //SMALL
#define READY 500           //READY
#define BREAK 2000          //BREAK
#define TERMINATE 4000  //TERMINATE

//reading start flag
#define beginning_index 1

//sleep time
#define TIME 0



int ID ;
int msgSingal;
int n = 5;


sem_t sem[5];


/************************************************************************
 
 
 PARENT FUNCTIONS
 
 
 *************************************************************************/


/** piping index rules :
 * even number for parent-to-children's write_communication
 * index = id * 2 - 2
 * odd  number for children-to-parent's write_communication
 * index = id * 2 - 1
 */

/************************************************************************
 
 Function:        generateInputFiles
 
 Description:     sleep 10 ms
 
 *************************************************************************/
void generateInputFiles(){
    printf("Genrate new 5 input_* files\n");
    
    srand((int)time(NULL));
    int index = 0 , count = 0;
    vector<int> array;
    
    int numbers[100];
    for (int i = 1; i < 101; i++) {
        numbers[i] = i;
    }
    
    while (true) {
        index = rand() % 100 + 1;
        if (numbers[index] != 0) {
            array.push_back(numbers[index]);
            numbers[index] = 0;
            count++;
        }
        if (count > 25) {
            break;
        }
    }
    
    sort(array.begin(), array.end());
    
    string values[5];
    int stringIndex = 0;
    
    for (int i = 1; i < 26 ;i++){
        
        values[stringIndex].append(to_string(array[i - 1]));
        values[stringIndex].append(" ");
        if (i % 5 == 0) {
            stringIndex++;
        }
        
    }
    
    for (int i = 0; i < 5; i++) {
        printf("%s \n" , values[i].c_str());
    }
    
    string path = "/Users/WillJia/Documents/Pipe/median/input_";
    for(int i = 1 ; i < 5 ; i++){
        ofstream myfile;
        myfile.open(path+to_string(i+1) + ".txt");
        myfile << values[i];
        myfile.close();
        
    }
    
    printf("\n");
    
}
/************************************************************************
 
 Function:        sleep_gap
 
 Description:     sleep 10 ms
 
 *************************************************************************/

void sleep_gap(){
    sleep(0);
}

/************************************************************************
 
 Function:        P2C_write
 
 Description:     parent write to child
 
 *************************************************************************/
void P2C_write(int fds[10][2] , int index , const void *__buf , size_t __nbyte){
    
    close(fds[index * 2 - 2][READ_END]);
    write(fds[index * 2 - 2][WRITE_END], __buf, __nbyte);
}

/************************************************************************
 
 Function:        P_from_C_read
 
 Description:     parent read from child
 
 *************************************************************************/
void P_from_C_read(int fds[10][2] , int index , void *__buf , size_t __nbyte){
    
    close(fds[index * 2 - 1][WRITE_END]);                    // close C2P write_end
    read(fds[index * 2 - 1][READ_END] ,__buf, __nbyte); // start read
    
}


/************************************************************************
 
 Function:        C2P_write
 
 Description:     child write to parent
 
 *************************************************************************/
void C2P_write(int fds[10][2] , int index , const void *__buf , size_t __nbyte){
    
    close(fds[index * 2 - 1][READ_END]);
    write(fds[index * 2 - 1][WRITE_END], __buf, __nbyte);
}

/************************************************************************
 
 Function:        C_from_P_read
 
 Description:     child read from parent
 
 *************************************************************************/
void C_from_P_read(int fds[10][2] , int index , void * __buf , size_t __nbyte){
    
    close(fds[index * 2 - 2][WRITE_END]);                    // close C2P write_end
    read(fds[index * 2 - 2][READ_END] , __buf, __nbyte); // start read
    
}



/************************************************************************
 
 Function:        assignID
 
 Description:     assignID to child process
 
 *************************************************************************/
void assignID(int fds[10][2]){
    for(int i = 1; i < 6 ; i++){
        ID = i;
        P2C_write(fds, 1, &ID, sizeof(ID));
    }
}

/************************************************************************
 
 Function:        p2c_send_signal
 
 Description:     parent send signal to  all child
 
 *************************************************************************/

void p2c_send_signal(int fds[10][2] , int index , int signal ){
    
    std::string s = std::to_string(signal);
    char const *pchar = s.c_str();
    P2C_write(fds, index, pchar, sizeof(pchar));
    
}

/************************************************************************
 
 Function:        waitForReady
 
 Description:     parent wait for all child ready signal
 
 *************************************************************************/
void waitForReady(int fds[10][2]){
    
    int C2P_singal; /* signal container */
    int index[5] = {0}; /* flag for sinal which has been received */
    int flag = 0; /* flag for signal which has not been received */
    char buf[1024];
    /* keep listening until all child process return message. */
    while (true) {
        for (int i = beginning_index ; i < 6 ; i++) {
            printf("**\n");
            memset(buf, 0, sizeof(buf));    // clear buf container
            P_from_C_read(fds, i, &buf, sizeof(buf));
            printf("Array %d contains %s" , i , buf);
            int x = 1280;
            p2c_send_signal(fds, i, x);
            
            
            memset(buf, 0, sizeof(buf));    // clear buf container
            P_from_C_read(fds, i, &buf, sizeof(buf));
            printf("%s\n", buf);
            
            p2c_send_signal(fds, i, x);
            
            
            memset(buf, 0, sizeof(buf));    // clear buffer container
            P_from_C_read(fds, i, &buf, sizeof(buf));
            
            C2P_singal = atoi(buf);         // change char[] buf to int
            
            //            printf("C2P singal %d \n" , C2P_singal);
            
            if (C2P_singal == 500) {        // if singal = READY
                index[i-1]++;               // specific child's spot ++
                printf("Child %d sends READY\n" , i);
            }else if(C2P_singal == 2000){   // input has mistake
                cout << "\n\n******Input file content has mistake*****" << endl;
                cout << "\n*************Program failed***************\n\n" << endl;
                exit(0);                    // exit
            }
            
        }
        
        for (int i = 0; i < 5 ; i++) {
            if (index[i] <= 0){
                flag++;
            }
        }
        
        if (flag == 0) {
            printf("************************************\n");
            printf("Parent READY\n");
            break;
        }else{
            flag = 0;
        }
    }
}



/************************************************************************
 
 
 CHILDREN FUNCTIONS
 
 
 *************************************************************************/


/************************************************************************
 
 Function:        childGetID
 
 Description:     child get ID
 
 *************************************************************************/

void childGetID(int fds[10][2]){
    
    C_from_P_read(fds, 1, &ID, sizeof(ID)); // use 0 pipe for original communicating
    
}



/************************************************************************
 
 Function:        pipeToSTD_IN_OUT
 
 Description:     change pipe to std:in and std:out
 
 *************************************************************************/
void pipeToSTD_IN_OUT(int fds[10][2]){
    
    /* initialize pipe. */
    
    /** piping index rules :
     * even number for parent-to-children's write_communication
     * index = id * 2 - 2
     * odd  number for children-to-parent's write_communication
     * index = id * 2 - 1
     */
    
    /* change pipe into std::in and std::out mode */
    close(fds[ID * 2 - 2][WRITE_END]); // close P2C write end
    close(fds[ID * 2 - 1][READ_END]);  // close C2P read end
    
    dup2(fds[ID * 2 - 2][READ_END], STDIN_FILENO);    //P2C
    dup2(fds[ID * 2 - 1][WRITE_END], STDOUT_FILENO);  //C2P
    // you should be able now to close the two remaining
    // pipe file desciptors as well as you dup'ed them already
    // (confirmed that it is working)
    close(fds[ID * 2 - 2][READ_END]);
    close(fds[ID * 2 - 1][WRITE_END]);
    
    /*https://stackoverflow.com/questions/51996946/c-both-input-and-output-pipe-to-the-external-program*/
}


/************************************************************************
 
 Function:        childExecProgram
 
 Description:     child exec program
 
 *************************************************************************/
void childExecProgram(const char* filename){
    /* change ID to char* */
    std::string s = std::to_string(ID);
    char const *pchar = s.c_str();
    
    /* drop child process, change into a new  running program */
    if (execlp(filename, pchar , NULL)<0){
        printf("Wrong!!");
    }
    
}


int main(int argc, const char * argv[]) {
    
    // Generate 25 random numbers into 5 input_file
    //    generateInputFiles();
    
    for (int i = 0; i < 5; i++) {
        sem_init(&sem[i], 0, 0);
    }
    
    
    printf("Process communication program begin.\n");
    
    int fds[10][2];
    char buf[1024];
    int times = 0;
    for (int i = 0; i < 10; i++) {
        pipe(fds[i]);
    }
    
    pid_t pid;
    
    // fork 5 children processes
    printf("Starting fork children process.\n");
    
    for (int i = 0 ; i < 5; i++) {
        pid = fork();
        if (pid != 0 && pid != -1) {
            printf("NO. %d Child has been forked by parent\n" , i + 1);
        }
        sleep(1);
        if(pid == 0 || pid == -1){
            break;
            //Ref : https://blog.csdn.net/cupidove/article/details/9297335
        }
    }
    
    if (pid < 0){/* error occurred*/
        fprintf(stderr, "Fork Failed");
        return 1;
        
    }else if (pid == 0) { /* child process*/
        childGetID(fds);
        
        pipeToSTD_IN_OUT(fds);
        
        // TODO: add file path
        string filename = "";
        
        childExecProgram(filename.c_str());
        
    }else{ /* parent process*/
        
        /*
         • It allots ids 1-5 to each of the children and sends the same along the parent→child pipe.
         */
        assignID(fds);
        /*
         • The parent waits on all the child→parent pipes until it receives the code READY from all
         child processes. This ensures that the algorithm initiates only after all child processes
         have read the input completely.
         */
        waitForReady(fds );
        
        
        /*
         • The parent instantiates k = n/2 (we find the kth smallest element in the array – to find
         median, we require k=n/2).
         */
        
        int k = 25 / 2;
        
        
        
        int request_id ;
        
        /*
         • It then repeats the REQUEST until it finds the median.
         • Once the median is found, the parent reports it and sends a user-defined signal to all its
         children - and the child processes exit after handling the signal
         */
        
        srand((int)time(NULL));
        
        while (true) {
            // Request
            times++;
            printf("************************************\n");
            printf("No. %d Try!!\n" , times);
            printf("************************************\n");
            
            while (true) {
                /*
                 • The parent selects a random child and queries it for a random element.
                 •
                 • The parent sends the command REQUEST to a random child.
                 */
                request_id = rand() % 5 + 1;
                
                p2c_send_signal(fds, request_id, REQUEST);
                sleep_gap();
                
                /*
                 • It then reads the response from the child along the corresponding child→parent pipe.
                 If the response is -1, it repeats the same again. If not, it continues.
                 */
                memset(buf, 0, sizeof(buf));    // clear buf container
                P_from_C_read(fds, request_id, buf, sizeof(buf));
                sleep_gap();
                if (atoi(buf) != -1) {
                    break;
                }
            }
            //            printf("*************************\n");
            
            printf("Parent sends REQUEST to Child %d\n" , request_id);
            /*
             • The first non-negative value forms our pivot element.
             */
            int pivot_p = atoi(buf);
            
            printf("Child %d sends %d to parent\n", request_id , pivot_p);
            
            /*
             • The parent subsequently broadcasts this pivot element to all its child processes.
             To do the same, it first writes the code PIVOT along each of the parent→child pipes and
             subsequently writes the value of the pivot element.
             */
            
            printf("Parent broadcasts pivot %d to all children\n", pivot_p);
            
            
            for (int i = beginning_index; i < 6; i++) {
                p2c_send_signal(fds, i, PIVOT);
                sleep_gap();
                memset(buf, 0, sizeof(buf));    // clear buf container
                P_from_C_read(fds, i, buf, sizeof(buf));
                //                printf("Child %d receives pivot ,%s",i  , buf);
                
            }
            
            int m = 0;
            int num[5] = {0};
            
            for (int i = beginning_index; i < 6; i++) {
                p2c_send_signal(fds, i, pivot_p);
                sleep(TIME);
                memset(buf, 0, sizeof(buf));    // clear buf container
                P_from_C_read(fds, i, buf, sizeof(buf));
                printf("Child %d receives pivot and replies %s\n",i  , buf);
                num[i - 1] = atoi(buf);
                m += atoi(buf);
                
            }
            
            
            /*
             • It then reads the response from each child. This represents the number of elements
             larger than the pivot in that child.
             It sums up the total from all its children, call it m.
             */
            
            
            printf("Parent: m = ");
            for (int i = 0; i < n - 1; i++) {
                printf("%d + ",num[i]);
            }
            printf("%d = %d. " , num[n-1] ,m );
            
            /*
             • if m = k, there are n/2 elements larger
             than pivot in the data set. Thus, pivot is the median. (Make sure you handle even values
             correctly).
             */
            //            printf("k ========= =====  %d  \n" , k);
            
            if (k == m) {
                printf("12 = 25/2 Median found! Pivot is %d !!\n",pivot_p );
                printf("Parent sends kill signals to all children\n");
                
                for (int i = beginning_index; i < 6; i++) {
                    p2c_send_signal(fds, i, BREAK);
                    sleep(TIME);
                    memset(buf, 0, sizeof(buf));    // clear buf container
                    while (true) {
                        P_from_C_read(fds, i, buf, sizeof(buf));
                        if (atoi(buf) == TERMINATE) {
                            printf("Child %d terminates\n",i);
                            break;
                        }
                        sleep(TIME);
                    }
                    
                }
                
                break;
            }else if( k < m ){
                printf("\nMedian not found!  Send SMALL to children\n" );
                
                /*
                 • If m>k, it sends the command SMALL to all its children which signifies that the children
                 should drop all elements smaller than the pivot element. (Since the median would lie on
                 the right)
                 */
                for (int i = beginning_index; i < 6; i++) {
                    p2c_send_signal(fds, i, SMALL);
                    sleep_gap();
                    memset(buf, 0, sizeof(buf));    // clear buf container
                    P_from_C_read(fds, i, buf, sizeof(buf));
                    printf("SMALL after: array size %s\n" , buf);
                }
                
                
            }else if(k > m) {
                printf("\nMedian not found!  Send LARGE to children\n" );
                
                /*
                 • if m<k, it sends the command LARGE to all its children which signifies that the children
                 should drop all elements larger than the pivot element. (Since the median would lie on the
                 left). It also updates k = k - m. (Find out why?)
                 */
                
                for (int i = beginning_index; i< 6; i++) {
                    p2c_send_signal(fds, i, LARGE);
                    sleep_gap();
                    memset(buf, 0, sizeof(buf));    // clear buf container
                    P_from_C_read(fds, i, buf, sizeof(buf));
                    printf("Large after: array size %s\n" , buf);
                }
                
                k = k - m ;
                
            }
            
            
            
            
        }
        
    } // end parent
    
}

