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
public:
    bool empty() const
    {
        the_mutex.try_lock();
        return the_queue.empty();
    }
    void enqueue(Data &data)
    {
        { // acquire lock
            std::unique_lock<std::mutex> lock(the_mutex);
            the_queue.push(data);
        } // release lock
        
        // wake up one thread
        the_condition_variable.notify_one();
    }
    void wait_and_pop(Data& popped_value)
    {
        {
            std::unique_lock<std::mutex>
            lock(this->the_mutex);
            while(this->the_queue.empty())
            { // if there are none wait for notification
                this->the_condition_variable.wait(lock);
            }
            popped_value = this->the_queue.front();
            this->the_queue.pop();
        }   // release lock
        return;
    }
    
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
        leftParen = characters[0];
        if (leftParen != '(') {
            throw ParseException("Parse Exception expecting (");
        }
        char oprChar = ' ';
        oprChar = characters[1];
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
        char space = 'a';
        space = characters[2];
        if(space != ' ' ) {
            std::string throwstr("Parse Exception expected space after operator, found:");
            throwstr += space;
            throw ParseException(throwstr);
        }
             int curPos = 3;
        char lhsFirst = (char)characters[3];
        if (lhsFirst == '(') {
            lhs = new ExprWrapper();
     
             curPos += lhs->Parse(line.substr(3));
        }
        else {
            lhs = new ExprWrapper();
            std::string valStr;
            while (characters[curPos] != ' ' && characters[curPos] != ')') {
                valStr += characters[curPos];
                curPos++;
            }
            float val = std::stof(valStr);
           lhs->Parse(val);
        }
        char spacerhs = 'a';
        
        spacerhs =characters[curPos];
        if(spacerhs != ' ' ) {
            std::string throwstr("Parse Exception expected space after operator, found:");
            throwstr += spacerhs;
            throw ParseException(throwstr);
        }
        curPos++;
        char rhsFirst = (char)characters[curPos];
        if (rhsFirst == '(') {
          
            rhs = new ExprWrapper();
            curPos += rhs->Parse(line.substr(curPos));
            
        }
        else {
            rhs = new ExprWrapper();
            std::string valStr;
            while (characters[curPos] != ' ' && characters[curPos] != ')') {
                valStr += characters[curPos];
                curPos++;
            }
            float val = std::stof(valStr);
            rhs->Parse(val);
        }
        char endExpr = (char)characters[curPos];
        if (endExpr != ')') {
            std::string throwstr("Parse Exception expected ) at end of expr, found:");
            throwstr += endExpr;
            throw ParseException(throwstr);
        }
        curPos++;
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
void runFunction()
{
    while(!endProgram || !workQueue.empty()) {
        std::shared_ptr<ExprWrapper> popped_value;
        workQueue.wait_and_pop(popped_value);
        try {
        if(popped_value->Parse() > 0) {
            float val = popped_value->GetValue();
            std::string str;
            str = "Expression: ";
            str += popped_value->GetLine();
            str += " Value: ";
            str += std::to_string(val);
            
            str += " \n";
            printQueue.enqueue(str);
   
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
} catch (...) {
    // ...
    std::string exStr = "General Parse Exception";
    printQueue.enqueue(exStr);
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
        
        
    
    while (std::getline (std::cin,*lineInput)) {
        std::shared_ptr<ExprWrapper> expr(new ExprWrapper());
        expr->SetLine(*lineInput);
        workQueue.enqueue(expr);
        lineInput = new std::string();
    }
    endProgram = true;
    for(int i=0; i < workers.size(); i++){
        if(workers[i].joinable()){
            workers[i].join();
        }
    }
    if(printThread.joinable()) {
        printThread.join();
    }

}


