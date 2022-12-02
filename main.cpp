#include <iostream>
#include <cstdio>
#include <unistd.h>
#include <cstring>
#include <cstdlib>
#include "simlib.h"

//TIME CONSTANTS
#define MINUTE 1
#define HOUR (60*MINUTE)
#define DAY (24*HOUR)
#define WEEK (7*DAY)
#define SIM_LEN (4*WEEK) // 1 month in minutes

//CAPACITIES
const int FURNACE_CAPACITY = 150;


// GLOBALS
int to_bake = 150; //number of products needed for baking process
const int no_of_workers = 2;

Facility Furnace("pec");
Facility Workers[no_of_workers];
Queue WorkerQueue;

Store Potter_circles ("Hrnciarske kruhy", 2);

Histogram celk("Celková doba pobytu v systému", 0, 10, 30);


class RawClay_product : public Process
{
    void Behavior() override
    {
        double enter_time = Time;

        int kt = -1;
        back:
        for (int a = 0; a < no_of_workers; a++)
        {

        }

        //Seize(Workers[x]) TODO figure out which worker is free and assign to work
        Enter(Potter_circles);

        Wait(Exponential(45));
        Leave(Potter_circles);
        Wait(4*DAY);
        to_bake+=1; // add to bake queue

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

class Baking : public Process
{
    void Behavior() override
    {
        if (to_bake - FURNACE_CAPACITY == 0)
        {
            //Loading
            for (auto & Worker : Workers)
            {
                Seize(Worker, 10);
            }

            Wait(Exponential(HOUR));

            for (auto & Worker : Workers)
            {
                Release(Worker);
            }

            //Baking
            Seize(Furnace);
            Wait(8*HOUR);
            Release(Furnace);
        }
    }
};


class Clay_generator : public Event
{
    void Behavior() override
    {
        (new RawClay_product)->Activate();
        (new RawClay_product)->Activate();
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
    Run();

    return 0;
}