#include <iostream>
#include <getopt.h>
#include "simlib.h"

// TIME CONSTANTS
#define MINUTE 1
#define HOUR (60 * MINUTE)
#define DAY (24 * HOUR)
#define WEEK (7 * DAY)
#define SIM_LEN (4 * WEEK) // 1 month in minutes

// CAPACITIES
const int FURNACE_CAPACITY = 150;
const int FURNACE_2ND_CAPACITY = 75;
const int NO_OF_WORKERS = 10;

// GLOBALS
int used_clay = 0;
int to_bake_first = 0;  // number of products that are currently ready for baking
int to_bake_second = 0; // number of products that are ready for second round of baking
int baked_second = 0;   // number of products that has already been baked both times

int usable_workers_i = 4;
int pottery_circles_i = 2;
int worktime_i = 10;
int avail_clay_i = 300;
int *usable_workers = &usable_workers_i;
int *pottery_circles = &pottery_circles_i;
int *worktime = &worktime_i;
int *avail_clay = &avail_clay_i;

Facility Furnace("Furnace");
Facility Workers[NO_OF_WORKERS];

Store Potter_circles("Pottery circles", *pottery_circles);
Queue WorkerQueue; // agregated queue for workers

Histogram Clay_products("Time until clay products are ready for baking", 0, DAY, 28);
Histogram Finished_products("Time until finished products are ready for baking", 0, DAY, 28);

/**
 * Helper function to put workers back to work
 */
void ActivateWorkerQueue()
{
    if (WorkerQueue.Length() > 0)
    {
        WorkerQueue.GetFirst()->Activate();
    }
}
/// Checks if string contains only digits
/// \param s string
/// \return True if digits only
bool check_is_number(const std::string &s)
{
    std::string::const_iterator it = s.begin();
    while (it != s.end() && std::isdigit(*it))
        ++it;
    return !s.empty() && it == s.end();
}

void check_arg(char *opt_arg, int *arg)
{
    char *endptr = nullptr;

    if (check_is_number(opt_arg))
    {
        *arg = (int)strtol(opt_arg, &endptr, 10);
    }
    else
    {
        std::cout << "Bad number";
        exit(EXIT_FAILURE);
    }
}

// Processes
class Baking : public Process
{
protected:
    int baked_count;

    void Behavior() override
    {
        // Select random worker
        int w1 = int(Random() * *usable_workers);

        // Loading
        Seize(Workers[w1]);
        Seize(Furnace);
        Wait(Exponential(HOUR));
        Release(Workers[w1]);
        ActivateWorkerQueue();

        // Baking
        Wait(16 * HOUR);

        // Select random worker
        w1 = int(Random() * *usable_workers);

        // Unloading
        Seize(Workers[w1]);
        Wait(Exponential(HOUR));
        Release(Furnace);
        Release(Workers[w1]);
        ActivateWorkerQueue();
    }

public:
    explicit Baking(int baked_count) : Process()
    {
        this->baked_count = baked_count;
    }
};

class Baking_2nd : public Baking
{
    void Behavior() override
    {
        Priority = 3;
        Baking::Behavior();
    }

public:
    explicit Baking_2nd(int baked_count) : Baking(baked_count) {}
};

class Finished_product : public Process
{
    void Behavior() override
    {
        int kt = -1;

    back:
        for (int a = 0; a < *usable_workers; a++)
        {
            if (!Workers[a].Busy())
            {
                kt = a;
                break;
            }
        }
        if (kt == -1)
        {
            Priority = 3;
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
        if (to_bake_second >= FURNACE_2ND_CAPACITY)
        {
            std::cout << "2nd bake started with " << to_bake_second << " products" << std::endl;
            std::cout << "Time " << Time << std::endl;
            to_bake_second -= FURNACE_2ND_CAPACITY;
            baked_second += FURNACE_2ND_CAPACITY;
            (new Baking_2nd(FURNACE_2ND_CAPACITY))->Activate();
        }
        else if (baked_second + to_bake_second == *avail_clay)
        {
            std::cout << "2nd bake started with " << to_bake_second << " products" << std::endl;
            std::cout << "Time " << Time << std::endl;

            (new Baking_2nd(to_bake_second))->Activate();
            baked_second += to_bake_second;
            to_bake_second = 0;
        }
    }
};

class Baking_1st : public Baking
{
    void Behavior() override
    {
        Baking::Behavior();
        for (int i = 0; i < baked_count; i++)
        {
            (new Finished_product)->Activate();
        }
    }

public:
    explicit Baking_1st(int baked_count) : Baking(baked_count) {}
};

class Clay_product : public Process
{
    void Behavior() override
    {
        int kt = -1;
    back:
        for (int a = 0; a < *usable_workers; a++)
        {
            if (!Workers[a].Busy())
            {
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

        Wait(4 * DAY);      // drying
        to_bake_first += 1; // add to bake queue

        if (to_bake_first >= FURNACE_CAPACITY)
        {
            std::cout << "1st bake started with " << to_bake_first << " products" << std::endl;
            std::cout << "Time " << Time << std::endl;

            to_bake_first -= FURNACE_CAPACITY;
            used_clay -= FURNACE_CAPACITY;
            (new Baking_1st(FURNACE_CAPACITY))->Activate();
        }
        else if (used_clay - to_bake_first == 0)
        {
            std::cout << "1st bake started with " << to_bake_first << " products" << std::endl;
            std::cout << "Time " << Time << std::endl;

            (new Baking_1st(to_bake_first))->Activate();
            to_bake_first = 0;
            used_clay = 0;
        }
    }
};

class Freetime : public Process
{
    int workerNumber;

    void Behavior() override
    {
        Wait(*worktime * HOUR); // after 10 hours workers go home

        if (Workers[workerNumber].Busy())
        {
            Workers[workerNumber].QueueIn(this, 10);
            Passivate();
        }
        else
        {
            Seize(Workers[workerNumber], 10);
        }

        Wait((24 - *worktime) * HOUR); // home for 14 hours

        Release(Workers[workerNumber]);
        ActivateWorkerQueue();
    }

public:
    explicit Freetime(int workerNumber) : Process()
    {
        this->workerNumber = workerNumber;
    }
};

// Events
class ClayGenerator : public Event
{
    void Behavior() override
    {
        if (used_clay >= *avail_clay)
        {
            return;
        }

        (new Clay_product)->Activate();
        used_clay++;
        Activate(Time);
    }
};

class FreetimeGenerator : public Event
{
    void Behavior() override
    {
        for (int i = 0; i < *usable_workers; i++)
        {
            (new Freetime(i))->Activate(); // every worker needs to have his own free time
        }

        Activate(Time + 24 * HOUR); // free time is every day
    }
};

int main(int argc, char *argv[])
{
    int opt;
    while ((opt = getopt(argc, argv, "c:w:t:l:h")) != -1)
    {
        switch (opt)
        {
        case 'c':
        {
            check_arg(optarg, pottery_circles);
            break;
        }
        case 'w':
        {
            check_arg(optarg, usable_workers);
            break;
        }
        case 't':
        {
            check_arg(optarg, worktime);
            break;
        }
        case 'l':
        {
            check_arg(optarg, avail_clay);
            break;
        }
        case 'h':
            std::cout << "-w Number of workers (max is 10)\n"
                         "-c Number of pottery circles\n"
                         "-t Work time of the workers in hours (max is 24)\n"
                         "-l Clay that is available for the simulation\n"
                         "-h Print this help\n"
                      << std::endl;
            exit(0);
        default:
            return 1;
        }
    }
    if (*usable_workers > NO_OF_WORKERS)
    {
        std::cout << "Maximum number of workers is 10, you entered the number " << *usable_workers << std::endl;
        exit(EXIT_FAILURE);
    }
    if (*worktime > 24)
    {
        std::cout << "Maximum work time is 24, you entered the number " << *worktime << std::endl;
        exit(EXIT_FAILURE);
    }

    RandomSeed(time(nullptr));
    Init(0.0, SIM_LEN);

    std::cout << std::string(23, '-') << "SIMULATION START" << std::string(23, '-') << std::endl;
    std::cout << "Number of workers: " << *usable_workers << std::endl;
    std::cout << "Number of pottery circles: " << *pottery_circles << std::endl;
    std::cout << "Work time in hours: " << *worktime << std::endl;
    std::cout << "Available clay: " << *avail_clay << std::endl
              << std::endl;

    (new ClayGenerator)->Activate();
    (new FreetimeGenerator)->Activate();
    for (int i = 0; i < *usable_workers; i++)
    {
        std::string worker = "Worker " + std::to_string(i);
        Workers[i].SetName(worker);
    }
    Run();

    for (int i = 0; i < *usable_workers; i++)
    {
        Workers[i].Output();
    }
    Furnace.Output();
    WorkerQueue.Output();
    Clay_products.Output();
    Finished_products.Output();

    std::cout << std::string(23, '-') << "SIMULATION END" << std::string(23, '-') << std::endl;
    return 0;
}
