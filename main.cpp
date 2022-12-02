#include <iostream>
#include <cstdio>
#include <unistd.h>
#include <cstring>
#include <cstdlib>
#include "simlib.h"
#include <cmath>

//TIME CONSTANTS
#define MINUTE 1
#define HOUR (60*MINUTE)
#define DAY (24*HOUR)
#define WEEK (7*DAY)
#define SIM_LEN (4*WEEK) // 1 month in minutes

//CAPACITIES
const int FURNACE_CAPACITY = 150;
const int AVAIL_CLAY = 600;
int USED_CLAY = 0;

// GLOBALS
int to_bake = 0; // number of products that are currently ready for baking
const int no_of_workers = 5;

Facility Furnace("pec");
Facility Workers[no_of_workers];
Queue WorkerQueue; // agregated queue for workers

Store Potter_circles ("Hrnciarske kruhy", 4);

Histogram celk("Celková doba pobytu v systému", 0, DAY, 40);

class Baking : public Process
{
    void Behavior() override
    {
        if (FURNACE_CAPACITY != to_bake) return;

        int w1 = int(Random()*no_of_workers);
        int w2 = (int(Random()*(no_of_workers-1)) + w1) % no_of_workers;
        //Loading
        Seize(Workers[w1]);
        Seize(Workers[w2]);

        Wait(Exponential(HOUR));

        Release(Workers[w1]);
        Release(Workers[w2]);

        //Baking
        Seize(Furnace);
        Wait(16 * HOUR);
        Release(Furnace);
        to_bake -= 150;
    }
};

class Clay_product : public Process
{
    void Behavior() override
    {
        double enter_time = Time;

        int kt = -1;
        back:
        for (int a = 0; a < no_of_workers; a++)
        {
            if (!Workers[a].Busy()) {
                kt = a;
                break;
            }
        }
            if (kt == -1)
            {
                Into(WorkerQueue);
                Passivate();
                goto back;
            }
        Seize(Workers[kt]);

        Wait(Exponential(45));
        Leave(Potter_circles);
        Release(Workers[kt]);

        if(WorkerQueue.Length() > 0)
        {
            WorkerQueue.GetFirst()->Activate();
        }

        Wait(4*DAY);
        to_bake+=1; // add to bake queue
        if(to_bake == FURNACE_CAPACITY)
        {
            Wait(Exponential(HOUR)); //loading
        }
        (new Baking)->Activate();

        celk(Time-enter_time);
    }
};

class Freetime : public Process
{
    void Behavior() override
    {
        Wait(10*HOUR); //after 10 hours workers go home
        for (auto & Worker : Workers)
        {
            Seize(Worker, 10);
        }

        Wait(14*HOUR); // home for 14 hours

        for (auto & Worker : Workers)
        {
            Release(Worker);
        }
    }

};


class Clay_generator : public Event
{
    void Behavior() override
    {
        (new Clay_product)->Activate();
        if (USED_CLAY <= AVAIL_CLAY)
        {
            USED_CLAY++;
            Activate(Time);
        }
    }
};

class FreetimeGenerator : public Event
{
    void Behavior() override
    {
        (new Freetime)->Activate();
        Activate(Time+24*HOUR); //free time is every day
    }
};

int main(int argc, char *argv[])
{

    RandomSeed(time(nullptr));
    Init(0.0, SIM_LEN);

    (new Clay_generator)->Activate();
    (new FreetimeGenerator)->Activate();
    Run();

    for (auto & Worker : Workers)
    {
        Worker.Output();
    }

    WorkerQueue.Output();
    celk.Output();
    return 0;
}