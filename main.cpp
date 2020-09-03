/*
 * Brandon Castor  cssc 2129 817046315
 * Alexander Howes cssc2165 820184866
*/

#include <thread>
#include <csignal>
#include <sstream>
#include <iostream>
#include <unistd.h>
#include <mutex>

using std::cout;
using std::endl;
using std::stol;
using std::chrono::seconds;
using std::mutex;

enum Units{
    SECONDS = 1,
    MINUTES = 60
};


std::mutex mtx;
pthread_t processes[3];

struct arguments{
    long time;
    Units unit;
    long alarm;
};



void unlock(int signum) {
    pthread_cancel(processes[2]);
    cout << "Ending program" << endl;
    pthread_exit(NULL);
}


void *interrupter(void *arg) {
    signal(SIGINT, unlock);
}
/**
 * clock function contains the alarm and countdown as well as displaying the clock
 * @param arg
 * @return
 */
// Print the time forever until it is interrupted by process #2
void *clock(void *arg) {
    auto* args = (arguments*) arg;
    long count_down = args->time;
    int current_time = 0;

    // Will loop forever until the clockInterrupter changes the threadLock with signal.
    while (count_down > 0) {
        time_t theTime = time(NULL);
        struct tm *now = localtime(&theTime);
        if(current_time == args->alarm){
            mtx.lock();
            cout<< "Alarm is going off " << endl;
            mtx.unlock();
        }
        mtx.lock();
        cout <<  now->tm_hour << ":" << now->tm_min << ":" << now->tm_sec << endl;
        mtx.unlock();
        switch (args->unit){
            case SECONDS:
                sleep(1);
                count_down--;
                current_time++;
                break;
            case MINUTES:
                for(int i = 0; i <= MINUTES; i++){

                    if(current_time++ == args->alarm){
                        mtx.lock();
                        cout<< "Alarm is going off " << endl;
                        mtx.unlock();
                    }
                    sleep(1);
                    if(count_down-- <= 0){
                        break;
                    }
                }
                break;
        }
    }
    raise(SIGINT);
}

/**
 * function used by the main thread to run the worker threads
 * @param args
 */
void run(arguments args) {
    // Create threads
    int interruptor_thread = pthread_create(&processes[1], NULL, interrupter, NULL); // Thread two

    if(interruptor_thread){
        cout << " Error Creating interrupter worker threads, Program will now exit!" << endl;
        exit(EXIT_FAILURE);
    }
    pthread_join(processes[1], NULL);
    int clock_thread = pthread_create(&processes[2], NULL, clock, (void *) &args); // Thread one

    if(clock_thread){
        cout << " Error Creating clock worker threads, Program will now exit!" << endl;
        exit(EXIT_FAILURE);
    }
    pthread_join(processes[2], NULL);

    // End program
    cout << "All threads have closed" << endl;
    pthread_exit(NULL);
}

/**
 * helps the main thread run the worker threads
 * @param tmp
 * @return
 */
void *helper(void *tmp) {
    run(*((arguments *)tmp));
}
/**
 * checks for correct inputs by the user
 * @param arguments
 */
void check_arguments(const arguments arguments) {

    if(arguments.time < 0){
        cout << "time parameter cannot be negative, we cannot go back in time" << endl;
        exit(1);
    }

    if(arguments.unit != 1 && arguments.unit != 60){

        cout << " units parameter can only be 1 or 60" << endl;
        exit(1);
    }

    if(arguments.alarm < 0){
        cout << "alarm parameter cannot be negative, we cannot go back in time" << endl;
        exit(1);
    }


}


int main(int argc, char *argv[]) {

    arguments args{25, Units::SECONDS, 17};

    switch (argc) {
        case 1: break;

        // Get the input provided by the user
        case 3:
            args = {25, Units(stol(argv[1])), stol(argv[2])};
            break;


        case 4:
            args = {stol(argv[1]), Units(stol(argv[2])), stol(argv[3])};
            break;

        default:
            cout << "Error. Please try mot, or mot <time> <unit> <alarm>" << endl;
            exit(1);
    }

    check_arguments(args);

    // Call the run function to start the clock

    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS, NULL);

    pthread_create(&processes[0], NULL, helper, (void *) &args);
    pthread_join(processes[0], NULL);

}


