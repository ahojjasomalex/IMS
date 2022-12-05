//
// Created by ubuntu on 02/12/22.
//

#ifndef IMS_MAIN_H
#define IMS_MAIN_H
#include "simlib.h"
#endif //IMS_MAIN_H

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