// src/tools/calculator_eval.cpp — YOU IMPLEMENT (spec §4.6).
//
// calc_evaluate: evaluate a well-formed token list with two stacks
// (one for numbers, one for operators and '('). See spec §4.6 and the
// worked trace in Appendix C. std::stack or std::vector are both fine.

#include "tools/calculator.h"
#include <vector>

using namespace std;

static int precedence(CalcToken::Kind op) {
    if(op == CalcToken::Kind::Times || op == CalcToken::Kind::Divide){
        return 2;
    }
    return 1;
}

bool calc_evaluate(const std::vector<CalcToken>& tokens, double* out,
                   std::string* error) {
    vector<double> num_stack;
    vector<CalcToken::Kind> op_stack;

    auto apply_operator = [&]() -> bool {
        CalcToken::Kind op = op_stack.back();
        op_stack.pop_back();

        double right = num_stack.back();
        num_stack.pop_back();

        double left = num_stack.back();
        num_stack.pop_back();

        double result = 0.0;

        if(op == CalcToken::Kind::Plus) {
            result = left + right;
        } 
        else if(op == CalcToken::Kind::Minus) {
            result = left - right;
        }
         else if(op == CalcToken::Kind::Times) {
            result = left * right;
        } 
        else if(op == CalcToken::Kind::Divide) {
            if(right == 0.0) {
                *error = "division by zero";
                return false;
            }
            result = left / right;
        }
        num_stack.push_back(result);
        return true;
    };

    for(const CalcToken& token : tokens){
        if(token.kind == CalcToken::Kind::Number){
            num_stack.push_back(token.value);
        }
        else if(token.kind == CalcToken::Kind::LParen){
            op_stack.push_back(token.kind);
        }
        else if(token.kind == CalcToken::Kind::RParen){
            while(!op_stack.empty() && op_stack.back() != CalcToken::Kind::LParen){
                if(!apply_operator()){
                    return false;
                }
            }
            
            if(!op_stack.empty()){
                op_stack.pop_back();
            }
        }
        else{
            while(!op_stack.empty() && op_stack.back() != CalcToken::Kind::LParen &&
                  precedence(op_stack.back()) >= precedence(token.kind)){
                if(!apply_operator()){
                    return false;
                }
            }
            op_stack.push_back(token.kind);
        }
    }

    while(!op_stack.empty()){
        if(!apply_operator()){
            return false;
        }
    }

    *out = num_stack.back();
    *error = "";
    return true;
}
