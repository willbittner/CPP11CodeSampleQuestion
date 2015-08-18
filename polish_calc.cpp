//
//  main.cpp
//  datto-test-cpp
//
//  Created by William Bittner on 8/17/15.
//  Copyright (c) 2015 William Bittner. All rights reserved.
//

#include <iostream>
#include <thread>
#include <string>
#include <sstream>
#include <chrono>
#include <vector>
#include <queue>
#include <exception>
class ParseException : std::exception {
private:
    
    std::string exceptionString;
    
public:
    virtual const char* what() const throw()
    {
        return exceptionString.c_str();
    }
    ParseException(std::string exstr = "General Parse Exception") : exceptionString(exstr) {};
};
enum OprType {
    Addition=0,
    Subtraction=1,
    Division=2,
    Multiplication=3
};
enum ValueOrExpr {
    Value=0,
    Expression=1
};
template<typename Data>
class concurrentQueue
{
private:
    std::queue<Data> the_queue;
    mutable std::mutex the_mutex;
    std::condition_variable the_condition_variable;
    bool quit = false;
public:

    bool empty() const
    {
        if(the_mutex.try_lock()) {
        	bool val = the_queue.empty();
        	the_mutex.unlock();
        	return val;
        }
        else {
        	return false;
        }
    }
    void dumpQueue() {
      // quit = true;
        the_condition_variable.notify_all();
        
    }
    void enqueue(Data &data)
    {
        { // acquire lock
            std::unique_lock<std::mutex> lock(the_mutex);
            the_queue.push(data);
            the_condition_variable.notify_one();
        } // release lock
        
        // wake up one thread
        
    }
    bool unsafeEmpty() {
    	return the_queue.empty();
    }
    bool try_pop(Data& popped_value) {
    	
    	
    	 if(this->the_mutex.try_lock()) {
    	
           if(this->the_queue.empty()) {
            this->the_mutex.unlock();
            return false;
        	}
                  popped_value = this->the_queue.front();
            this->the_queue.pop();
            this->the_mutex.unlock();
            return true;
        }
            
       
        else
        {
        	return false; //unable to grab lock
        }
    }
    void wait_and_pop(Data& popped_value)
    {
        {
            std::unique_lock<std::mutex>
            lock(this->the_mutex);
            while(this->the_queue.empty() && !quit)
            { 
                this->the_condition_variable.wait(lock);  
            }
            if (quit == true && this->the_queue.empty()) {
        
                return;
            }
            
            popped_value = this->the_queue.front();
            this->the_queue.pop();   
            
        }   
        return;
    };
    
};

concurrentQueue<std::string> printQueue;
class ExprWrapper
{
private:
    ExprWrapper* lhs;
    ExprWrapper* rhs;
    OprType operatorType;
    ValueOrExpr exprType;
    float value;
    std::string lineval;
    int curPos = 0;
    int findEndExpr(std::string str, int curPos) {
        char endExpr = str[curPos];
        while (endExpr != ')') {
            
            if (endExpr != ' ') {
                std::string throwstr("Parse Exception expected space until ) at end of expr, found other");
                throwstr += endExpr;
                throw ParseException(throwstr);
            }
            if(curPos >= str.length()) {
                std::string throwstr("Parse exception, expecting terminating ) but never found");
                throwstr += endExpr;
                throw ParseException(throwstr);
            }
            curPos++;
            endExpr = str[curPos];

        }
        if (endExpr != ')') {
            std::string throwstr("Parse Exception expected ) at end of expr, found:");
            throwstr += endExpr;
            throw ParseException(throwstr);
        }
        return curPos;
    }
    int parseFloat(std::string lv, int& cp) {
        std::string valStr;
      
       
        while (lv[cp] != ' ' && lv[cp] != ')') {
            valStr += lv[cp];
            cp++;
        }
        
        float val = 0.0;
        try {
         val = std::stof(valStr);
        }
        catch (...) {
            std::string exstr = "Parse Exception - failed to parse float from string - value: ";
            exstr += valStr;
            exstr += " Current Pos: ";
            exstr += cp;
            throw ParseException(exstr);
        }
            Parse(val);
        return cp; // add support for fail on parse float later
    }
    
    char getNext() {
        if(curPos >= lineval.length()) return -1;
        char curChar = lineval[curPos];
        curPos++;
        return curChar;
    }
    char peekNext() {
        return lineval[curPos];
    }
    std::string getSubExpr() {
        curPos--;
        if (peekNext() != '(') {
            std::string exc = "In getting subExprStirng - failed due to currentChar in peek is not ( it is a " ;
            exc += peekNext();
            throw ParseException(exc);
        }
        int opCount = -1;
        int startPos = curPos;
        char nextChar = peekNext();
        std::string subexpr;
    
        while ( (nextChar = getNext()) >= 0 )
        {
            subexpr += nextChar;

            if (nextChar == '(') opCount ++;
            if (nextChar == ')') {
                if (opCount == 0) {
                    return subexpr;
                }
                opCount--;
            }
        }
        std::string exc = "In getting subExrString - iterated over whole string: ";
        exc += lineval;
        exc += " Starting pos: ";
        exc += std::to_string(startPos);
        exc += " Opcount: " ;
        exc += std::to_string(opCount);
        throw ParseException(exc);
        
        
        
    }
public:
    void SetLine(std::string linestr) {
        lineval = linestr;
    }
    std::string GetLine() {
        return lineval;
    }
    int Parse() {
        return Parse(lineval);
    }
    int Parse(std::string line) {
        lineval = line;
        exprType = ValueOrExpr::Expression;
        //Parse is called after a ( is found,
        // expect input: ( operator lhs rhs )
        
        // get the (
        if(line.length() == 0) return 0;
        std::vector<char> characters;
        char readVal = ' ';
        
        for(int i=0; i < line.length(); i++)
        {
            readVal = line[i];
            characters.push_back(line[i]);
            
        }
        
        //     std::cout << "Parsing Expression: " << line << std::endl;
        char leftParen = ' ';
        leftParen = getNext();
        if (leftParen != '(') {
            throw ParseException("Parse Exception expecting (");
        }
        char oprChar = ' ';
        oprChar = getNext();
        switch(oprChar) {
            case '+':
                operatorType = OprType::Addition;
                break;
            case '-':
                operatorType = OprType::Subtraction;
                break;
            case '/':
                operatorType = OprType::Division;
                break;
            case '*':
                operatorType = OprType::Multiplication;
                break;
            default:
                std::cerr << "Invalid Operator: " << oprChar << std::endl;
                throw ParseException("Parse exception - Ivalid Operator");
                
        }
        char space = getNext();
        if(space != ' ' ) {
            std::string throwstr("Parse Exception expected space after operator, found:");
            throwstr += space;
            throw ParseException(throwstr);
        }
        char lhsFirst = getNext();
        if (lhsFirst == '(') {
            lhs = new ExprWrapper();
            
            lhs->Parse(getSubExpr());
    
        }
        else {
            curPos--;
            lhs = new ExprWrapper();
            curPos = lhs->parseFloat(line,curPos);
        }
        char spacerhs = getNext();
        if(spacerhs != ' ' ) {
            std::string throwstr("Parse Exception expected space after operator, found:");
            throwstr += spacerhs;
            throw ParseException(throwstr);
        }
       
        char rhsFirst = getNext();
        if (rhsFirst == '(') {
            
            rhs = new ExprWrapper();
            rhs->Parse(getSubExpr());
            
        }
        else {
            curPos--;
            rhs = new ExprWrapper();
            curPos = rhs->parseFloat(line,curPos);
        }
        curPos = findEndExpr(line,curPos);
        return curPos;
        
    }
    void Parse(float val) {
        exprType = ValueOrExpr::Value;
        value = val;
    }
    float GetValue() {
        if (exprType == ValueOrExpr::Value) {
            return value;
        }
        else {
            float lhsval = lhs->GetValue();
            float rhsval = rhs->GetValue();
            if (rhsval == 0.0 && operatorType == OprType::Division) {
                std::string str = "Division by zero in expression \n";
                str += lineval;
                
                printQueue.enqueue(str);
                
                return 0.0;
            }
            switch(operatorType) {
                case OprType::Addition:
                    return (lhsval + rhsval);
                case OprType::Subtraction:
                    return (lhsval - rhsval);
                case OprType::Multiplication:
                    return (lhsval * rhsval);
                case OprType::Division:
                    return (lhsval / rhsval);
                default:
                    return value;
                    
            };
            
            
        }
    };
    
};
concurrentQueue<std::shared_ptr<ExprWrapper>> workQueue;
bool endProgram = false;
void workFunc(std::shared_ptr<ExprWrapper>& popped_value) {
    try {
        int pval = 0;
        if (popped_value->GetLine().length() >= 3) {
        if((pval = popped_value->Parse()) > 0) {
            float val = popped_value->GetValue();
            std::string str;
            str = "Expression: ";
            str += popped_value->GetLine();
            str += " Value: ";
            str += std::to_string(val);
            
            str += " \n";
            printQueue.enqueue(str);
            
        }
        else {
            
        }
        }
        else {
        	// skipping empty lines
        }
        
    } catch (const std::exception& ex) {
        std::string str = "";
        str += ex.what();
        printQueue.enqueue(str);
    } catch (const std::string& ex) {
        std::string str = "";
        str += ex;
        printQueue.enqueue(str);
        // ...
    } catch(const ParseException& ex){
        std::string str = "";
        str += ex.what();
        printQueue.enqueue(str);
        
    }catch (...) {
        // ...
        std::string exStr = "General Parse Exception";
        printQueue.enqueue(exStr);
    }
    
}
void runFunction()
{
	while(!endProgram )
	{
		std::shared_ptr<ExprWrapper> popped_value;
	    workQueue.wait_and_pop(popped_value);
	    if(popped_value != NULL) workFunc(popped_value);
	    while(!workQueue.empty()) {
	        

	        if(workQueue.try_pop(popped_value)) {
	        workFunc(popped_value);
	        }
	       // workQueue.doWork(workFunc);
	    }
	}
}

void printFunction()
{
    while(!endProgram  || !printQueue.empty()) {
        std::string popped_value;
        printQueue.wait_and_pop(popped_value);
        std::cout << popped_value;
        std::flush(std::cout);
        
    }
}
std::vector< std::thread > workers;

int main()
{
    
    int numThreads = std::thread::hardware_concurrency() - 1;
    std::cout << "Hardware Concurrency: " << numThreads << "\n";
    std::string* lineInput = new std::string();
    std::thread printThread(printFunction);
    while(--numThreads - 2) {
        workers.push_back(std::thread(runFunction));
    }
    
    
    std::this_thread::sleep_for(std::chrono::milliseconds(1000));
    while (std::getline(std::cin,*lineInput)) {
    	
        if (*lineInput == "end") break;
        std::shared_ptr<ExprWrapper> expr(new ExprWrapper());
    
        expr->SetLine(*lineInput);
        workQueue.enqueue(expr);
        lineInput = new std::string();
        
    }
    endProgram = true;

    
    if(workQueue.empty()) {
    	//std::cout << "Exiting - Queue is Empty \n";
    }
    else
    {
    	//std::cout << "Exiting - Queue is NOT Empty \n";
    	while(!workQueue.empty()) {
    		std::shared_ptr<ExprWrapper> data;
    		if(workQueue.try_pop(data)){
    		 workFunc(data);
    		}
    	}
    	std::cout << "Exiting - Queue has been emptied \n";
    }
    for(int i=0; i < workers.size(); i++){
    	workQueue.dumpQueue();
        if(workers[i].joinable()){
            workers[i].join();
        }
    }
    if(printThread.joinable()) {
        printThread.join();
    }

}


