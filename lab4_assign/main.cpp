//
// Created by Susan Liu on 12/1/22.
//
#include <iostream>
#include <fstream>
#include <vector>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <utility>
#include <cstring>

using namespace std;

/******************************* io_request ************************************/

typedef struct io_request{
    int io_id = -1;
    int track = 0;
    int arrival_time = 0;
    int start_time = 0;
    int end_time = 0;
} io_request;

/******************************* Global Variable ***************************/
vector<io_request*> io_inputs;
string sched;
int CURRENT_TIME = 0;
int total_time = 0;
int tot_movement = 0;
double total_turnaround = 0;
double total_waitting = 0;
double avg_turnaround = 0;
double avg_waittime = 0;
int max_waittime = 0;
int head = 0;
io_request* CURRENT_RUNNING = nullptr;
deque<io_request*> request_queue;

/******************************* Scheduler ************************************/

class Scheduler {
public:
    virtual void add_request(io_request* req){}
    virtual io_request* get_next_request(){ return nullptr; }
    virtual void remove_request(){}
};
class FIFO : public Scheduler{
public:
    void add_request(io_request* req){
        request_queue.push_back(req);
    }
    io_request* get_next_request(){
        //cout << "get next request" << endl;
        io_request* req = request_queue.front();

        //request_queue.pop_front();
        return req;
    }
    void remove_request(){
        request_queue.pop_front();
    }
};

/******************************* methods *********************************/

void read_file(char* input_file){

    int i = 0;
    string line;
    string fileName(input_file);
    ifstream infile(fileName);

    if(!infile.is_open()){
        cout << "Cannot Open File" << endl;
        exit(1);
    }
    while(getline(infile, line)) {

        if (line[0] == '#') continue;
        char *c = const_cast<char *>(line.c_str());
        io_request* req = new io_request();
        req->io_id = i;
        char *token = strtok(c, " \t\n");
        req->arrival_time = atoi(token);
        token = strtok(NULL, " \t\n");
        req->track = atoi(token);
        io_inputs.push_back(req);
        i++;
    }
    infile.close();
}
Scheduler* scheduler = new Scheduler;
void read_sched(int argc, char *argv[]){

    int opt;

    while ((opt = getopt(argc, argv, "s:")) != -1) {
        switch (opt) {
            case 's':
                sched = optarg;
        }
    }

    if(sched == "i") {
        scheduler = new FIFO();
    }
}
void print_output(){

    avg_turnaround = total_turnaround / io_inputs.size();
    avg_waittime = total_waitting / io_inputs.size();

    for(io_request* req: io_inputs){
        printf("%5d: %5d %5d %5d\n",req->io_id, req->arrival_time, req->start_time, req->end_time);
    }
    printf("SUM: %d %d %.2lf %.2lf %d\n",
           total_time, tot_movement, avg_turnaround, avg_waittime, max_waittime);
}
void simulation(){

    int i = 0;

    while(true){
        //if a new I/O arrived at the system at this current time
        if(i < io_inputs.size() && CURRENT_TIME == io_inputs[i]->arrival_time){
            //cout << "CURRENT TIME: " + to_string(CURRENT_TIME) << endl;
            scheduler->add_request(io_inputs[i]);
            i++;
        }

        //if an IO is active and completed at this time
        if(CURRENT_RUNNING != nullptr && CURRENT_RUNNING->track == head){
            //cout << "----------------------" << endl;
            CURRENT_RUNNING->end_time = CURRENT_TIME;
            total_turnaround += CURRENT_RUNNING->end_time - CURRENT_RUNNING->arrival_time;
            total_waitting += CURRENT_RUNNING->start_time - CURRENT_RUNNING->arrival_time;
            max_waittime = max(CURRENT_RUNNING->start_time - CURRENT_RUNNING->arrival_time, max_waittime);
            CURRENT_RUNNING = nullptr;
            //cout << request_queue.size() << endl;
            scheduler->remove_request();
            //cout << request_queue.size() << endl;
        }


        //if no IO request active now
        if(CURRENT_RUNNING == nullptr && !request_queue.empty()){
            //cout << "get next request, queue.size = " + to_string(request_queue.size()) << endl;
            if(i != io_inputs.size() || !request_queue.empty()){
                io_request* next_req = scheduler->get_next_request();
                //cout << "CURRENT TIME: " + to_string(CURRENT_TIME) << endl;
                next_req->start_time = CURRENT_TIME;
                CURRENT_RUNNING = next_req;
                //scheduler->remove_request();
                continue;
            }
        }
        else if(CURRENT_RUNNING == nullptr && i == io_inputs.size() && request_queue.empty()){
            //cout << "exit" << endl;
            total_time = CURRENT_TIME;
            break;
        }
        //if an IO is active
        if(CURRENT_RUNNING != nullptr){
//            cout << "id: " + to_string(CURRENT_RUNNING->io_id) + " track: " + to_string(CURRENT_RUNNING->track) + " head: " +
//                    to_string(head) << endl;
            int seek_track = CURRENT_RUNNING->track;
            if(seek_track == head){
                continue;
            }
            else{
                if(seek_track > head) head ++;
                else if(seek_track < head) head--;
                tot_movement ++;
            }
        }
        CURRENT_TIME++;
    }
}

int main(int argc, char *argv[]){

    read_sched(argc, argv);
    read_file(argv[argc - 1]);

    simulation();
    //cout << "--------------------" << endl;
    print_output();
}