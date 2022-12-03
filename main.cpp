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
const int FURNACE_2ND_CAPACITY = 80;
const int AVAIL_CLAY = 250;
int USED_CLAY = 0;

// GLOBALS
int to_bake_first = 0; // number of products that are currently ready for baking
int to_bake_second = 0;
const int no_of_workers = 2;

Facility Furnace("Furnace");
Facility Workers[no_of_workers];
Queue WorkerQueue; // agregated queue for workers

Store Potter_circles ("Hrnciarske kruhy", 2);

Histogram Clay_products("Doba kym sa vyrobia hlinene produkty", 0, DAY, 28);
Histogram Finished_products("Doba kym sa vyrobia hotove produkty", 0, DAY, 28);


/**
 * Helper function to put workers back to work
 */
void ActivateWorkerQueue()
{
    if(WorkerQueue.Length() > 0)
    {
        WorkerQueue.GetFirst()->Activate();
    }
}

// Processes
class Baking : public Process
{
    void Behavior() override {
        int w1 = int(Random() * no_of_workers);

        //Loading
        Seize(Workers[w1]);
        Seize(Furnace);
        Wait(Exponential(HOUR));
        Release(Workers[w1]);
        ActivateWorkerQueue();

        // baking
        Wait(16 * HOUR);

        //Unloading
        w1 = int(Random() * no_of_workers);

        //Unloading
        Seize(Workers[w1]);
        Wait(Exponential(HOUR));
        Release(Furnace);
        Release(Workers[w1]);
        ActivateWorkerQueue();
    }

};

class Finished_product : public Process
{
    void Behavior() override
    {
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
            Priority = 2;
            Into(WorkerQueue);
            Passivate();
            goto back;
        }

        Seize(Workers[kt]);
        Wait(Exponential(45)); // glazing and painting
        Release(Workers[kt]);

        ActivateWorkerQueue();
        Finished_products(Time);

        to_bake_second++;
        if (to_bake_second >= FURNACE_2ND_CAPACITY) {
            to_bake_second -= FURNACE_2ND_CAPACITY;

            (new Baking)->Activate();
        }
    }
};

class Clay_product : public Process
{
    void Behavior() override
    {
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
        Enter(Potter_circles);
        Wait(Exponential(45));
        Leave(Potter_circles);
        Release(Workers[kt]);

        ActivateWorkerQueue();
        Clay_products(Time);

        Wait(4*DAY); //drying
        to_bake_first+=1; // add to bake queue
        if (to_bake_first >= FURNACE_CAPACITY)
        {
            to_bake_first -= FURNACE_CAPACITY;
            //choose random worker
            (new Baking)->Activate();
            for (int i = 0; i < FURNACE_CAPACITY; i++)
            {
                (new Finished_product)->Activate();
            }
        }
    }
};

class Freetime : public Process
{
    int workerNumber;

    void Behavior() override
    {
        Wait(10*HOUR); //after 10 hours workers go home

        if (Workers[workerNumber].Busy())
        {
            Workers[workerNumber].QueueIn(this, 10);
            Passivate();
        }
        else
        {
            Seize(Workers[workerNumber], 10);
        }

        Wait(14*HOUR); // home for 14 hours

        Release(Workers[workerNumber]);
        ActivateWorkerQueue();
    }

public:
    explicit Freetime(int workerNumber) : Process() {
        this->workerNumber = workerNumber;
    }
};

//Events
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
        for (int i = 0; i < no_of_workers; i++) {
            (new Freetime(i))->Activate(); //every worker needs to have his own free time
        }

        Activate(Time+24*HOUR); //free time is every day
    }
};

int main(int argc, char *argv[])
{
    RandomSeed(time(nullptr));
    Init(0.0, SIM_LEN);

    (new Clay_generator)->Activate();
    (new FreetimeGenerator)->Activate();
    for (int i = 0; i < no_of_workers; i++)
    {
        std::string worker = "Worker " + std::to_string(i);
        Workers[i].SetName(worker);
    }
    Run();

    for (auto & Worker : Workers)
    {
        Worker.Output();
    }
    Furnace.Output();
    WorkerQueue.Output();
    Clay_products.Output();
    Finished_products.Output();
    return 0;
}