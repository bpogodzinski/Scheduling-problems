#include <../include/Task.hpp>
#include <../include/Utils.hpp>
#include <../include/ProblemInstance.hpp>

#include <iostream>
#include <vector>

int main(int argc, char const *argv[])
{
    const char* filepath = argv[1];
    
    ProblemInstance settings = utils::loadProblemInstance(filepath);
    return 0;
}
