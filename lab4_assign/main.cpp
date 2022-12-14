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
#include <deque>
#include <climits>

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
        if(request_queue.empty()) return nullptr;
        io_request* req = request_queue.front();

        //request_queue.pop_front();
        return req;
    }
    void remove_request(){
        request_queue.pop_front();
    }
};
class SSTF : public Scheduler{
public:

    int target_index = -1;
    //deque<io_request*>::iterator itr;
    void add_request(io_request* req){
        request_queue.push_back(req);
    }
    io_request* get_next_request(){
        //cout << "get next request" << endl;
        //deque<io_request*>::iterator itr;
        int min_distance = 10000;

        if(request_queue.empty()) return nullptr;

        for(int i = 0; i < request_queue.size(); i++){
            if(abs(request_queue[i]->track - head) < min_distance){
                min_distance = abs(request_queue[i]->track - head);
                target_index = i;
            }
        }
        io_request* req = request_queue[target_index];
        return req;
    }
    void remove_request(){
        //cout << "before erase: " + to_string(request_queue.size()) << endl;
        request_queue.erase(request_queue.begin() + target_index, request_queue.begin() + target_index + 1);
        //cout << "after erase: " + to_string(request_queue.size()) << endl;
    }
};
class SCAN : public Scheduler{
public:
    int direction = 1; //alwasy starts at 0 -> right
    int target_i = 0;

    void add_request(io_request* req){
        request_queue.push_back(req);
    }
    io_request* get_next_request(){

        int min_distance = INT32_MAX;
        io_request* req;
        bool change_dir = true;

        if(request_queue.empty()) return nullptr;

        for(int i = 0; i < request_queue.size(); i++){
            int curr_dis = head - request_queue[i]->track;
            if(direction == 1 && curr_dis <= 0){
                if(abs(curr_dis) < min_distance){
                    change_dir = false;
                    min_distance = abs(curr_dis);
                    target_i = i;
                    continue;
                }
            }else if(direction == -1 && curr_dis >= 0){
                if(abs(curr_dis) < min_distance){
                    change_dir = false;
                    min_distance = abs(curr_dis);
                    target_i = i;
                }
            }
        }
        if(change_dir){
            direction = direction * -1;
            get_next_request();
        }
        req = request_queue[target_i];

        return req;
    }
    void remove_request(){
        //cout << "before erase: " + to_string(request_queue.size()) << endl;
        request_queue.erase(request_queue.begin() + target_i, request_queue.begin() + target_i + 1);
        //cout << "after erase: " + to_string(request_queue.size()) << endl;
    }
};
class FLOOK : public Scheduler{
public:
    int direction = 1; //alwasy starts at 0 -> right
    int target_i = 0;
    //deque<io_request*> active_q;
    deque<io_request*> add_queue;

    void add_request(io_request* req){

        //cout << "add request" << endl;
        add_queue.push_back(req);

    }

    io_request* get_next_request(){

        int min_distance = INT32_MAX;
        io_request* req;
        bool change_dir = true;



        if(request_queue.empty() && add_queue.empty()) return nullptr;

        if(request_queue.empty() && !add_queue.empty())
            swap(request_queue, add_queue);

        for(int i = 0; i < request_queue.size(); i++){
            int curr_dis = head - request_queue[i]->track;
            if(direction == 1 && curr_dis <= 0){
                if(abs(curr_dis) < min_distance){
                    change_dir = false;
                    min_distance = abs(curr_dis);
                    target_i = i;
                    continue;
                }
            }else if(direction == -1 && curr_dis >= 0){
                if(abs(curr_dis) < min_distance){
                    change_dir = false;
                    min_distance = abs(curr_dis);
                    target_i = i;
                }
            }
        }
        if(change_dir){
            direction = direction * -1;
            get_next_request();
        }

        //cout << "active queue size" + to_string(request_queue.size()) << endl;
        req = request_queue[target_i];

        //cout << "track: " + to_string(req->track) << endl;

        return req;
    }
    void remove_request(){
        //cout << "before erase: " + to_string(request_queue.size()) << endl;

        request_queue.erase(request_queue.begin() + target_i, request_queue.begin() + target_i + 1);
        if(request_queue.empty() && !add_queue.empty())
            swap(request_queue, add_queue);
        //cout << "after erase: " + to_string(request_queue.size()) << endl;
    }
};
class CLOOK : public Scheduler{
public:
    int direction = 1; //alwasy starts at 0 -> right
    int target_i = 0;
    int min_index = 0;

    void add_request(io_request* req){
        request_queue.push_back(req);
    }
    io_request* get_next_request(){

        int min_distance = INT32_MAX;
        io_request* req;
        bool change_dir = true;
        int min_track = INT32_MAX;

        if(request_queue.empty()) return nullptr;

        for(int i = 0; i < request_queue.size(); i++){
            int curr_dis = head - request_queue[i]->track;
            if(direction == 1 && curr_dis <= 0){
                if(abs(curr_dis) < min_distance){
                    change_dir = false;
                    min_distance = abs(curr_dis);
                    target_i = i;
                    continue;
                }
            }
            if(request_queue[i]->track < min_track){
                min_track = request_queue[i]->track;
                min_index = i;
            }
        }

        if(change_dir){
            req = request_queue[min_index];
            target_i = min_index;
        }else{
            req = request_queue[target_i];
        }

        return req;
    }
    void remove_request(){
        //cout << "before erase: " + to_string(request_queue.size()) << endl;
        request_queue.erase(request_queue.begin() + target_i, request_queue.begin() + target_i + 1);
        //cout << "after erase: " + to_string(request_queue.size()) << endl;
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
    if(sched == "j"){
        scheduler = new SSTF();
    }
    if(sched == "s"){
        scheduler = new SCAN();
    }
    if(sched == "c"){
        scheduler = new CLOOK();
    }
    if(sched == "f"){
        scheduler = new FLOOK();
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
            scheduler->remove_request();
        }


        //if no IO request active now
        if(CURRENT_RUNNING == nullptr){
            //cout << "get next request, queue.size = " + to_string(request_queue.size()) << endl;
            if(i != io_inputs.size() || !request_queue.empty()){
                io_request* next_req = scheduler->get_next_request();
                if(next_req != nullptr){
                    next_req->start_time = CURRENT_TIME;
                    CURRENT_RUNNING = next_req;
                    continue;
                }
            }
        }
        if(CURRENT_RUNNING == nullptr && i == io_inputs.size() && request_queue.empty()){
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