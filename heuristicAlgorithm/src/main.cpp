#include <../include/Task.hpp>
#include <../include/Utils.hpp>

#include <iostream>
#include <vector>

int main(int argc, char const *argv[])
{
    const char* filepath = argv[1];
    std::vector<Task> tasks = utils::loadProblemInstance(filepath);
    return 0;
}
