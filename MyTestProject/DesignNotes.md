Design notes :
  1. Application.cpp : is main control instance. It creates CashCoutModule module to inilialize (read cuinfo.text) and prepare cash unit data.
  
  2. CashUnitStructure.h : This defines the structs for cdm_cash_unit and cdm_cu_info ( Added few special member functions)
  
  3. CashOutModule.h and CashOutModule.cpp : This is the class where actual flow control is handled (Important method is 'withdrawalFlow' which handles the transaction flow).
     Below is overall process
  
    -> Read and prepare cdm_cu_info (cassette unit configuration)
    -> Prepare valid denominations along with each deno limit
        - For limit calculation it used fields 'count' and 'maximum' (maximum is limit per dispense operation)
        - limit for a denomination is min of (count, maximum)
        - if maximum is set to 0 then there is no limit placed per transaction. it will take count as max limit.
    -> Take customer input
    -> Generate Mix for customer input amount
        - Two functions are available 
          - 'greedyAlgo' without limits
          - 'greedyAlgo_limit' with limits
          - Currently greedyAlgo_limit is enabled
          - If mix is not possible it recommends two amounts for customer to try
    -> Display Mix and ask for confirmation of user
        - If user cancels (nothing happens goes back to menu)
        - If user press yes, goes to next step to present notes
    -> Present notes. Update casseteInfo object
        - As soon as notes are presented, it will update cassette unit structure
        - Wait for customer (20 seconds). ( Faced a challange with implementation std::cin as it is a blocking call and there is no straight forward approach to cancel 
          std::cin read mode. I used a thread based approach and a hack to handle this scenario)
        - On Timeout : Update retract counters and show message
        - Upon note taken event : Do nothing ( cassette structure is already updated)
        
  NOTE : I am yet to implement persist logic (i.e. writing to cuinfo.txt file the latest counter). However, once application launched, 
          we can do multiple transactions in chaining and it will update and show the cassette couunters on screen.
          
IMP --------->:Testing recommendation <-------------IMP
    1. Select Show Cassette Details (password protected '1234')
    2. Onserve cassette status 
    1. Select withdrawal transaction
    2. It wil show notes which are being considered (along with limit)
    3. Enter amount
    4. Confirm the MIX
    5. Take your cash or let timeout happens ( based on test case)
    6. It will show the main menu
    7. Select Show Cassette Details
    8. Observe the cassette details modified
    9. Follow steps 1 - 8 in chaining and observe the expected status after each txn
        
Improvemnets :
  - More of modern c++ constructs should have been used.
  - Some of edge cases the flow
  - Proper message wording
  - Not updating the text file to persit. It maintains state till application is running. 
        
   
