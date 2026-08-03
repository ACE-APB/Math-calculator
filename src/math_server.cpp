/**
 * @file math_server.cpp
 * @brief C++ HTTP 服务器，提供数学计算API
 */

#include <iostream>
#include <string>
#include <sstream>
#include <vector>
#include <stack>
#include <cctype>
#include <cmath>
#include <algorithm>
#include <stdexcept>
#include <limits>
#include "httplib.h"

using namespace std;

// ==================== 数学函数 ====================

int gcd(int a, int b) {
    if (a < 0) a = -a;
    if (b < 0) b = -b;
    if (a < b) swap(a, b);
    while (b != 0) {
        int remainder = a % b;
        a = b;
        b = remainder;
    }
    return a;
}

int lcm(int a, int b) {
    if (a < 0) a = -a;
    if (b < 0) b = -b;
    if (a == 0 || b == 0) return 0;
    return (a / gcd(a, b)) * b;
}

int gcdMultiple(const vector<int>& numbers) {
    if (numbers.empty()) return 0;
    int result = numbers[0];
    for (size_t i = 1; i < numbers.size(); ++i) {
        result = gcd(result, numbers[i]);
        if (result == 1) break;
    }
    return result;
}

int lcmMultiple(const vector<int>& numbers) {
    if (numbers.empty()) return 0;
    long long result = numbers[0];
    for (size_t i = 1; i < numbers.size(); ++i) {
        result = (result / gcd(static_cast<int>(result), numbers[i])) * numbers[i];
        if (result > numeric_limits<int>::max()) {
            throw overflow_error("结果超出整数范围！");
        }
    }
    return static_cast<int>(result);
}

// ==================== 表达式求值 ====================

int getPriority(char op) {
    switch (op) {
        case '(': return 0;
        case '+': 
        case '-': return 1;
        case '*': 
        case '/': return 2;
        default: return -1;
    }
}

double calculate(double a, double b, char op) {
    switch (op) {
        case '+': return a + b;
        case '-': return a - b;
        case '*': return a * b;
        case '/':
            if (abs(b) < 1e-12) throw runtime_error("除数不能为零！");
            return a / b;
        default:
            throw runtime_error(string("未知运算符: ") + op);
    }
}

bool isNegativeSign(const string& expr, int idx) {
    if (idx < 0 || idx >= static_cast<int>(expr.length())) return false;
    char ch = expr[idx];
    if (ch != '-') return false;
    if (idx == 0) return true;
    char prev = expr[idx - 1];
    return prev == '(' || prev == '+' || prev == '-' || prev == '*' || prev == '/';
}

double evalExpression(const string& expr) {
    if (expr.empty()) throw runtime_error("表达式为空！");
    
    stack<double> numStack;
    stack<char> opStack;
    string numBuffer;
    int len = static_cast<int>(expr.length());

    for (int i = 0; i < len; ++i) {
        char ch = expr[i];
        if (isspace(ch)) continue;

        if (ch == '-' && isNegativeSign(expr, i)) {
            numBuffer += '-';
            continue;
        }

        if (isdigit(ch) || ch == '.') {
            numBuffer += ch;
            continue;
        }

        if (!numBuffer.empty()) {
            numStack.push(stod(numBuffer));
            numBuffer.clear();
        }

        if (ch == '(') {
            opStack.push(ch);
        } else if (ch == ')') {
            while (!opStack.empty() && opStack.top() != '(') {
                char op = opStack.top();
                opStack.pop();
                double b = numStack.top(); numStack.pop();
                double a = numStack.top(); numStack.pop();
                numStack.push(calculate(a, b, op));
            }
            if (opStack.empty()) throw runtime_error("括号不匹配！");
            opStack.pop();
        } else if (ch == '+' || ch == '-' || ch == '*' || ch == '/') {
            while (!opStack.empty() && getPriority(opStack.top()) >= getPriority(ch)) {
                char op = opStack.top();
                opStack.pop();
                double b = numStack.top(); numStack.pop();
                double a = numStack.top(); numStack.pop();
                numStack.push(calculate(a, b, op));
            }
            opStack.push(ch);
        } else {
            throw runtime_error(string("非法字符: ") + ch);
        }
    }

    if (!numBuffer.empty()) {
        numStack.push(stod(numBuffer));
    }

    while (!opStack.empty()) {
        char op = opStack.top();
        opStack.pop();
        double b = numStack.top(); numStack.pop();
        double a = numStack.top(); numStack.pop();
        numStack.push(calculate(a, b, op));
    }

    if (numStack.size() != 1) throw runtime_error("表达式不完整！");
    return numStack.top();
}

// ==================== JSON处理（简化版） ====================

string createJsonResponse(bool success, double result, const string& error = "") {
    stringstream ss;
    ss << "{";
    ss << "\"success\":" << (success ? "true" : "false") << ",";
    if (success) {
        ss << "\"result\":" << result;
    } else {
        ss << "\"error\":\"" << error << "\"";
    }
    ss << "}";
    return ss.str();
}

vector<int> parseNumbers(const string& json) {
    vector<int> numbers;
    size_t pos = json.find("[");
    if (pos == string::npos) return numbers;
    
    size_t end = json.find("]", pos);
    if (end == string::npos) return numbers;
    
    string numsStr = json.substr(pos + 1, end - pos - 1);
    stringstream ss(numsStr);
    string token;
    while (getline(ss, token, ',')) {
        token.erase(0, token.find_first_not_of(" \t\n\r"));
        token.erase(token.find_last_not_of(" \t\n\r") + 1);
        if (!token.empty()) {
            numbers.push_back(stoi(token));
        }
    }
    return numbers;
}

string parseExpression(const string& json) {
    size_t pos = json.find("\"expression\":\"");
    if (pos == string::npos) return "";
    pos += 14;
    size_t end = json.find("\"", pos);
    if (end == string::npos) return "";
    return json.substr(pos, end - pos);
}

// ==================== 主函数 ====================

int main() {
    httplib::Server svr;

    // CORS支持（允许跨域请求）
    svr.set_post_routing_handler([](const auto& req, auto& res) {
        res.set_header("Access-Control-Allow-Origin", "*");
        res.set_header("Access-Control-Allow-Methods", "POST, OPTIONS");
        res.set_header("Access-Control-Allow-Headers", "Content-Type");
    });

    // GCD API
    svr.Post("/gcd", [](const httplib::Request& req, httplib::Response& res) {
        try {
            vector<int> numbers = parseNumbers(req.body);
            if (numbers.empty()) {
                res.set_content(createJsonResponse(false, 0, "未输入数字"), "application/json");
                return;
            }
            int result = gcdMultiple(numbers);
            res.set_content(createJsonResponse(true, result), "application/json");
        } catch (const exception& e) {
            res.set_content(createJsonResponse(false, 0, e.what()), "application/json");
        }
    });

    // LCM API
    svr.Post("/lcm", [](const httplib::Request& req, httplib::Response& res) {
        try {
            vector<int> numbers = parseNumbers(req.body);
            if (numbers.empty()) {
                res.set_content(createJsonResponse(false, 0, "未输入数字"), "application/json");
                return;
            }
            int result = lcmMultiple(numbers);
            res.set_content(createJsonResponse(true, result), "application/json");
        } catch (const exception& e) {
            res.set_content(createJsonResponse(false, 0, e.what()), "application/json");
        }
    });

    // Calculator API
    svr.Post("/calc", [](const httplib::Request& req, httplib::Response& res) {
        try {
            string expr = parseExpression(req.body);
            if (expr.empty()) {
                res.set_content(createJsonResponse(false, 0, "表达式为空"), "application/json");
                return;
            }
            double result = evalExpression(expr);
            res.set_content(createJsonResponse(true, result), "application/json");
        } catch (const exception& e) {
            res.set_content(createJsonResponse(false, 0, e.what()), "application/json");
        }
    });

    cout << "🚀 C++ Math Server running on http://localhost:8080" << endl;
    cout << "📐 GCD API: POST /gcd" << endl;
    cout << "📏 LCM API: POST /lcm" << endl;
    cout << "🔢 Calculator API: POST /calc" << endl;
    cout << "Press Ctrl+C to stop" << endl;

    svr.listen("localhost", 8080);
    
    return 0;
}