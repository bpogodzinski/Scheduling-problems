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
#include <queue>

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
    
    friend bool operator == (MachineBlock x, MachineBlock y)
    {
        return x.start == y.start && x.length == y.length && x.end == y.end && x.taskNumber == y.taskNumber && x.machineNumber == y.machineNumber && x.blockType == y.blockType;
    }
    friend bool operator < (MachineBlock x, MachineBlock y)
    {
        if(x.machineNumber == y.machineNumber) return x.taskNumber < y.taskNumber;
        else return x.machineNumber < y.machineNumber;
    }
};

struct Solution
{
    std::vector<MachineBlock> machine1;
    std::vector<MachineBlock> machine2;

    std::vector<MachineBlock>* getMachine(utils::MachineNumber machineNumber)
    {
       return machineNumber == utils::MACHINE1 ?  &machine1 : &machine2;
    }

    std::string toString()
    {
        std::string output;
        for (auto &&block : machine1)
        {
            std::string start = std::to_string(block.start);
            std::string type = block.blockType == utils::OPERATION ? std::to_string(block.taskNumber) : "M";
            std::string end = std::to_string(block.end);
            output += start + " " + type + " " + end + "|";
        }
        output += '\n';
        for (auto &&block : machine2)
        {
            std::string start = std::to_string(block.start);
            std::string type = block.blockType == utils::OPERATION ? std::to_string(block.taskNumber) : "M";
            std::string end = std::to_string(block.end);
            output += start + " " + type + " " + end + "|";
        }
        output += '\n';
        return output;
    }

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
        while(!blocks.empty())
        {
            MachineBlock candidate = blocks.front();
            blocks.pop_front();
            addOrderedBlockToMachine(candidate);
        }
        return *this;
    }

    bool isBlockValidToPutOnMachine(const MachineBlock &candidate)
    {
        assert(candidate.blockType == utils::OPERATION);
        auto correspondingOperation = findCorrespondingOperation(candidate);
        if(correspondingOperation.has_value())
        {
            unsigned int candidateStartTime = (getMachine(candidate.machineNumber)->empty()) ? 0 : getMachine(candidate.machineNumber)->back().end;
            MachineBlock tempMB = {candidateStartTime, candidate.length};
            return !areBlocksColliding(tempMB, *correspondingOperation.value());
        }
        else return true;
    }

    std::optional<MachineBlock*> findCorrespondingOperation(const MachineBlock &operation)
    {
        assert(operation.blockType == utils::OPERATION);

        utils::MachineNumber machineNumberToSearch = (operation.machineNumber == utils::MACHINE1) ? utils::MACHINE2 : utils::MACHINE1;
        auto machineToSearch = getMachine(machineNumberToSearch);

        auto itCorrespondingOperation = std::find_if(machineToSearch->begin(), machineToSearch->end(),[&](MachineBlock &x){ return x.taskNumber == operation.taskNumber; });
        if(itCorrespondingOperation != machineToSearch->end())
            return std::optional<MachineBlock*>(&(*itCorrespondingOperation));
        else return std::nullopt;
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

        auto machine = getMachine(candidate.machineNumber);

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

    void addOrderedBlockToMachine(MachineBlock &candidate)
    {
        assert(candidate.blockType == utils::OPERATION);

        auto machine = getMachine(candidate.machineNumber);

        if (doesOperationFitBeforeMaintenance(candidate))
        {
            if(isBlockValidToPutOnMachine(candidate))
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
                auto correspondingOperation = findCorrespondingOperation(candidate).value();
                MachineBlock tempMB = {correspondingOperation->end, candidate.length, correspondingOperation->end + candidate.length, candidate.taskNumber, candidate.machineNumber, candidate.blockType}; 
                if(doesOperationFitBeforeMaintenance(tempMB, true))
                {
                    candidate.start = correspondingOperation->end;
                    candidate.end = candidate.length + candidate.start;
                    machine->push_back(candidate);
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
                    return addOrderedBlockToMachine(candidate);
                }
                
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
             return addOrderedBlockToMachine(candidate);
        }
    }

    bool doesOperationFitBeforeMaintenance(MachineBlock &candidate)
    {
        return getTimeToNextMaintenance(candidate.machineNumber) >= candidate.length;
    }
    bool doesOperationFitBeforeMaintenance(MachineBlock &candidate, bool flag)
    {
        return getTimeToNextMaintenance(candidate) >= candidate.length;
    }

    unsigned int getTimeToNextMaintenance(utils::MachineNumber &machineNumber)
    {
        auto lastMaintenance = getLastMachineBlock(machineNumber, utils::MAINTENANCE);
        auto lastOperation = getLastMachineBlock(machineNumber, utils::OPERATION);
        unsigned int lastMaintenanceEndTime = lastMaintenance.has_value() ? lastMaintenance.value().end : 0;
        unsigned int lastOperationEndTime = lastOperation.has_value() ? lastOperation.value().end : 0;

        return utils::settings->maintenancePeriod - abs(lastOperationEndTime - lastMaintenanceEndTime);
    }
    unsigned int getTimeToNextMaintenance(MachineBlock &candidate)
    {
        auto lastMaintenance = getLastMachineBlock(candidate.machineNumber, utils::MAINTENANCE);
        unsigned int lastMaintenanceEndTime = lastMaintenance.has_value() ? lastMaintenance.value().end : 0;
        unsigned int lastOperationEndTime = candidate.start;

        return utils::settings->maintenancePeriod - abs(lastOperationEndTime - lastMaintenanceEndTime);
    }

    unsigned int getCmax()
    {
        auto compareEndTime = [](const MachineBlock& x, const MachineBlock& y){ return x.end < y.end; };
        auto lastM1operation = getLastMachineBlock(utils::MACHINE1, utils::OPERATION);
        auto lastM1operationValue = lastM1operation.value();
        auto lastM2operation = getLastMachineBlock(utils::MACHINE2, utils::OPERATION);
        auto lastM2operationValue = lastM2operation.value();
        auto lastOperation = std::max(lastM1operationValue, lastM2operationValue, compareEndTime);
        return lastOperation.end;
    }

    std::optional<MachineBlock> getLastMachineBlock(const utils::MachineNumber &machine, const utils::BlockType &block)
    {
        auto machineToSearch = machine == utils::MACHINE1 ? machine1 : machine2;
        auto it = std::find_if(machineToSearch.rbegin(), machineToSearch.rend(), [&](MachineBlock &x){ return x.blockType == block;});
        if (it != machineToSearch.rend())
            return std::optional<MachineBlock>(*it);
        else
            return std::nullopt;
    }
};

struct Node 
{ 
    unsigned int level, boundCmax;
    Solution solution; 
}; 

class OptimalSearch
{
private:
    std::random_device rd;
    std::mt19937 randomGenerator;
    ProblemInstance* settings;
    
public:
    Solution bestSolution;
    Solution currentSolution;
    unsigned int bestCmax;
    OptimalSearch(ProblemInstance &settings):settings(&settings), randomGenerator(rd()){}

    std::list<MachineBlock> createRandomOrder()
    {   
        unsigned int numberOfMachines = 2;
        std::vector<MachineBlock> tmpVector;
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

    OptimalSearch& createInitialSolution()
    {
        currentSolution.machine1.clear();
        currentSolution.machine2.clear();
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

    float calculateSD(std::vector<int> &localCmaxs)
    {
        if(localCmaxs.size() < 300) return 999;
        float sum = 0.0, mean, standardDeviation = 0.0;
        sum = std::accumulate(localCmaxs.begin(), localCmaxs.end(), 0);
        mean = sum/localCmaxs.size();
        for (auto &&cmax : localCmaxs)
        {
            standardDeviation += pow(cmax - mean, 2);
        }
        
        return sqrt(standardDeviation / localCmaxs.size());
    }

    void fullSearch(std::vector<MachineBlock> sortedOrder)
    {
        unsigned long long int count = 1;
        do
        {
            std::list<MachineBlock> tempList(sortedOrder.begin(), sortedOrder.end());
            currentSolution.orderedSolution(tempList);
            unsigned int currentCmax = currentSolution.getCmax();
            if(currentCmax < bestCmax) 
            {
                bestSolution = currentSolution;
                bestCmax = currentCmax;
            }
            currentSolution.machine1.clear();
            currentSolution.machine2.clear();
            std::cout << "\r" << count++;
        } while (std::next_permutation(sortedOrder.begin(), sortedOrder.end()));
        std::cout << "\n";
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
    OptimalSearch algorithm(settings);
    algorithm.createInitialSolution();
    algorithm.bestSolution = algorithm.currentSolution;
    algorithm.bestCmax = algorithm.bestSolution.getCmax();
    auto order = algorithm.createRandomOrder();
    std::vector<MachineBlock> vectorInitialOrder(order.begin(), order.end());
    std::sort(vectorInitialOrder.begin(), vectorInitialOrder.end());
    algorithm.fullSearch(vectorInitialOrder);
    std::cout << algorithm.bestSolution.toString() << std::endl;

    return 0;
}

// ProblemInstance settings = loadProblemInstance(filepath);
    // utils::settings = &settings;
    // OptimalSearch algorithm(settings);
    // algorithm.createInitialSolution();
    // algorithm.bestSolution = algorithm.currentSolution;
    // algorithm.bestCmax = algorithm.bestSolution.getCmax();
    // auto order = algorithm.createRandomOrder();
    // std::vector<MachineBlock> vectorInitialOrder(order.begin(), order.end());
    // std::sort(vectorInitialOrder.begin(), vectorInitialOrder.end());
    // algorithm.fullSearch(vectorInitialOrder);
    // std::cout << algorithm.bestSolution.toString() << std::endl;

    // MachineBlock m1;
    // m1.length = 8;
    // m1.taskNumber = 1;
    // m1.machineNumber = utils::MACHINE1;
    // m1.blockType = utils::OPERATION;
    // MachineBlock m2;
    // m2.length = 3;
    // m2.taskNumber = 2;
    // m2.machineNumber = utils::MACHINE1;
    // m2.blockType = utils::OPERATION;
    // MachineBlock m3;
    // m3.length = 9;
    // m3.taskNumber = 4;
    // m3.machineNumber = utils::MACHINE1;
    // m3.blockType = utils::OPERATION;
    // MachineBlock m4;
    // m4.length = 7;
    // m4.taskNumber = 2;
    // m4.machineNumber = utils::MACHINE2;
    // m4.blockType = utils::OPERATION;
    // MachineBlock m5;
    // m5.length = 3;
    // m5.taskNumber = 1;
    // m5.machineNumber = utils::MACHINE2;
    // m5.blockType = utils::OPERATION;
    // MachineBlock m6;
    // m6.length = 5;
    // m6.taskNumber = 3;
    // m6.machineNumber = utils::MACHINE2;
    // m6.blockType = utils::OPERATION;
    // MachineBlock m7;
    // m7.length = 8;
    // m7.taskNumber = 5;
    // m7.machineNumber = utils::MACHINE2;
    // m7.blockType = utils::OPERATION;
    // MachineBlock m8;
    // m8.length = 8;
    // m8.taskNumber = 5;
    // m8.machineNumber = utils::MACHINE1;
    // m8.blockType = utils::OPERATION;
    // MachineBlock m9;
    // m9.length = 5;
    // m9.taskNumber = 3;
    // m9.machineNumber = utils::MACHINE1;
    // m9.blockType = utils::OPERATION;
    // MachineBlock m10;
    // m10.length = 2;
    // m10.taskNumber = 4;
    // m10.machineNumber = utils::MACHINE2;
    // m10.blockType = utils::OPERATION;

    // temp.push_back(m1);
    // temp.push_back(m2);
    // temp.push_back(m3);
    // temp.push_back(m4);
    // temp.push_back(m5);
    // temp.push_back(m6);
    // temp.push_back(m7);
    // temp.push_back(m8);
    // temp.push_back(m9);
    // temp.push_back(m10);