#pragma once



#include<iostream>
#include <fstream>
#include<string>
#include<sstream>
#include<vector>
#include<algorithm>
#include "CashUnitStructure.h"
#include<map>
#include <thread>
#include <limits>
#include<numeric>
#include<iomanip>


class CashOutModule {


public: 
	static LPCUINFO cuInfo;

public :

	CashOutModule();

	int withdrawalFlow();

	LPCUINFO getCuInfo();

	void printCuInfo();

	bool getwidrawalStatus();

	std::string getSopPassword();

	~CashOutModule();


private:

	void init();

	void readCuinfoData(std::vector<std::vector<std::string>>& vec);

	LPCASHUNIT* prepareCassetteData(std::vector<std::vector<std::string>>& vec);
	void printCassetteData(const CASHUNIT& cass);

	bool updateCashCounter(std::map<int, int, std::greater<int>>& denoMix);

	bool updateRetractCounter(std::map<int, int, std::greater<int>>& denoMix);

	int greedyAlgo(std::vector<int>& arr, int sum, std::map<int, int, std::greater<int>>& denoMix);

	int greedyAlgo_limit(std::vector<std::pair<int, int>> denolistwithLimits, int sum, std::map<int, int, std::greater<int>>& denoMix);


	void validDenoAvailable(std::vector<int>& denoList, std::vector<std::pair<int, int>>& denolistwithLimits);

	bool takeCash();


private:
	std::string password;
	int dispenserStatus = false;

};