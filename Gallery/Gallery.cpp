#include <iostream>
#include <string>
#include "DatabaseAccess.h"
#include "DataAccessTest.h"
#include "AlbumManager.h"
#include "Gallery.h"

int main()
{
	// initialization data access
	DatabaseAccess dataAccess;
	// initialize album manager
	AlbumManager albumManager(dataAccess);

	std::string albumName;
	std::cout << "Welcome to Gallery!" << std::endl;
	std::cout << "===================" << std::endl;
	std::cout << "Type " << HELP << " to a list of all supported commands" << std::endl;
	
	do
	{
		int commandNumber = getCommandNumberFromUser();
		
		try	
		{
			albumManager.executeCommand(static_cast<CommandType>(commandNumber));
		} 
		catch (std::exception& e) 
		{	
			std::cout << e.what() << std::endl;
		}
	}
	while (true);

	/*DataAccessTest h;

	h.createDataBase();
	h.addRecords();*/
}