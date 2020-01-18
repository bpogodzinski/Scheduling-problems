#include <Task.hpp>
#include <thirdParty/json.hpp>

#include <vector>
#include <fstream>
#include <iostream>
#include <cstdlib>

using Json = nlohmann::json;

namespace utils
{
    std::vector<Task> loadProblemInstance(const char* filepath)
    {
        std::ifstream file(filepath);
        Json jsonParser;
        file >> jsonParser;
        std::vector<Task> tasks;
        tasks.reserve(jsonParser.size());
        for (auto && task: jsonParser["tasks"].items())
        {
            tasks.push_back(Task(std::strtoul(task.key().c_str(), NULL, 10), task.value()["1"], task.value()["2"]));
        }
        return tasks;
    }
}