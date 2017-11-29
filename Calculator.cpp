#define _CRT_SECURE_NO_WARNINGS // for MS crt.
#include <fstream>
#include <stack>
#include <string>
#include <iomanip>
#include <cmath>
using namespace std;
bool isLetter(char c);
bool isOperator(char c);
bool isBracket(char c);
int getToken(const string& expr, string& token, size_t& idx);
int getOperatorPriority(const string& op);
int infix_to_postfix(const string& infix, string& postfix);
int compute_postfix(const string& postfix_expr, double& result);
int main()
{
    ifstream cin("input.txt");          // input
    ofstream cout("output.txt");        // output
    string infix_expr, postfix_expr;
    double result = 0.0;
    
    while (getline(cin, infix_expr)) {
        if (infix_to_postfix(infix_expr, postfix_expr)) {
            if (compute_postfix(postfix_expr, result)) {
                cout << fixed << setprecision(2) << result << endl;
                continue ; // have no error, so next expr.
            }
        }
    
        cout << "ERROR IN INFIX NOTATION" << endl;
    }
    return 0;
}
int infix_to_postfix(const string& infix_expr, string& postfix_expr)
{
    string token;
    size_t idx = 0;
    stack<string> stk;
    int balance = 0; // use to check the bracket's balance.
    postfix_expr.clear();
    while (getToken(infix_expr, token, idx)) {
        switch (token[0]) {
            /* If we see '+','-','*','/','%','^','(' , then we pop entries from
             * the stack until we find an entry of lower priority
             * One exception is that we never remove a '(' from the stack except when processing a ')'.*/
            case '+':
            case '-':
            case '*':
            case '/':
            case '%':
            case '(':
                if (token[0] == '(')
                    ++balance;
                while (!stk.empty() && getOperatorPriority(stk.top()) >= getOperatorPriority(token) && stk.top() != "(") {
                    postfix_expr += stk.top();
                    stk.pop();
                    postfix_expr += " ";
                }
                stk.push(token);
                break;
            /* right association, handle it specifically! */
            case '^':
                while (!stk.empty() && getOperatorPriority(stk.top()) > getOperatorPriority(token) && stk.top() != "(") {
                    postfix_expr += stk.top();
                    stk.pop();
                    postfix_expr += " ";
                }
                stk.push(token); // later come, early out.(right association)
                break;
            /* If we see a ')', then we pop the stack, writing symbols until we encounter
             * a (corresponding) left parenthesis, which is poped but not output. */
            case ')':
                --balance;
                while (!stk.empty() && stk.top() != "(") {
                    postfix_expr += stk.top();
                    stk.pop();
                    postfix_expr += " ";
                }
                stk.pop();
                break;
            default:
                postfix_expr += token;
                postfix_expr += " ";
                break;
        }
    }
    while (!stk.empty()) {
        postfix_expr += " ";
        postfix_expr += stk.top();
        stk.pop();
    }
    if (balance != 0) {
        return 0;
    }
    return 1;
}
int compute_postfix(const string& postfix_expr, double& result)
{
    stack<string> stk;
    string token;
    size_t idx = 0;
    double operand1 = 0.0, operand2 = 0.0;
    char buf[128] = {0};
    int resStatus = 0;
    while (getToken(postfix_expr, token, idx)) {
        memset(buf, 0, 128 * sizeof(char));
        switch (token[0]) {
            case '+':
            case '-':
            case '*':
            case '/':
            case '%':
            case '^':
                // firstly, get two operands to compute.
                operand1 = atof(stk.top().c_str());
                stk.pop();
                operand2 = atof(stk.top().c_str());
                stk.pop();
                switch (token[0]) {
                    case '+':
                        sprintf(buf, "%f", operand1 + operand2);
                        break;
                    case '-':
                        sprintf(buf, "%f", operand2 - operand1);
                        break;
                    case '*':
                        sprintf(buf, "%f", operand1 * operand2);
                        break;
                    case '/':
                        if (!operand1) {
                            resStatus = 0;
                            goto Exit;
                        }
                        sprintf(buf, "%f", operand2 / operand1);
                        break;
                    case '%':
                        // if operand is a float number, then error!
                        if ((int)operand1 != operand1 || (int)operand2 != operand2 || !operand1) {
                            resStatus = 0;
                            goto Exit;
                        }
                        // care: the format should be "%d".
                        sprintf(buf, "%d", (int)operand2 % (int)operand1);
                        break;
                    case '^':
                        if (operand2 <= 0) {
                            resStatus = 0;
                            goto Exit;                      
                        }
                        sprintf(buf, "%f", pow(operand2, operand1));
                        break;
                }
                stk.push(string(buf)); 
                break;
            default:
                stk.push(token); // numbers push into the stack directly.
                break;
        }
    }
    // now the number in the stack is the result.
    result = atof(stk.top().c_str());
    resStatus = 1;
Exit:
    return resStatus;
}
int getOperatorPriority(const string& op)
{
    int priority = -1;
    switch (op[0])
    {
        case '(':
            priority = 4;
            break;
        case '+':
        case '-':
            priority = 1;
            break;
        case '*':
        case '/':
        case '%':
            priority = 2;
            break;
        case '^':
            priority = 3;
            break;
    }
    return priority;
}
int getToken(const string& expr, string& token, size_t& idx)
{
    char curChar = 0;
    bool inTok = false ;
    int res = 0;
    token.clear();
    size_t len = expr.length();
    while (idx < len) {
        curChar = expr[idx++];
        if (isspace(curChar) || isOperator(curChar) || isBracket(curChar)) {
            if (!inTok && (isOperator(curChar) || isBracket(curChar))) {
                token += curChar;
                res = 1; // token is a operatro or bracket, return token immediately.
                break;
            }
            if (inTok) {
                if (isOperator(curChar) || isBracket(curChar))
                    --idx; // back input pointer and get the operator or bracket when we call getToken() next time.
                res = 1;
                break;
            }
            inTok = false;
        }  else if (isLetter(curChar)) { // letter is illegal.
            res = 0;
            break;
        }  else if (!inTok) { // begin a number.
            inTok = true;
            token += curChar;
        } else if (inTok) {
            token += curChar;
        }
    }
    if (!token.empty())
        res = 1;
    return res;
}
bool isLetter(char c)
{
    if (c >= 'a' && c <= 'z' || c >= 'A' && c <= 'Z')
        return true;
    return false;
}
bool isOperator(char c)
{
    if (c == '+' || c == '-' || c == '*' || c == '/' || c == '%' || c == '^')
        return true ;
    return false ;
}
bool isBracket(char c)
{
    if (c == '(' || c == ')')
        return true;
    return false;
}
