#pragma once

#include <Task.hpp>

#include <exception>
#include <vector>

class ProblemInstance
{
private:
    unsigned int maintenanceLength;
    unsigned int maintenancePeriod;
    unsigned int neighbourSearchCount;
    unsigned int algorithmRetries;
    float operationRenewPunishmentFactor;
    std::vector<Task> tasks;
public:
    ProblemInstance(unsigned int maintenanceLength, unsigned int maintenancePeriod, unsigned int neighbourSearchCount, unsigned int algorithmRetries, float operationRenewPunishmentFactor, const std::vector<Task> &tasks);
    unsigned int getMaintenanceLength();
    unsigned int getMaintenancePeriod();
    unsigned int getNeighbourSearchCount();
    unsigned int getAlgorithmRetries();
    float getOperationRenewPunishmentFactor();
    ~ProblemInstance();
};

ProblemInstance::ProblemInstance(unsigned int maintenanceLength, unsigned int maintenancePeriod, unsigned int neighbourSearchCount, unsigned int algorithmRetries, float operationRenewPunishmentFactor, const std::vector<Task> &tasks)
{
    this->maintenanceLength = maintenanceLength;
    this->maintenancePeriod = maintenancePeriod;
    this->neighbourSearchCount = neighbourSearchCount;
    this->algorithmRetries = algorithmRetries;
    this->tasks = tasks;
    if ( 0 < operationRenewPunishmentFactor && operationRenewPunishmentFactor < 1 )
        this->operationRenewPunishmentFactor = operationRenewPunishmentFactor;
    else
        throw std::logic_error("Punishment factor not between 0 < x < 1");
}

unsigned int ProblemInstance::getMaintenanceLength(){
    return this->maintenanceLength;
}

unsigned int ProblemInstance::getMaintenancePeriod(){
    return this->maintenancePeriod;
}

unsigned int ProblemInstance::getNeighbourSearchCount(){
    return this->neighbourSearchCount;
}

unsigned int ProblemInstance::getAlgorithmRetries(){
    return this->algorithmRetries;
}

float ProblemInstance::getOperationRenewPunishmentFactor(){
    return this->operationRenewPunishmentFactor;
}



ProblemInstance::~ProblemInstance()
{
}
