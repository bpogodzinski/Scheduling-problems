#pragma once

class Task
{
private:
    unsigned int taskNumber;
    unsigned int operation1Length;
    unsigned int operation2Length;
public:
    Task(const unsigned int taskNumber, const unsigned int operation1Length, const unsigned int operation2Length);
    ~Task();
};

Task::Task(const unsigned int taskNumber, const unsigned int operation1Length, const unsigned int operation2Length)
{
    this->operation1Length = operation1Length;
    this->operation2Length = operation2Length;
    this->taskNumber = taskNumber;
}

Task::~Task()
{
}
