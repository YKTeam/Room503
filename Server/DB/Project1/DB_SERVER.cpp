#include "stdafx.h"

int main()
{
	MainDB* db = new MainDB;


	if (db->Initialize())
		db->Connect_DB();
	else
		std::cout << "fail" << std::endl;
	
}