/// \author Alex Bazo
/// \author Marek Danco
/// \date 2022-12-05

#ifndef IMS_MAIN_H
#define IMS_MAIN_H
#include "simlib.h"
#endif //IMS_MAIN_H

/// \brief Function prints help string to stdout
void print_help();

/// \brief Helper function to activate worker queue
void ActivateWorkerQueue();

/// \brief Checks if string contains only digits
/// \param s string
/// \return True if digits only
bool check_is_number(const std::string &s);

/// \brief Function checks if giver program arguments are valid
/// \param opt_arg optarg parameter
/// \param arg pointer to specific global integer to store value of optarg parameter
void check_arg(char *opt_arg, int *arg);

// Processes
class Baking : public Process
{
protected:
    int baked_count;
    void Behavior() override;

public:
    explicit Baking(int baked_count) : Process()
    {
        this->baked_count = baked_count;
    }
};

class Baking_1st : public Baking
{
    void Behavior() override;

public:
    explicit Baking_1st(int baked_count) : Baking(baked_count) {}
};

class Baking_2nd : public Baking
{
    void Behavior() override;

public:
    explicit Baking_2nd(int baked_count) : Baking(baked_count) {}
};

class Clay_product : public Process
{
    void Behavior() override;
};

class Finished_product : public Process
{
    int work_period;
    void Behavior() override;
public:
    explicit Finished_product(int work_period) : Process() {
        this->work_period = work_period;
    }
};

class Freetime : public Process
{
    void Behavior() override;
    int workerNumber;
public:
    explicit Freetime(int worker_number) : Process() {
        this->workerNumber = worker_number;
    }
};

// Events
class Clay_generator : public Event
{
    void Behavior() override;
};

class Freetime_generator : public Event
{
    void Behavior() override;
};


