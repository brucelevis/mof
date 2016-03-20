//
//  Application.h
//  GameSrv
//
//  Created by prcv on 13-6-9.
//
//

#ifndef shared_Application_h
#define shared_Application_h

class Application
{
public:
	Application(){}
    
	~Application(){}
    
	bool isDaemon() const;
    /// Returns true if the application runs from the command line.
    /// Returns false if the application runs as a Unix daemon
    /// or Windows service.
    
	int run(int argc, char** argv);
    /// Runs the application by performing additional initializations
    /// and calling the main() method.
    
	virtual int init(int argc, char** argv){return 0;}
	virtual int main(int argc, char** argv){return 0;}
    virtual void deinit() {}
    
	static void terminate();
    /// Sends a friendly termination request to the application.
    /// If the application's main thread is waiting in
    /// waitForTerminationRequest(), this method will return
    /// and the application can shut down.
    
protected:
	void waitForTerminationRequest();
    
	bool checkDaemon(int argc, char** argv);
	void beDaemon();
    
    static bool sTermiated;
};


#endif
