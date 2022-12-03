//
// Created by ubuntu on 02/12/22.
//

#ifndef IMS_MAIN_H
#define IMS_MAIN_H
#include "simlib.h"
#endif //IMS_MAIN_H

//Processes
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

class Baking_2nd : public Baking
{
    void Behavior() override;

public:
    explicit Baking_2nd(int baked_count) : Baking(baked_count) {}
};

class Finished_product : public Process
{
    void Behavior() override;
    int work_period;

public:
    explicit Finished_product(int work_period) : Process() {
        this->work_period = work_period;
    }
};

class Baking_1st : public Baking
{
    void Behavior() override;
public:
    explicit Baking_1st(int baked_count) : Baking(baked_count) {}
};

class Clay_product : public Process
{
    void Behavior() override;
};

class Freetime : public Process
{
    int workerNumber;
    void Behavior() override;

public:
    explicit Freetime(int workerNumber) : Process()
    {
        this->workerNumber = workerNumber;
    }
};

// Events
class ClayGenerator : public Event
{
    void Behavior() override;
};

class FreetimeGenerator : public Event
{
    void Behavior() override;
};