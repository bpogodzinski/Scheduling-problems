#include "include/thirdParty/json.hpp"

#include <random>
#include <algorithm>
#include <vector>
#include <map>
#include <list>
#include <cstdlib>
#include <fstream>
#include <optional>
#include <iostream>

using Json = nlohmann::json;

struct ProblemInstance;

namespace utils
{
    ProblemInstance *settings;

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
    unsigned int candidateListSize = 5;
    unsigned int tabuListSize = 4;
    float operationRenewPunishmentFactor;
    std::vector<Task> tasks;

    ProblemInstance(unsigned int maintenanceLength, unsigned int maintenancePeriod, unsigned int neighbourSearchCount, unsigned int algorithmRetries, float operationRenewPunishmentFactor, const std::vector<Task> &tasks)
    :maintenanceLength(maintenanceLength), maintenancePeriod(maintenancePeriod), neighbourSearchCount(neighbourSearchCount), algorithmRetries(algorithmRetries), tasks(tasks)
    {
        assert( 0 < operationRenewPunishmentFactor && operationRenewPunishmentFactor < 1 );
        this->operationRenewPunishmentFactor = operationRenewPunishmentFactor;
    }
};

struct MachineBlock
{
    unsigned int start = 0;
    unsigned int length = 0;
    unsigned int end = 0;
    unsigned int taskNumber = 0;
    utils::MachineNumber machineNumber;    
    utils::BlockType blockType;
    
    friend bool operator == (const MachineBlock& x, const MachineBlock& y)
    {
        return x.start == y.start && x.length == y.length && x.end == y.end && x.taskNumber == y.taskNumber && x.machineNumber == y.machineNumber && x.blockType == y.blockType;
    }
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

    Solution& orderedSolution(std::list<MachineBlock> blocks)
    {
        while(!blocks.empty)
        {
            MachineBlock candidate = blocks.front();
            blocks.pop_front();
        }
    }

    bool isBlockValidToPutOnMachine(const MachineBlock &candidate)
    {
        assert(candidate.blockType == utils::OPERATION);

        utils::MachineNumber machineNumberToSearch = (candidate.machineNumber == utils::MACHINE1) ? utils::MACHINE2 : utils::MACHINE1;
        auto machineToSearch = machineMap[machineNumberToSearch];

        auto itCorrespondingOperation = std::find_if(machineToSearch->begin(), machineToSearch->end(),[&](MachineBlock &x){ return x.taskNumber == candidate.taskNumber; });
        if(itCorrespondingOperation != machineToSearch->end())
        {
            unsigned int candidateStartTime = (machineMap[candidate.machineNumber]->empty()) ? 0 : machineMap[candidate.machineNumber]->back().end;
            MachineBlock tempMB = {candidateStartTime, candidate.length};
            return !areBlocksColliding(tempMB, *itCorrespondingOperation);
        }
        else return true;
    }

    bool areBlocksColliding(const MachineBlock &operation, const MachineBlock &correspondingOperation)
    {
        auto compareStartTime = [](MachineBlock x, MachineBlock y){ return x.start < y.start; };
        auto furtherOperationStartTime = std::max(operation, correspondingOperation, compareStartTime).start;
        auto closerOperationStartTime = std::min(operation, correspondingOperation, compareStartTime).start;
        auto closerOperationLength = std::min(operation, correspondingOperation, compareStartTime).length;
        return !(furtherOperationStartTime - closerOperationStartTime >= closerOperationLength);
    }

    void addBlockToMachine(MachineBlock &candidate)
    {
        assert(candidate.blockType == utils::OPERATION);

        auto machine = machineMap[candidate.machineNumber];

        if (doesOperationFitBeforeMaintenance(candidate))
        {
            if(machine->empty())
            {
                candidate.start = 0;
                candidate.end = candidate.length;
                machine->push_back(candidate);
            }
            else
            {
                candidate.start = machine->back().end;
                candidate.end = candidate.start + candidate.length;
                machine->push_back(candidate);
            }
        }
        else
        {
             MachineBlock maintenance;
             maintenance.blockType = utils::MAINTENANCE;
             maintenance.start = machine->back().end;
             maintenance.length = utils::settings->maintenanceLength;
             maintenance.end = maintenance.start + maintenance.length;
             maintenance.machineNumber = candidate.machineNumber;

             machine->push_back(maintenance);
             return addBlockToMachine(candidate);
        }
    }

    bool doesOperationFitBeforeMaintenance(MachineBlock &candidate)
    {
        return getTimeToNextMaintenance(candidate.machineNumber) >= candidate.length;
    }

    unsigned int getTimeToNextMaintenance(utils::MachineNumber &machineNumber)
    {
        auto machine = machineMap[machineNumber];
        auto lastMaintenance = getLastMachineBlock(machineNumber, utils::MAINTENANCE);
        auto lastOperation = getLastMachineBlock(machineNumber, utils::OPERATION);
        unsigned int lastMaintenanceEndTime = lastMaintenance.has_value() ? lastMaintenance.value()->end : 0;
        unsigned int lastOperationEndTime = lastOperation.has_value() ? lastOperation.value()->end : 0;

        return utils::settings->maintenancePeriod - abs(lastOperationEndTime - lastMaintenanceEndTime);
    }

    unsigned int getCmax()
    {
        auto compareEndTime = [](const MachineBlock* x, const MachineBlock* y){ return x->end < y->end; };
        auto lastOperation = std::max(getLastMachineBlock(utils::MACHINE1, utils::OPERATION).value(),getLastMachineBlock(utils::MACHINE1, utils::OPERATION).value(),compareEndTime);
        return lastOperation->end;
    }

    std::optional<MachineBlock*> getLastMachineBlock(const utils::MachineNumber &machine, const utils::BlockType &block)
    {
        auto it = std::find_if(machineMap[machine]->rbegin(), machineMap[machine]->rend(), [&](MachineBlock &x){ return x.blockType == block;});
        if (it != machineMap[machine]->rend())
            return std::optional<MachineBlock*>(&(*it));
        else
            return std::nullopt;
    }
};

bool operator== (std::pair<MachineBlock, MachineBlock>& x, std::pair<MachineBlock, MachineBlock>& y)
{
    auto b1 = x.first == y.first || x.first == y.second;
    auto b2 = x.second == y.first || x.second == y.second;
    return b1 && b2;
}

class TabuSearch
{
private:
    std::random_device rd;
    std::mt19937 randomGenerator;
    ProblemInstance* settings;
    
public:
    Solution bestSolution;
    Solution currentSolution;
    TabuSearch(ProblemInstance &settings):settings(&settings), randomGenerator(rd()){}

    std::list<MachineBlock> createRandomOrder()
    {   
        unsigned int numberOfMachines = 2;
        std::vector<MachineBlock> tmpVector;
        // tmpVector.reserve(settings->tasks.size() * numberOfMachines);
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

    std::list<MachineBlock> getBlocksOrder(Solution &solution)
    {
        std::vector<MachineBlock> tmpVector;
        tmpVector.insert(tmpVector.end(), solution.machine1.begin(), solution.machine1.end());
        tmpVector.insert(tmpVector.end(), solution.machine2.begin(), solution.machine2.end());
        std::list<MachineBlock> blocks(tmpVector.begin(), tmpVector.end());

        blocks.remove_if([](MachineBlock x){return x.blockType == utils::MAINTENANCE;});
        std::for_each(blocks.begin(), blocks.end(), [](MachineBlock &x){ x.start = 0; x.end = 0;});
        return blocks;
    }

    TabuSearch& createInitialSolution()
    {
        std::list<MachineBlock> blocks = this->createRandomOrder();
        currentSolution.randomSolution(blocks);
        return *this;
    }

    std::vector<std::pair<MachineBlock, MachineBlock>> generateCandidatesForSwap(std::list<MachineBlock> &blocks)
    {
        unsigned int candidatesCount = utils::settings->neighbourSearchCount;
        std::vector<std::pair<MachineBlock, MachineBlock>> swapCandidates;
        swapCandidates.reserve(candidatesCount);

        do
        {
            std::pair<MachineBlock, MachineBlock> swap = getRandomSwap(blocks);
            if(isSwapCandidateInSwapList(swap, swapCandidates)) continue;
            else swapCandidates.push_back(swap);
        }
        while (swapCandidates.size() != candidatesCount);

        return swapCandidates;
    }

    std::pair<MachineBlock, MachineBlock> getRandomSwap(const std::list<MachineBlock> &blocks)
    {
        std::vector<MachineBlock> tempVector(blocks.begin(), blocks.end());
        std::shuffle(tempVector.begin(), tempVector.end(), randomGenerator);
        MachineBlock first = tempVector.back();
        tempVector.pop_back();
        auto it = std::find_if(tempVector.begin(), tempVector.end(), [&](MachineBlock &x){ return x.machineNumber == first.machineNumber;});
        assert(it != tempVector.end());
        MachineBlock second = *it;
        return std::make_pair(first, second);
    }

    bool isSwapCandidateInSwapList(std::pair<MachineBlock, MachineBlock> swap,const std::vector<std::pair<MachineBlock, MachineBlock>> &swapList)
    {
        return std::any_of(swapList.begin(), swapList.end(), [&](std::pair<MachineBlock, MachineBlock> x){ return x == swap;});
    }

    std::list<MachineBlock> swap(std::pair<MachineBlock, MachineBlock> &pair, std::list<MachineBlock> list)
    {
        auto it1 = std::find(list.begin(), list.end(), pair.first);
        auto it2 = std::find(list.begin(), list.end(), pair.second);
        assert(it1 != list.end() && it2 != list.end());
        std::swap(*it1, *it2);
        return list;
    }

    void optimize()
    {
        std::list<MachineBlock> blocks = getBlocksOrder(currentSolution);
        auto swapList = generateCandidatesForSwap(blocks);
        for (auto &&pair : swapList)
        {
            auto swappedOrder = swap(pair, blocks);
            Solution solution;
            solution.orderedSolution(swappedOrder);
        }
        

        
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
    utils::settings = &settings;
    TabuSearch algorithm(settings);
    algorithm.createInitialSolution();
    algorithm.optimize();

    return 0;
}
