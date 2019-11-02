#pragma once

#include "DatabaseAccess.h"

class DataAccessTest
{
public:
	DataAccessTest() {};
	~DataAccessTest() {};

	void createDataBase();
	void addRecords();
private:
	DatabaseAccess _dataAccess;
};