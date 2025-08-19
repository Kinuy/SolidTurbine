#pragma once
#include "ITurbine.h"
#include "OperationConditions.h"
#include "OperationDataLogger.h"

// TODO: Documentation
class OperationAnalyzer
{
private:

	OperationDataLogger* logger;

public:

	OperationAnalyzer(OperationDataLogger* logger);

	void analyzePerformance(ITurbine& turbine, OperationConditions& operation);

};

