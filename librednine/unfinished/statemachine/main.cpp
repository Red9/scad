#include <stdio.h>



class StateMachine {
    typedef void (StateMachine::*stateFunctionPointer)();
    stateFunctionPointer nextState;
public:

    StateMachine() {
        nextState = &StateMachine::function1;
    }

    void Run(void) {
        (this->*nextState)();
    }


private:

    void functionWait(void) {
    }

    void function1(void) {
        printf("\r\nFunction1");
        nextState = &StateMachine::function2;
    }
    void function2(void) {
        printf("\r\nFunction2");
        nextState = &StateMachine::function3;
    }
    void function3(void) {
        printf("\r\nFunction3");
        nextState = &StateMachine::functionWait;
    }

};

int main(int argc, char** argv) {

    printf("\r\nHello, World!");

    StateMachine s;
    while(true){
        s.Run();
    }
    
    

    return 0;
}