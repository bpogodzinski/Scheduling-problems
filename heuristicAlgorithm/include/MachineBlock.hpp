#pragma once

enum BlockType{
    OPERATION,
    MAINTENANCE
};

class MachineBlock
{
private:
    BlockType blockType;
    unsigned int start;
    unsigned int end;
    unsigned int taskNumber;
    unsigned int operationNumber;    
public:
    MachineBlock(const unsigned int start, const unsigned int end, const unsigned int taskNumber, const unsigned int operationNumber, const BlockType &blockType);
    unsigned int getStart();
    unsigned int getLength();
    unsigned int getEnd();
    unsigned int getTaskNumber();
    unsigned int getOperationNumber(); 
};

MachineBlock::MachineBlock(const unsigned int start, const unsigned int end, const unsigned int taskNumber, const unsigned int operationNumber, const BlockType &blockType)
{
    this->start = start;
    this->end = end;
    this->taskNumber = taskNumber;
    this->operationNumber = operationNumber;
    this->blockType = blockType;
}

unsigned int MachineBlock::getEnd(){
    return this->end;
}

unsigned int MachineBlock::getLength(){
    return this->end - this->start;
}

unsigned int MachineBlock::getStart(){
    return this->start;
}

unsigned int MachineBlock::getOperationNumber(){
    return this->operationNumber;
}

unsigned int MachineBlock::getTaskNumber(){
    return this->taskNumber;
}
