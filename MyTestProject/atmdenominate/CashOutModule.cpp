

#include "CashOutModule.h"
#include<thread>
#include<chrono>
#include <utility>


LPCUINFO CashOutModule::cuInfo;


int exp_factor = 100;

bool isTimeout = false;
std::string input;

enum CASHUNIT_TYPE {
    BILL = 0,
    REJECT,
    RETRACT
};

enum CASS_STATUS {
    ONLINE = 0,
    INOP
};



// construtor
CashOutModule::CashOutModule() {

    init();

    // hardcoded passward to view counter info 

    password = "1234";
}

// Initialize dispenser module
void CashOutModule::init() {

    cuInfo = getCuInfo();
}



// return SOP (standard operating & service) password
std::string CashOutModule::getSopPassword() {
    return password;
}

// Destructor to free up the resources 
// deletes cassette information objects
CashOutModule::~CashOutModule() {

    std::cout << "destroying cashout module resources....\n";

   // cdm_cash_unit** csslist = CashOutModule::cuInfo->lppList;

    int total_count = cuInfo->count;

    for (int i = 0; i < total_count; i++)
        delete cuInfo->lppList[i];

    delete cuInfo->lppList;

    std::cout << "destroy activity complete \n";

}


// Name : updateCashCounter : Update the cassette counters after each operation
// Param :
//  cuInfo : cassette information
//  denomix : denomination mix map for dispensed notes
// Return :
//  bool : If operation successful

bool CashOutModule::updateCashCounter(std::map<int, int, std::greater<int>>&denoMix) {

    bool rc = false;

    LPCASHUNIT* csslist = cuInfo->lppList;

    for (int i = 0; i < cuInfo->count; i++) {

        CASHUNIT& cassUnit = *csslist[i];

        if (cassUnit.Type == BILL) {
            int denom = cassUnit.denominations / exp_factor;

            if (denoMix.find(denom) != denoMix.end()) {

                rc = true;

                cassUnit.count -= denoMix[denom];

                // if count reached zero, make status not operational
                if (cassUnit.count <= 0) {

                    cassUnit.Status = INOP;
                }

            }
        }


    }

    return rc;

}


// Name : updateRetractCounter: Update the retract counters if customer doesn't take cash
// Param :
//  cuInfo : cassette information
//  denomix : denomination mix map for dispensed notes
// Return :
//  bool : If operation successful
bool CashOutModule::updateRetractCounter(std::map<int, int, std::greater<int>>& denoMix) {

    LPCASHUNIT* csslist = cuInfo->lppList;

    bool rc = false;

    int total_bills = std::accumulate(denoMix.begin(), denoMix.end(), 0,
        [](auto prev_sum, auto& entry) {
            return prev_sum + entry.second;
        });

    for (int i = 0; i < cuInfo->count; i++) {

        CASHUNIT& cassUnit = *csslist[i];

        if (cassUnit.Type == RETRACT) {
            if (cassUnit.Status == ONLINE) {
                cassUnit.count += total_bills;
                rc = true;
                std::cout << "Your notes have sucessfully retracted. please contact your bank to get it refunded\n";
                break;
            }
            else {
                std::cout << "Not able to retract, retract cassette is not operational" << "\n";
            }
        }
    }

    return rc;
}


// Name : greedyAlgo: Calculate the mix following greedy alogirith with honoring the limits placed on each denomination
// Param :
//  denolistwithLimits : cassette list with limits
//  sum : amount for which bill mix to be calculated
// denomix : deno mix map where result to be written
// Return :
//  int : If mix possible return 1 other wise 0

int CashOutModule::greedyAlgo_limit(std::vector<std::pair<int, int>> denolistwithLimits, int sum, std::map<int, int, std::greater<int>>& denoMix)
{
    int sum_original = sum;

    std::sort(denolistwithLimits.rbegin(), denolistwithLimits.rend());

    for (int i = 0; i < denolistwithLimits.size(); i++) {
        int curDeno = denolistwithLimits[i].first;
        int curLimit = denolistwithLimits[i].second;

        while (sum >= curDeno) {
            int cnt = sum / curDeno;
            int extra_cnt = 0;

            if (cnt > curLimit) {
                extra_cnt = cnt - curLimit;
                cnt = curLimit;
            }
            sum = (sum % curDeno) + (extra_cnt * curDeno);            
            denoMix[curDeno] = cnt;
            break;
        }
    }

    if (sum != 0) {

        int remaining_amt = sum;

        // calculate low amount
        int highest_min_possible_amt = sum_original - remaining_amt;

        // calculate high amount (check for border condition)
        int lowest_max_possible_amt = 0;
        int lowest_deno = denolistwithLimits[denolistwithLimits.size() - 1].first;
        int lowest_deno_limit = denolistwithLimits[denolistwithLimits.size() - 1].second;
        int lowest_deno_utilized = denoMix[lowest_deno];
        int lowest_deno_remaining = lowest_deno_limit - lowest_deno_utilized;



        if(lowest_deno_remaining >= 1)
            lowest_max_possible_amt = highest_min_possible_amt + denolistwithLimits[denolistwithLimits.size() - 1].first;

        std::cout << "*********Given amount is not dispensiable, suggested amount closer to your entered amounts are : \n";

        if (highest_min_possible_amt)
            std::cout << "******Optional closet minimum : EUR " << highest_min_possible_amt << "\n";

        if (lowest_max_possible_amt)
            std::cout << "******Optional closet maximum : EUR " << lowest_max_possible_amt << "\n";

        std::cout << "Try another transaction with above amounts \n";

        // erase deno mix
        denoMix.erase(denoMix.begin(), denoMix.end());

        return 0;
    }

    return 1;

}

// Name : greedyAlgo: Calculate the mix following greedy alogirith. i.e. pick as many as highest deno available till amount is satisfied
// Param :
//  arr : cassette lsit
//  sum : amount for which bill mix to be calculated
// denomix : deno mix map where result to be written
// Return :
//  int : If mix possible return 1 other wise 0

int CashOutModule::greedyAlgo(std::vector<int>& arr, int sum, std::map<int, int, std::greater<int>>& denoMix)
{
    int sum_original = sum;
    std::sort(arr.begin(), arr.end(), std::greater<int>());

    for (int i = 0; i < arr.size(); i++) {

        while (sum >= arr[i]) {
            int cnt = sum / arr[i];
            sum = sum % arr[i];
            denoMix[arr[i]] = cnt;
        }
    }

    if (sum != 0) {

        denoMix.erase(denoMix.begin(), denoMix.end());

        int remaining_amt = sum;

        int highest_min_possible_amt = sum_original - remaining_amt;

        int lowest_max_possible_amt = highest_min_possible_amt + arr[arr.size() - 1];

        std::cout << "Invalid amount!!!!Given amount is not dispensiable, suggested amount closer to your entered amounts are : \n";
        
        if(highest_min_possible_amt)
        std::cout << "Optional closet minimum : EUR " << highest_min_possible_amt << "\n";

        if(lowest_max_possible_amt)
        std::cout << "Optional closet maximum : EUR " << lowest_max_possible_amt << "\n";

        std::cout << "Try another transaction with above amounts \n";

        return 0;
    }

    return 1;

}

// Name : validDenoAvailable: Prepare valid denomination data available for a transaction
// Param :
//  cuInfo : cassette info
//  denoList : list to hold valid cassette denomination
//  denolistwithLimits : max limit of denomination count for a perticular denomination
// Return :
//  void

void CashOutModule::validDenoAvailable(std::vector<int>&denoList, std::vector<std::pair<int, int>>& denolistwithLimits) {

    LPCASHUNIT* csslist = cuInfo->lppList;

    for (int i = 0; i < cuInfo->count; i++) {

        CASHUNIT& cassUnit = *csslist[i];

        if (cassUnit.Type == BILL && cassUnit.Status == ONLINE && cassUnit.count > 0) {

            denoList.push_back(cassUnit.denominations / exp_factor);

            // zero means no restriction
            int perTxnLimit = cassUnit.maximum == 0 ? INT_MAX : cassUnit.maximum;

            denolistwithLimits.push_back({ cassUnit.denominations / exp_factor, std::min(cassUnit.count, perTxnLimit) });

        }
    }

}



// Name : presentCashThread: Thread to simulate cash taken process with a 20 second timeout
// Param :
//  None
// Return :

auto presentCashThread = []() {

    std::cout << "Please take your cash, you have 20 mins...\n";
    std::cout << "Press 1 to simulate note taken\n";

    std::cin >> input;

    if (!isTimeout)
        std::cout << "Note taken event....\n";
};


// Name : takeCash: prsent the cash and wait for customer to take amount for a certain timeout
// Param :
//  None
// Return :
//  bool : Return true if cash is taken. Return false on timeout

bool CashOutModule::takeCash() {

    input.clear();
    isTimeout = false;

    // start thread
    std::thread t1(presentCashThread);

    // Wait for 20 seconds or note taken event
    auto start = std::chrono::steady_clock::now();
    auto now = start;
    while (now < start + std::chrono::seconds{ 20 } && input.empty()) {
        std::this_thread::sleep_for(std::chrono::seconds(2));
        now = std::chrono::steady_clock::now();
    }

    // if timeout happened, input would be empty
    // TO DO : here we need to somehow close the cin stream ( for now I am using dummy quetion for user to force input so that cin will close
    if (input.empty()) {
        std::cout << "!!! TIMEOUT !!! Your cash will be retracted. please press any key followed by enter to go to main menu";
        isTimeout = true;
        t1.join();
    }
    else {
        t1.join();
        std::cout << "Your pressed : " << input << "\n";
    }

    return !isTimeout;


}



// Name : withdrawalFlow: Withdrawal Transaction Flow. It follows below steps
//  1. Get valid denomination
//  2. Take customer input
//  3. Generate Mix for customer input amount using valid denomination calculated in step 1. 
//  4. Display Mix and ask for confirmation
//  5. Present notes. Update casseteInfo object
// Param :
//  cuInfo : Cassette information
// Return :
//  int : Return 1 on successful transaction (i.e. customer taken the cash) otherwise 0 for all cases

int CashOutModule::withdrawalFlow() {

    int rc = 0;
    // 1. Get Valid Denomination

    std::vector<int> denoList;
    std::vector<std::pair<int, int>> denolistwithLimits;

    validDenoAvailable(denoList, denolistwithLimits);

    // 1.1 print valid deno with limits (for tracing purpose)

    std::cout << "********TRACE INFO SECTION START: \n Following are valid denomination considered for this transaction (deno, limit) : " << "\n";

    for (auto denoPair : denolistwithLimits) {
        std::cout << denoPair.first << "--" << denoPair.second << "\n";
    }

    /*
    for (int i = 0; i < denoList.size(); i++) {

        std::cout << "    (" << denoList[i] << " , " << limits[i] << ")" << "\n";
    } 
    */

    std::cout << "********TRACE INFO SECTION END*************" << "\n";

    // 2. Get Customer input

    std::cout << "How much amount your would like to withdraw ? (press enter upon amount entry)" << "\n";
    int dispenseAmount;
    std::cin >> dispenseAmount;


    // 3. Generate Mix using greedy algorithm
    // store denomination mix in a map <key, value>
    // key   : denomination
    // value : mix count

    std::map<int, int, std::greater<int>> denoMix;

    // int result = greedyAlgo(denoList, dispenseAmount, denoMix);

    int result = greedyAlgo_limit(denolistwithLimits, dispenseAmount, denoMix);


    // 3.1 check if amount is possible
    if (!result) {
        return -1;
    }

    // 4. Display Confirmation
    for (auto item : denoMix)
    {
        std::cout << std::setw(10) << "EUR " << std::setw(5) << item.first << std::setw(3) << " X " << std::setw(3) << item.second << std::endl;
    }
    std::cout << "Please confirm your amount, \n Press 1 for YES \n Press 2 for Cancel\n";
    int choice;
    std::cin >> choice;

    // 5. Present notes and wait for taken event

    bool cashTaken = false;

    switch (choice)
    {
    case 1:
        // 5.1 update the cash counter as notes have been dispensed & presented to the customer
        updateCashCounter(denoMix);

        // 5.3 Ask customer to take notes
        cashTaken = takeCash();

        // 5.3. If notes not taken then move all notes to retract counters. 
        if (!cashTaken) {
            updateRetractCounter(denoMix);
            std::cout << "**************Transaction unssucessful, Thank you for using our service!!!**********" << "\n";
        }
        else {
            rc = 1;
            std::cout << "**************Transaction successfull, Thank you for using our service!!!**********" << "\n";
        }

        break;
    case 2:
        std::cout << "**************Transaction cancelled, Thank you for using our service!!!**********" << "\n";
        break;
    default:
        std::cout << "**************Transaction cancelled, Thank you for using our service!!!**********" << "\n";
        break;
    }

    return rc;

}


// Name : getwidrawalStatus: Check if cassette configurations are correct
//      withdrawal will be available if all below conditions met
//      1) validCassCount should be ( >= && <= 5) cassettes should present (with count and status should be valid)
//      2) 1 retract cassette available ( not checking the status)
//      3) 1 reject cassette must be available (not checking the status) 
// Param :
//  cuInfo : cassette information object
// Return :
//  bool : true / false

bool CashOutModule::getwidrawalStatus() {

  //  cdm_cash_unit** cd = CashOutModule::cuInfo->lppList;

    LPCASHUNIT* csslist = CashOutModule::cuInfo->lppList;


    int validCassCount = 0;

    int validRetractCount = 0;
    int validRejectCount = 0;

    for (int i = 0; i < CashOutModule::cuInfo->count; i++) {

        CASHUNIT& cassUnit = *csslist[i];

        if (cassUnit.Type == BILL && cassUnit.count > 0 && cassUnit.Status == ONLINE) {
            validCassCount++;
        } else if (cassUnit.Type == REJECT) {
            validRejectCount++;
        } else if (cassUnit.Type == RETRACT) {
            validRetractCount++;
        }
    }

    if (validCassCount >= 1 && validCassCount <= 5 && validRejectCount == 1 && validRetractCount == 1) {
        dispenserStatus = true;
    }

    return dispenserStatus;

}

// Name : readCuinfoData: read cassette info from file named : cuinfo.txt
//      Data must be stored as comma separated values separated list of type, unitid, denominations, count, maximum, status 
// Param :
//  vec : cassette data will be written into this variable
// Return :
//  void

void CashOutModule::readCuinfoData(std::vector<std::vector<std::string>>& vec) {

    std::ifstream cuinfoData("cuinfo.txt");
    std::string line;
    while (std::getline(cuinfoData, line)) {

        if (line[0] != '#' and line[0] != ';') {
            //std::cout << line << "\n";
            std::istringstream ss(line);
            std::string str;
            std::vector<std::string> cassrow;
            while (std::getline(ss, str, ',')) {
                cassrow.push_back(str);
            }

            vec.push_back(cassrow);

        }
    }

    cuinfoData.close();


}



// Name : prepareCassetteData: create LPCASHUNIT* for list of cassette
// Param :
//  vec : contains cassette data read from cuinfo.txt file
// Return :
//  LPCASHUNIT* pointing to list of cassette

LPCASHUNIT* CashOutModule::prepareCassetteData(std::vector<std::vector<std::string>>& vec) {

    int const size = vec.size();
    LPCASHUNIT* lppList = new CASHUNIT * [size];

    int i = 0;
    for (int i = 0; i < vec.size(); i++) {

        // extract each cassette data
        int type = std::stoi(vec[i][0]);
        const char* unitid = vec[i][1].c_str();
        short denominations = std::stoi(vec[i][2]);
        int count = std::stoi(vec[i][3]);
        int maximum = std::stoi(vec[i][4]);
        short Status = std::stoi(vec[i][5]);

        CASHUNIT* cassUnit = new CASHUNIT{ type, unitid, denominations, count, maximum, Status };

        lppList[i] = cassUnit;
    }

    return lppList;
}

// Name : getCuInf : prepare cdm information object
// Param :
//  void 
// Return :
//  LPCUINFO

LPCUINFO CashOutModule::getCuInfo() {

    // read data from text file

    std::vector<std::vector<std::string>> rawcasseteInfo;

    readCuinfoData(rawcasseteInfo);

    // prepare cassete list

    LPCASHUNIT* caaslist = prepareCassetteData(rawcasseteInfo);

    // Make cuinfo

    LPCUINFO cuInfo = new CUINFO{ (int)rawcasseteInfo.size(), caaslist };

    return cuInfo;
}


void CashOutModule::printCassetteData(const CASHUNIT& cass) {

    std::cout << cass.Type << std::setw(15) << cass.UnitID << std::setw(15) << cass.denominations / exp_factor << std::setw(15) << cass.count << std::setw(15) << cass.maximum << std::setw(15) << cass.Status << std::endl;

}

void CashOutModule::printCuInfo() {

    LPCASHUNIT* csslist = cuInfo->lppList;

    std::cout << "**************************CUINF DATA START****************************\n";

    std::cout << "Type" << std::setw(15) << "UnitID" << std::setw(15) << "Denomination" << std::setw(15) << "Count" << std::setw(15) << "Max" << std::setw(15) << "Status" << std::endl;

    for (int i = 0; i < cuInfo->count; i++) {

        printCassetteData(*csslist[i]);

    }

    std::cout << "**************************CUINF DATA END****************************\n";

}



