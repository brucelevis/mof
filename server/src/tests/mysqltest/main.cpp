#include <cstdio>
#include <iostream>
#include <string>
#include <cmath>
#include <map>

using namespace std;
#define LOG cout

#include "base/callback.h"
#include "thread/thread.h"
#include "mq/MQ.h"
#include "base/os.h"
#include "db/db_mysql.h"

const char* host = "tcp://192.168.160.37:3306";
const char* username = "root";
const char* password = "123456";


class QueryDatabases : public DBOperation
{
public:
    bool exec(Statement* statement){
        
        
        MyDBResult result(statement->executeQuery("show tables"));
        while (result.mResult->next()){
            string dbname = result.mResult->getString(0);
            cout << dbname << endl;
        }
        
        return true;
    }
};


int main(int argc, char** argv)
{
    AsyncDBProcessor processor;
    processor.init(4, "tcp://192.168.160.37:3306,root,123456,Nx_Log");
    processor.run();
    
    for (int i = 0; i < 10; i++){
        QueryDatabases* query = new QueryDatabases;
        processor.call(query);
    }
    
    os::sleep(10000);
    processor.stop();
    processor.deinit();
    
    return 0;
}
