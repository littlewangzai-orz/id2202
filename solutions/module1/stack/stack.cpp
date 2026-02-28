#include <iostream>
#include <stack>
#include <string>
#include <sstream>
#include <fstream>
#include <stdexcept>
#include <regex>

// 
class StackMachine {
private:
    std::stack<int> stack;

public:
    void push(int value) {
        stack.push(value);
    }

    void pop() {
        if (stack.empty()) {
            throw std::runtime_error("Stack is empty");
        }
        stack.pop();
    }

    void add() {
        if(stack.size() < 2){
            return;
        }
        else {
            int a = stack.top();
            stack.pop();
            int b = stack.top();
            stack.pop();
            stack.push(a + b);
        }
        
    }

    void sub() {
        if(stack.size() < 2){
            return;
        }
        else {
            int a = stack.top();
            stack.pop();
            int b = stack.top();
            stack.pop();
            stack.push(a - b);
        }
    }

    void mul() {
        if(stack.size() < 2){
            return;
        }
        else {
            int a = stack.top();
            stack.pop();
            int b = stack.top();
            stack.pop();
            stack.push(a * b);
        }
    }

    void div() {
       if(stack.size() < 2){
            return;
        }
        else {
            int a = stack.top();
            stack.pop();
            int b = stack.top();
            stack.pop();
            stack.push( a / b);
        }
    }

    void show() {
        if(stack.empty()){
            std::cout << "stack is empty" << std::endl;
            return;
        }
        else {
            std::cout << stack.top() << std::endl;
        }
    }
    void clean_stack(){
            while(!stack.empty()){
                stack.pop();
            }
    }
};

bool executeInstruction(const std::string& command, StackMachine& machine) {
    // use RE to match push10 and translate it into push 10
    std::regex push_no_space_pattern(R"(push(\d+))");
    std::smatch match;

    std::string modified_command = command;  
    if (std::regex_match(command, match, push_no_space_pattern)) {
        
        modified_command = "push " + match[1].str();
    }

    std::istringstream iss(modified_command);
    std::string instr;
    iss >> instr;

    if (instr == "push") {
        int value;
        if (iss >> value) {  
            std::string extra;
            if (!(iss >> extra)) {  
                machine.push(value);
                return true;
            } else {
                std::cerr << "Error: Extra content after push command" << std::endl;
                return false;
            }
        } else {
            std::cerr << "Error: push command requires a numeric argument" << std::endl;
            return false;
        }
    } else if (instr == "pop" || instr == "add" || instr == "sub" || instr == "mul" || instr == "div" || instr == "show") {
        
        std::string extra;
        if (iss >> extra) {
            std::cerr << "Error: Extra content after command: " << instr << std::endl;
            return false;
        }

        
        if (instr == "pop") machine.pop();
        else if (instr == "add") machine.add();
        else if (instr == "sub") machine.sub();
        else if (instr == "mul") machine.mul();
        else if (instr == "div") machine.div();
        else if (instr == "show") machine.show();

        return true;
    } else {
        std::cerr << "Error: Unknown command: " << command << std::endl;
        return false;
    }
}

int main() {
    StackMachine machine;
    std::string line;

    // RE pattern
    std::regex pattern(R"(\s*(\s*push\s*\d+|\s*pop\s*|\s*add\s*|\s*sub\s*|\s*mul\s*|\s*div\s*|\s*show\s*)\s*;(\s*(\s*push\s*\d+|\s*pop\s*|\s*add\s*|\s*sub\s*|\s*mul\s*|\s*div\s*|\s*show\s*)\s*;)*)");

    while (std::getline(std::cin, line)) {
        machine.clean_stack();  //make stack clean
        //remove the blankspace 
        line.erase(remove_if(line.begin(), line.end(), isspace), line.end());
        if (line.empty()) {
            break; //break when it meet empty line
        }
        // use RE to make sure that all the patterns are matched
        if (!std::regex_match(line, pattern)) {
            std::cout << "Error: Each command must end with a semicolon and be correctly formatted" << std::endl;
            return 1;
        }

        std::stringstream ss(line);
        std::string command;

        // use ; to divide
        while (std::getline(ss, command, ';')) {
            // delete first and last blanksapce
            command.erase(0, command.find_first_not_of(" \t"));
            command.erase(command.find_last_not_of(" \t") + 1);

            // jump empty command 
            if (command.empty()) continue;

            // execute and check its validty
            if (!executeInstruction(command, machine)) {
                return 1;  
            }
        }
    }
    return 0;
}