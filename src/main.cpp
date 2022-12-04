#include <iostream>
#include <getopt.h>
#include "simlib.h"
#include "main.h"

// TIME CONSTANTS
#define MINUTE 1
#define HOUR (60 * MINUTE)
#define DAY (24 * HOUR)
#define WEEK (7 * DAY)
#define MONTH (4 * WEEK)
#define SIM_LEN (6 * MONTH)


// CAPACITIES
const int FURNACE_CAPACITY = 150;
const int FURNACE_2ND_CAPACITY = 80;
const int NO_OF_WORKERS = 10;

// GLOBALS
int used_clay = 0;
int to_bake_first = 0;  // number of products that are currently ready for baking
int to_bake_second = 0; // number of products that are ready for second round of baking
int baked_second = 0;   // number of products that has already been baked both times
int verbose = 0;

int usable_workers_i = 2;
int pottery_circles_i = 2;
int worktime_i = 10;
int avail_clay_i = 1650;
int *usable_workers = &usable_workers_i;
int *pottery_circles = &pottery_circles_i;
int *worktime = &worktime_i;
int *avail_clay = &avail_clay_i;

Facility Furnace("Furnace");
Facility Workers[NO_OF_WORKERS];

Store Potter_circles("Pottery circles", *pottery_circles);
Queue WorkerQueue("Worker queue"); // aggregated queue for workers

Histogram Clay_products("Clay products ready for baking", 0, WEEK, 24);
Histogram Glazed_products("Glazed products ready for baking", 0, WEEK, 24);
Histogram Finished_products("Done products", 0, WEEK, 24);

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
///
/// \param opt_arg
/// \param arg
void check_arg(char *opt_arg, int *arg)
{
    char *endptr = nullptr;

    if (check_is_number(opt_arg))
    {
        *arg = (int) strtol(opt_arg, &endptr, 10);
    } else
    {
        std::cout << "Bad number";
        exit(EXIT_FAILURE);
    }
}

enum log_level{
    EMPTY = 0,
    ERROR = 1,
    WARNING = 2,
    INFO = 3
};
std::string loglevel[4] = {"", "ERROR: ", "WARNING: ", "INFO: "};
// Verbose logging
class mystreambuf : public std::streambuf
{
};

mystreambuf nostreambuf;
std::ostream nocout(&nostreambuf);
#define log(x) ((x <= verbose)? std::cerr << loglevel[x] : nocout)

// Processes
void Baking::Behavior()
{
    // Select random worker
    int w1 = int(Random() * *usable_workers);

    // Loading
    Seize(Furnace);
    log(INFO) << "Seized furnace in time " << Time << std::endl;
    Seize(Workers[w1]);
    log(INFO) << "Seized worker[" << w1 << "] for loading in time " << Time << std::endl;
    Wait(Uniform(HOUR - 15, HOUR));
    Release(Workers[w1]);
    log(INFO) << "Released worker[" << w1 << "] from loading in time " << Time << std::endl;
    ActivateWorkerQueue();
    log(INFO) << "Activated worker queue in time " << Time << std::endl;
    // Baking
    log(INFO) << "Baking started in time " << Time << std::endl;
    Wait(16 * HOUR);

    // Select random worker
    w1 = int(Random() * *usable_workers);

    // Unloading
    Seize(Workers[w1]);
    log(INFO) << "Seized worker[" << w1 << "] for unloading in time " << Time << std::endl;
    Wait(Uniform(HOUR - 15, HOUR));
    Release(Furnace);
    log(INFO) << "Released furnace in time " << Time << std::endl;
    Release(Workers[w1]);
    log(INFO) << "Released worker[" << w1 << "] from unloading in time " << Time << std::endl;
    ActivateWorkerQueue();
    log(INFO) << "Activated worker queue in time " << Time << std::endl;
    log(INFO) << "BAKING PROCESS FINISHED IN TIME " << Time << std::endl << std::endl;
}


void Baking_2nd::Behavior()
{
    Priority = 3;
    Baking::Behavior();
    for (int i = 0; i < baked_count; i++)
    {
        if (Random() < 0.2) {
            baked_second--;
            (new Finished_product(5))->Activate();
        } else {
            Finished_products(Time);
        }
    }
}


void Finished_product::Behavior()
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
    log(INFO) << "Seized worker[" << kt << "] for painting product in time " << Time << std::endl;
    Wait(Uniform(work_period - work_period / 10.0, work_period + work_period / 10.0)); // glazing and painting
    Release(Workers[kt]);
    log(INFO) << "Released worker[" << kt << "] from painting product in time " << Time << std::endl;

    ActivateWorkerQueue();
    Glazed_products(Time);

    to_bake_second++;
    if (to_bake_second >= FURNACE_2ND_CAPACITY)
    {
        log(EMPTY) << std::endl;
        log(INFO) <<  "2ND BAKE PROCESS STARTED WITH " << to_bake_second << " PRODUCTS IN TIME " << Time
               << std::endl;
        to_bake_second -= FURNACE_2ND_CAPACITY;
        baked_second += FURNACE_2ND_CAPACITY;
        (new Baking_2nd(FURNACE_2ND_CAPACITY))->Activate();
    }
    else if (baked_second + to_bake_second == *avail_clay)
    {
        log(EMPTY) << std::endl;
        log(INFO) << "2ND BAKE PROCESS STARTED WITH " << to_bake_second << " PRODUCTS IN TIME " << Time << std::endl;

        int baked_amount = to_bake_second;
        baked_second += to_bake_second;
        to_bake_second = 0;
        (new Baking_2nd(baked_amount))->Activate();
    }
}

void Baking_1st::Behavior()
{
    Baking::Behavior();
    for (int i = 0; i < baked_count; i++)
    {
        (new Finished_product(60))->Activate();
    }
}

void Clay_product::Behavior()
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
    log(INFO) << "Seized worker[" << kt << "] for making clay product in time " << Time << std::endl;


    Enter(Potter_circles);
    log(INFO) << "Seized a pottery circle in time " << Time << std::endl;
    Wait(Uniform(HOUR - 30, HOUR));
    Leave(Potter_circles);
    log(INFO) << "Released a pottery circle in time " << Time << std::endl;
    Release(Workers[kt]);
    log(INFO) << "Released worker[" << kt << "] from making clay product in time " << Time << std::endl;


    ActivateWorkerQueue();
    Clay_products(Time);

    Wait(4 * DAY);      // drying
    log(INFO) << "One product dried up for baking in time " << Time << std::endl;

    to_bake_first += 1; // add to bake queue

    if (to_bake_first >= FURNACE_CAPACITY)
    {
        log(INFO) << std::endl << "1ST BAKE PROCESS STARTED WITH " << to_bake_first << " PRODUCTS IN TIME " << Time
               << std::endl;

        to_bake_first -= FURNACE_CAPACITY;
        used_clay -= FURNACE_CAPACITY;
        (new Baking_1st(FURNACE_CAPACITY))->Activate();
    } else if (used_clay - to_bake_first == 0)
    {
        log(INFO) << std::endl << "1ST BAKE PROCESS STARTED WITH " << to_bake_first << " PRODUCTS IN TIME " << Time
               << std::endl;

        int baked_count = to_bake_first;
        to_bake_first = 0;
        used_clay = 0;
        (new Baking_1st(baked_count))->Activate();
    }
}


void Freetime::Behavior()
{
    Priority = 10;
    Wait(*worktime * HOUR); // after x hours workers go home
    log(INFO) << "Trying to seize worker[" << workerNumber << "] for free time in time " << Time << std::endl;

    double tryingTime = Time;
    Seize(Workers[workerNumber]);
    log(INFO) << "Seized worker[" << workerNumber << "] for free time in time " << Time << std::endl;
    double seizedTime = Time;
    double overtime = seizedTime - tryingTime;
    log(INFO) << "Worker[" << workerNumber << "] worked in overtime for " << overtime << " minutes" << std::endl;

    Wait(((24 - *worktime) * HOUR) - overtime); // home for 24-x hours

    Release(Workers[workerNumber]);
    log(INFO) << "Released worker[" << workerNumber << "] from free time in time " << Time << std::endl;
    ActivateWorkerQueue();
}

// Events

void Clay_generator::Behavior()
{
    if (used_clay >= *avail_clay)
    {
        return;
    }

    (new Clay_product)->Activate();
    used_clay++;
    Activate(Time);
}


void Freetime_generator::Behavior()
{
    for (int i = 0; i < *usable_workers; i++)
    {
        (new Freetime(i))->Activate(); // every worker needs to have his own free time
    }

    Activate(Time + 24 * HOUR); // free time is every day
}

int main(int argc, char *argv[])
{
    int opt;
    while ((opt = getopt(argc, argv, "c:w:t:l:v:h")) != -1)
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
            case 'v':
            {
                check_arg(optarg, &verbose);
                break;
            }
            case 'h':
            {
                std::cout <<
                "-w  <number of workers (max is 10)>\n"
                "-c  <number of pottery circles used to make clay products (1 worker can use 1 circle at a time)>\n"
                "-t  <[hours] of workshift time each day (max is 24)>\n"
                "-l  <[kilograms] of clay that is available for the duration of simulation>\n"
                "-v  <verbose logging level to stderr (use this with \"2> redirect_file\")>"
                << std::endl;

                exit(EXIT_SUCCESS);
            }
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
    if (*pottery_circles > *usable_workers)
    {
        log(WARNING) << "WARNING: Number of pottery circles is greater than number of workers" << std::endl;
    }

    Potter_circles.SetCapacity(*pottery_circles);

    RandomSeed(time(nullptr));
    Init(0.0, SIM_LEN);

    std::cout << std::string(23, '-') << "SIMULATION START" << std::string(23, '-') << std::endl;
    std::cout << "Number of workers: " << *usable_workers << std::endl;
    std::cout << "Number of pottery circles: " << *pottery_circles << std::endl;
    std::cout << "Work time in hours: " << *worktime << std::endl;
    std::cout << "Available clay: " << *avail_clay << std::endl;
    std::cout << "Verbose level: " << verbose << std::endl << std::endl;

    (new Clay_generator)->Activate();
    (new Freetime_generator)->Activate();
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
    Glazed_products.Output();
    Finished_products.Output();

    std::cout << std::string(23, '-') << "SIMULATION END" << std::string(23, '-') << std::endl;
    return 0;
}
