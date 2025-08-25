#include "DXFHandleGenerator.h"



DXFHandleGenerator::DXFHandleGenerator(int startHandle) : currentHandle(startHandle) {}


std::string DXFHandleGenerator::getNextHandle()  {
    return std::to_string(currentHandle++);
}