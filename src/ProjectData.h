#pragma once

#include <string>

// ProjectData - responsible for storing project specific data
struct ProjectData
{
	std::string projectId;
	std::string projectName;
	std::string revision;
	std::string date;
	std::string customer;
	std::string engineer;

	ProjectData(
		std::string projectId = "XId", 
		std::string projectName = "Xname", 
		std::string revision = "Xrevision", 
		std::string date = "Xdate", 
		std::string customer = "Xcoustomer", 
		std::string engineer = "Xengineer") 
		:
		projectId(projectId),
		projectName(projectName),
		revision(revision), 
		date(date), 
		customer(customer), 
		engineer(engineer) {
	}
};