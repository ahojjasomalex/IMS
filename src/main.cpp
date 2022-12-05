/// \author Alex Bazo
/// \author Marek Danco
/// \date 2022-12-05

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

// CONSTANTS
const int NO_OF_WORKERS = 10;

// GLOBALS
int furnace_capacity = 150;
int furnace_2nd_capacity = furnace_capacity / 2;
int workshop_product_capacity = 500;
int used_clay = 0;      // clay in kgs used for products
int to_bake_first = 0;  // number of products that are currently ready for baking
int to_bake_second = 0; // number of products that are ready for second round of baking
int baked_second = 0;   // number of products that has already been baked both times
int verbose = 0;        // verbose logging level

int usable_workers_i = 2;
int pottery_circles_i = 2;
int worktime_i = 10;
int avail_clay_i = 1800;

// Pointers to integers
int *usable_workers = &usable_workers_i;
int *pottery_circles = &pottery_circles_i;
int *worktime = &worktime_i;
int *avail_clay = &avail_clay_i;

Facility Furnace("Furnace");
Facility Workers[NO_OF_WORKERS];

Store Potter_circles("Pottery circles", *pottery_circles);
Store Product_store("Product store", workshop_product_capacity);
Queue WorkerQueue("Worker queue"); // aggregated queue for workers

Histogram Clay_products;
Histogram Glazed_products;
Histogram Finished_products;

void print_help()
{
    std::cout <<
              "-w, --workers            <number of workers (max is 10)>\n"
              "-c, --circles            <number of pottery circles used to make clay products (1 worker can use 1 circle at a time)>\n"
              "-t, --workshift          <[hours] of workshift time each day (max is 24)>\n"
              "-l, --clay               <[kilograms] of clay that is available for the duration of simulation>\n"
              "-v, --verbose            <verbose logging level to stderr (use this with \"2> redirect_file\")>"
              "-x, --furnace-capacity   <number of products that can be baked at the time>"
              "-z, --workshop-capacity  <number products that can be in workshop at the time>"
              "-h, --help               <prints this help>"
              << std::endl;
}

void ActivateWorkerQueue()
{
    if (WorkerQueue.Length() > 0)
    {
        WorkerQueue.GetFirst()->Activate();
    }
}

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
        *arg = (int) strtol(opt_arg, &endptr, 10);
    } else
    {
        std::cout << "Bad number";
        exit(EXIT_FAILURE);
    }
}

// Verbose logging levels
enum log_level{
    EMPTY = 0,
    //ERROR = 1,
    WARNING = 2,
    INFO = 3
};
std::string loglevel[4] = {"", "ERROR: ", "WARNING: ", "INFO: "};
// Helper class for logging
class mystreambuf : public std::streambuf
{
};

mystreambuf nostreambuf;
std::ostream nocout(&nostreambuf);
// Logging macro
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

void Baking_1st::Behavior()
{
    Baking::Behavior();
    for (int i = 0; i < baked_count; i++)
    {
        (new Finished_product(60))->Activate();
    }
}

void Baking_2nd::Behavior()
{
    Priority = 5; // Second baking has priority over first bake
    Baking::Behavior();
    for (int i = 0; i < baked_count; i++)
    {
        if (Random() < 0.2) {
            baked_second--;
            (new Finished_product(5))->Activate();
        } else {
            Leave(Product_store);  // Finished product leaving workshop
            Finished_products(Time);
        }
    }
}

void Clay_product::Behavior()
{
    int kt = -1;
    waiting_for_worker:
    for (int a = 0; a < *usable_workers; a++)
    {
        if (!Workers[a].Busy())
        {
            kt = a;
            break;
        }
    }
    if (kt == -1 || Product_store.Full() || Potter_circles.Full())
    {
        Into(WorkerQueue);
        Passivate();
        goto waiting_for_worker;
    }

    double start_time = Time;

    Seize(Workers[kt]);
    log(INFO) << "Seized worker[" << kt << "] for making clay product in time " << Time << std::endl;
    Enter(Potter_circles);
    log(INFO) << "Worker[" << kt << "] seized a pottery circle in time " << Time << std::endl;



    Wait(Uniform(HOUR - 30, HOUR));
    Leave(Potter_circles);
    log(INFO) << "Worker[" << kt << "] released a pottery circle in after " << Time-start_time << " minutes" << std::endl;
    Release(Workers[kt]);
    log(INFO) << "Released worker[" << kt << "] from making clay product in time " << Time << std::endl;

    ActivateWorkerQueue();
    Clay_products(Time);
    Enter(Product_store);
    Wait(4 * DAY);  // Drying
    log(INFO) << "One product dried up for baking in time " << Time << std::endl;

    to_bake_first++;

    if (to_bake_first >= furnace_capacity) // Baking with full furnace
    {
        log(INFO) << std::endl <<  to_bake_first << " PRODUCTS DRIED UP. 1ST BAKING PROCESS STARTED IN TIME " << Time
                  << std::endl;

        to_bake_first -= furnace_capacity;
        used_clay -= furnace_capacity;
        (new Baking_1st(furnace_capacity))->Activate();
    }
    else if (used_clay - to_bake_first == 0 || (Product_store.Full() && to_bake_first >= furnace_capacity / 2)) // Baking with residual products
    {
        log(INFO) << std::endl <<  to_bake_first << " PRODUCTS DRIED UP. 1ST BAKING PROCESS STARTED IN TIME " << Time
                  << std::endl;

        int baked_count = to_bake_first;
        used_clay -= to_bake_first;
        to_bake_first = 0;
        (new Baking_1st(baked_count))->Activate();
    } else if (Product_store.Full() && to_bake_second >= furnace_capacity / 2) {
        log(EMPTY) << std::endl;
        log(INFO) << std::endl <<  to_bake_second << " PRODUCTS GLAZED. 2ND BAKING PROCESS STARTED IN TIME " << Time
                  << std::endl;

        int baked_amount = to_bake_second;
        baked_second += to_bake_second;
        to_bake_second = 0;
        (new Baking_2nd(baked_amount))->Activate();
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
        Priority = 5;
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
    if (to_bake_second >= furnace_2nd_capacity)
    {
        log(EMPTY) << std::endl;
        log(INFO) << std::endl <<  to_bake_second << " PRODUCTS GLAZED. 2ND BAKING PROCESS STARTED IN TIME " << Time
                  << std::endl;
        to_bake_second -= furnace_2nd_capacity;
        baked_second += furnace_2nd_capacity;
        (new Baking_2nd(furnace_2nd_capacity))->Activate();
    }
    else if (baked_second + to_bake_second == *avail_clay || (Product_store.Full() && to_bake_second >= furnace_2nd_capacity / 2))
    {
        log(EMPTY) << std::endl;
        log(INFO) << std::endl <<  to_bake_second << " PRODUCTS GLAZED. 2ND BAKING PROCESS STARTED IN TIME " << Time
                  << std::endl;

        int baked_amount = to_bake_second;
        baked_second += to_bake_second;
        to_bake_second = 0;
        (new Baking_2nd(baked_amount))->Activate();
    } else if (Product_store.Full() && to_bake_first >= furnace_2nd_capacity / 2) {
        log(INFO) << std::endl <<  to_bake_first << " PRODUCTS DRIED UP. 1ST BAKING PROCESS STARTED IN TIME " << Time
                  << std::endl;

        int baked_count = to_bake_first;
        used_clay -= to_bake_first;
        to_bake_first = 0;
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
    int option_index = 0;
    static struct option long_options[] = {
            {"circles", required_argument, nullptr, 'c'},
            {"workers", required_argument, nullptr, 'w' },
            {"workshift", required_argument, nullptr,'t' },
            {"clay", required_argument, nullptr, 'l' },
            {"verbose", required_argument, nullptr, 'v'},
            {"furnace-capacity", required_argument,nullptr, 'x'},
            {"workshop-capacity", required_argument, nullptr,'z' },
            {"help", 0, nullptr, 0 }
    };
    while ((opt = getopt_long(argc, argv, "c:w:t:l:v:x:z:h", long_options, &option_index)) != -1)
    {
        switch (opt)
        {
            case 0:
            {
                switch (option_index)
                {
                    case 0:
                        check_arg(optarg, &pottery_circles_i);
                        break;
                    case 1:
                        check_arg(optarg, &usable_workers_i);
                        break;
                    case 2:
                        check_arg(optarg, &worktime_i);
                        break;
                    case 3:
                        check_arg(optarg, &avail_clay_i);
                        break;
                    case 4:
                        check_arg(optarg, &verbose);
                        break;
                    case 5:
                        check_arg(optarg, &furnace_capacity);
                        break;
                    case 6:
                        check_arg(optarg, &workshop_product_capacity);
                        break;
                    case 7:
                    {
                        print_help();
                        exit(EXIT_SUCCESS);
                    }
                    default:
                        return 1;
                }
                break;
            }
            case 'c':
                check_arg(optarg, &pottery_circles_i);
                break;
            case 'w':
                check_arg(optarg, &usable_workers_i);
                break;
            case 't':
                check_arg(optarg, &worktime_i);
                break;
            case 'l':
                check_arg(optarg, &avail_clay_i);
                break;
            case 'v':
                check_arg(optarg, &verbose);
                break;
            case 'x':
                check_arg(optarg, &furnace_capacity);
                break;
            case 'z':
                check_arg(optarg, &workshop_product_capacity);
                break;
            case 'h':
            {
                print_help();
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
    if (furnace_capacity > workshop_product_capacity)
    {
        std::cout << "Workshop capacity cannot be less than furnace capacity" << std::endl;
        exit(EXIT_FAILURE);
    }

    // Set parameters from arguments
    furnace_2nd_capacity = furnace_capacity / 2;
    Potter_circles.SetCapacity(*pottery_circles);
    Product_store.SetCapacity(workshop_product_capacity);

    RandomSeed(time(nullptr));
    Init(0.0, SIM_LEN);

    std::cout << std::string(23, '-') << "SIMULATION START" << std::string(23, '-') << std::endl;
    std::cout << "Number of workers: " << *usable_workers << std::endl;
    std::cout << "Number of pottery circles: " << *pottery_circles << std::endl;
    std::cout << "Work time in hours: " << *worktime << std::endl;
    std::cout << "Available clay: " << *avail_clay << std::endl;
    std::cout << "Furnace capacity: " << furnace_capacity << std::endl;
    std::cout << "Workshop capacity: " << workshop_product_capacity << std::endl;
    std::cout << "Verbose level: " << verbose << std::endl << std::endl;

    (new Clay_generator)->Activate();
    (new Freetime_generator)->Activate();
    for (int i = 0; i < *usable_workers; i++)
    {
        std::string worker = "Worker " + std::to_string(i);
        Workers[i].SetName(worker);
    }

    // Histograms initialization
    Clay_products.SetName("Clay products ready for baking");
    Clay_products.Init(0, WEEK, SIM_LEN/WEEK);

    Glazed_products.SetName("Glazed products ready for baking");
    Glazed_products.Init(0, WEEK, SIM_LEN/WEEK);

    Finished_products.SetName("Finished products");
    Finished_products.Init(0, WEEK, SIM_LEN/WEEK);

    Run();

    for (int i = 0; i < *usable_workers; i++)
    {
        Workers[i].Output();
    }
    Furnace.Output();
    WorkerQueue.Output();

    // Stores
    Potter_circles.Output();
    Product_store.Output();

    //Histograms
//    Clay_products.Output();
//    Glazed_products.Output();
    Finished_products.Output();


    std::cout << std::string(23, '-') << "SIMULATION END" << std::string(23, '-') << std::endl;
    return 0;
}
