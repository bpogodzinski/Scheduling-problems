#include <Task.hpp>
#include <thirdParty/json.hpp>
#include <ProblemInstance.hpp>

#include <vector>
#include <fstream>
#include <iostream>
#include <cstdlib>
#include <algorithm>
#include <random>

namespace utils
{
    using Json = nlohmann::json;

    ProblemInstance loadProblemInstance(const char* filepath)
    {
        std::ifstream file(filepath);
        Json jsonParser;
        file >> jsonParser;
        std::vector<Task> tasks;
        tasks.reserve(jsonParser.size());
        for (auto && task: jsonParser["tasks"].items())
            tasks.push_back(Task(std::strtoul(task.key().c_str(), NULL, 10), task.value()["1"], task.value()["2"]));

        return ProblemInstance(jsonParser["maintenanceLength"], jsonParser["maintenancePeriod"], jsonParser["neighbourSearchCount"], jsonParser["algorithmRetries"], jsonParser["operationRenewPunishmentFactor"], tasks);
    }
}

namespace tabuSearch
{
    std::random_device rd;
    std::mt19937 randomGenerator(rd());

    void randomizeOrder(std::vector<Task> &tasks)
    {
        std::shuffle(tasks.begin(), tasks.end(), randomGenerator);
    }
}
