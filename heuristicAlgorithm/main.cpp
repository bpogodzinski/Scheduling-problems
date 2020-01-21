#include "include/thirdParty/json.hpp"

#include <random>
#include <algorithm>
#include <vector>
#include <map>
#include <list>
#include <cstdlib>
#include <fstream>

using Json = nlohmann::json;

namespace utils
{
    enum MachineNumber
    {
        MACHINE1,
        MACHINE2
    };

    enum BlockType
    {
        OPERATION,
        MAINTENANCE
    };
}

struct MachineBlock
{
    unsigned int start = 0;
    unsigned int end = 0;
    unsigned int length = 0;
    unsigned int taskNumber = 0;
    utils::MachineNumber machineNumber;    
    utils::BlockType blockType;
    
};

struct Solution
{
    std::vector<MachineBlock> machine1;
    std::vector<MachineBlock> machine2;
    std::map<utils::MachineNumber, std::vector<MachineBlock>*> machineMap = { {utils::MACHINE1, &this->machine1}, {utils::MACHINE2, &this->machine2} };

    Solution& randomSolution(std::list<MachineBlock> blocks)
    {
        while (!blocks.empty())
        {
            MachineBlock candidate = blocks.front();
            blocks.pop_front();
            isBlockValidToPutOnMachine(candidate) ? addBlockToMachine(candidate) : blocks.push_back(candidate);
        }

        return *this;        
    }

    bool isBlockValidToPutOnMachine(const MachineBlock &candidate)
    {
        assert(candidate.blockType == utils::OPERATION);

        utils::MachineNumber machineToSearch = (candidate.machineNumber == utils::MACHINE1) ? utils::MACHINE2 : utils::MACHINE1;
        auto itCorrespondingOperation = std::find_if(machineMap[machineToSearch]->begin(), machineMap[machineToSearch]->end(),[&](MachineBlock &x){ return x.taskNumber == candidate.taskNumber; });
        if(itCorrespondingOperation != machineMap[machineToSearch]->end())
        {
            unsigned int candidateStartTime = (machineMap[candidate.machineNumber]->empty()) ? 0 : machineMap[candidate.machineNumber]->back().end;
            return areBlocksColliding(candidate.length, candidateStartTime, *itCorrespondingOperation) ? false : true;
        }
        else return true;
    }

    bool areBlocksColliding(unsigned int candidateLength, unsigned int candidateStartTime, const MachineBlock &correspondingOperation)
    {
        if(candidateStartTime > correspondingOperation.start){
            return (candidateStartTime - correspondingOperation.start) >= correspondingOperation.length;
        }
        else if(correspondingOperation.start > candidateStartTime){
            return (correspondingOperation.start - candidateStartTime) >= candidateLength;
        }
        else return true;
    }

    //TODO! : finish both functions
    void addBlockToMachine(MachineBlock &candidate)
    {
        if (doesOperationFitBeforeMaintenance(candidate))
        {
            machineMap[candidate.machineNumber]->push_back(candidate);
        }
        else
        {
            
        }
    }

    MachineBlock* getLastMachineBlock(const utils::MachineNumber &machine, const utils::BlockType &block)
    {
        auto it = std::find_if(machineMap[machine]->rbegin(), machineMap[machine]->rend(), [&](MachineBlock &x){ return x.blockType == block;});
        if (it != machineMap[machine]->rend())
            return &(*it);
        else
            return nullptr;
    }
};

struct Task
{
    unsigned int taskNumber = 0;
    unsigned int machine1OperationLength = 0;
    unsigned int machine2OperationLength = 0;
    std::map<utils::MachineNumber, unsigned int> machineLengthMap = { {utils::MACHINE1, machine1OperationLength}, {utils::MACHINE2, machine2OperationLength} };

    Task(const unsigned int taskNumber, const unsigned int machine1OperationLength, const unsigned int machine2OperationLength)
    :taskNumber(taskNumber), machine1OperationLength(machine1OperationLength), machine2OperationLength(machine2OperationLength) {}
};

struct ProblemInstance
{
    unsigned int maintenanceLength;
    unsigned int maintenancePeriod;
    unsigned int neighbourSearchCount;
    unsigned int algorithmRetries;
    float operationRenewPunishmentFactor;
    std::vector<Task> tasks;

    ProblemInstance(unsigned int maintenanceLength, unsigned int maintenancePeriod, unsigned int neighbourSearchCount, unsigned int algorithmRetries, float operationRenewPunishmentFactor, const std::vector<Task> &tasks)
    :maintenanceLength(maintenanceLength), maintenancePeriod(maintenancePeriod), neighbourSearchCount(neighbourSearchCount), algorithmRetries(algorithmRetries), tasks(tasks)
    {
        if ( 0 < operationRenewPunishmentFactor && operationRenewPunishmentFactor < 1 )
            this->operationRenewPunishmentFactor = operationRenewPunishmentFactor;
        else
            throw std::logic_error("Punishment factor not between 0 < x < 1");
    }
};

class TabuSearch
{
private:
    std::random_device rd;
    std::mt19937 randomGenerator;

    ProblemInstance* settings;
    Solution bestSolution;
    Solution currentSolution;

public:
    TabuSearch(ProblemInstance &settings):settings(&settings), randomGenerator(rd()){}

    ProblemInstance getSettings()
    {
        return *this->settings;
    }

    std::list<MachineBlock> createRandomOrder()
    {   
        unsigned int numberOfMachines = 2;
        std::vector<MachineBlock> tmpVector;
        tmpVector.reserve(settings->tasks.size() * numberOfMachines);
        for (auto &&task : settings->tasks)
        {
            MachineBlock block1, block2;
            block1.blockType = utils::OPERATION;
            block1.machineNumber = utils::MACHINE1;
            block1.length = task.machineLengthMap[utils::MACHINE1];
            block1.taskNumber = task.taskNumber;

            block2.blockType = utils::OPERATION;
            block2.machineNumber = utils::MACHINE2;
            block2.length = task.machineLengthMap[utils::MACHINE2];
            block2.taskNumber = task.taskNumber;
            
            tmpVector.push_back(block1);
            tmpVector.push_back(block2);
        }
        std::shuffle(tmpVector.begin(), tmpVector.end(), randomGenerator);
        std::list<MachineBlock> machineBlockList(tmpVector.begin(), tmpVector.end());
        return machineBlockList;
    }

    TabuSearch& createInitialSolution()
    {
        std::list<MachineBlock> blocks = this->createRandomOrder();
        currentSolution.randomSolution(blocks);
        return *this;
    }

};

ProblemInstance loadProblemInstance(const char* filepath)
{
    std::ifstream file(filepath);
    Json jsonParser;
    file >> jsonParser;
    std::vector<Task> tasks;
    tasks.reserve(jsonParser.size());
    for (auto && task: jsonParser["tasks"].items())
        tasks.push_back(Task(std::strtoul(task.key().c_str(), NULL, 10), task.value()["1"], task.value()["2"]));

    return ProblemInstance(jsonParser["maintenanceLength"], jsonParser["maintenancePeriod"], jsonParser["neighbourSearchCount"],
        jsonParser["algorithmRetries"], jsonParser["operationRenewPunishmentFactor"], tasks);
}


int main(int argc, char const *argv[])
{
    const char* filepath = argv[1];

    ProblemInstance settings = loadProblemInstance(filepath);
    TabuSearch algorithm(settings);
    algorithm.createInitialSolution();


    return 0;
}
