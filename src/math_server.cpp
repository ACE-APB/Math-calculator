/**
 * @file math_server.cpp
 * @brief C++ HTTP 服务器，提供数学计算API
 * @version 2.0.0
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

// ================================================================
// ==================== GCD & LCM ================================
// ================================================================

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

// ================================================================
// ==================== 质数判断 ================================
// ================================================================

bool is_prime(int a) {
    if (a < 2) return false;
    if (a == 2) return true;
    if (a % 2 == 0) return false;
    for (int i = 3; i * i <= a; i += 2) {
        if (a % i == 0) return false;
    }
    return true;
}

vector<int> filterPrimes(const vector<int>& numbers) {
    vector<int> primes;
    for (int num : numbers) {
        if (is_prime(num)) {
            primes.push_back(num);
        }
    }
    return primes;
}

// ================================================================
// ==================== 欧拉函数 ================================
// ================================================================

// 欧拉函数 - 优化法（质因数分解）
int eulerPhi(int n) {
    if (n <= 0) return 0;
    if (n == 1) return 1;
    
    int result = n;
    int num = n;
    
    for (int p = 2; p * p <= num; p++) {
        if (num % p == 0) {
            while (num % p == 0) {
                num /= p;
            }
            result -= result / p;
        }
    }
    
    if (num > 1) {
        result -= result / num;
    }
    
    return result;
}

// ================================================================
// ==================== 模幂运算 ================================
// ================================================================

long long fastPow(long long base, long long exp, long long mod) {
    if (mod == 1) return 0;
    base = ((base % mod) + mod) % mod;
    long long result = 1;
    while (exp > 0) {
        if (exp & 1) {
            result = (result * base) % mod;
        }
        base = (base * base) % mod;
        exp >>= 1;
    }
    return result;
}

long long findCycle(long long n, long long b) {
    if (b == 1) return 0;
    if (gcd(static_cast<int>(n), static_cast<int>(b)) != 1) return -1;
    n = ((n % b) + b) % b;
    long long current = 1;
    for (long long i = 1; i <= b; i++) {
        current = (current * n) % b;
        if (current == 1) {
            return i;
        }
    }
    return -1;
}

long long modPow(long long n, long long m, long long b) {
    if (b == 1) return 0;
    if (m == 0) return 1 % b;
    return fastPow(n, m, b);
}

// ================================================================
// ==================== 表达式求值 ================================
// ================================================================

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

// ================================================================
// ==================== 二元一次不定方程 ==========================
// ================================================================

int exgcd(int a, int b, int &x, int &y) {
    if (b == 0) {
        x = 1;
        y = 0;
        return a;
    }
    int x1, y1;
    int gcd = exgcd(b, a % b, x1, y1);
    x = y1;
    y = x1 - (a / b) * y1;
    return gcd;
}

struct EquationResult {
    bool hasSolution;
    int x0, y0;
    int gcd;
    int a, b, c;
    string message;
};

EquationResult solveIndefiniteEquation(int a, int b, int c) {
    EquationResult result;
    result.a = a;
    result.b = b;
    result.c = c;
    
    if (a == 0 && b == 0) {
        if (c == 0) {
            result.hasSolution = true;
            result.message = "方程有无数解（任何整数都行）";
        } else {
            result.hasSolution = false;
            result.message = "方程无解";
        }
        return result;
    }
    
    if (a == 0) {
        if (c % b == 0) {
            result.hasSolution = true;
            result.x0 = 0;
            result.y0 = c / b;
            result.gcd = abs(b);
        } else {
            result.hasSolution = false;
            result.message = "方程无解（b 不能整除 c）";
        }
        return result;
    }
    
    if (b == 0) {
        if (c % a == 0) {
            result.hasSolution = true;
            result.x0 = c / a;
            result.y0 = 0;
            result.gcd = abs(a);
        } else {
            result.hasSolution = false;
            result.message = "方程无解（a 不能整除 c）";
        }
        return result;
    }
    
    int absA = abs(a);
    int absB = abs(b);
    int x, y;
    int gcd = exgcd(absA, absB, x, y);
    
    if (c % gcd != 0) {
        result.hasSolution = false;
        result.message = "方程无解（" + to_string(c) + " 不能被 " + to_string(gcd) + " 整除）";
        return result;
    }
    
    if (a < 0) x = -x;
    if (b < 0) y = -y;
    
    result.hasSolution = true;
    result.x0 = x * (c / gcd);
    result.y0 = y * (c / gcd);
    result.gcd = gcd;
    
    return result;
}

// ================================================================
// ==================== JSON处理 ================================
// ================================================================

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

string createJsonResponseBool(bool success, bool result, const string& error = "") {
    stringstream ss;
    ss << "{";
    ss << "\"success\":" << (success ? "true" : "false") << ",";
    if (success) {
        ss << "\"result\":" << (result ? "true" : "false");
    } else {
        ss << "\"error\":\"" << error << "\"";
    }
    ss << "}";
    return ss.str();
}

string createJsonResponseArray(bool success, const vector<int>& result, const string& error = "") {
    stringstream ss;
    ss << "{";
    ss << "\"success\":" << (success ? "true" : "false") << ",";
    if (success) {
        ss << "\"result\":[";
        for (size_t i = 0; i < result.size(); ++i) {
            if (i > 0) ss << ",";
            ss << result[i];
        }
        ss << "]";
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

bool parseModPowParams(const string& json, long long& n, long long& m, long long& b) {
    size_t nPos = json.find("\"n\":");
    size_t mPos = json.find("\"m\":");
    size_t bPos = json.find("\"b\":");
    
    if (nPos == string::npos || mPos == string::npos || bPos == string::npos) {
        return false;
    }
    
    try {
        n = stoll(json.substr(nPos + 4));
        m = stoll(json.substr(mPos + 4));
        b = stoll(json.substr(bPos + 4));
        return true;
    } catch (...) {
        return false;
    }
}

bool parseEquationParams(const string& json, int& a, int& b, int& c) {
    size_t aPos = json.find("\"a\":");
    size_t bPos = json.find("\"b\":");
    size_t cPos = json.find("\"c\":");
    
    if (aPos == string::npos || bPos == string::npos || cPos == string::npos) {
        return false;
    }
    
    try {
        a = stoi(json.substr(aPos + 4));
        b = stoi(json.substr(bPos + 4));
        c = stoi(json.substr(cPos + 4));
        return true;
    } catch (...) {
        return false;
    }
}

bool parsePhiParams(const string& json, int& n) {
    size_t nPos = json.find("\"n\":");
    if (nPos == string::npos) return false;
    
    try {
        n = stoi(json.substr(nPos + 4));
        return true;
    } catch (...) {
        return false;
    }
}

// ================================================================
// ==================== 主函数 ================================
// ================================================================

int main() {
    httplib::Server svr;

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

    // Prime API
    svr.Post("/prime", [](const httplib::Request& req, httplib::Response& res) {
        try {
            vector<int> numbers = parseNumbers(req.body);
            if (numbers.empty()) {
                res.set_content(createJsonResponse(false, 0, "未输入数字"), "application/json");
                return;
            }
            if (numbers.size() == 1) {
                bool result = is_prime(numbers[0]);
                res.set_content(createJsonResponseBool(true, result), "application/json");
            } else {
                vector<int> primes = filterPrimes(numbers);
                res.set_content(createJsonResponseArray(true, primes), "application/json");
            }
        } catch (const exception& e) {
            res.set_content(createJsonResponse(false, 0, e.what()), "application/json");
        }
    });

    // Phi API - 欧拉函数
    svr.Post("/phi", [](const httplib::Request& req, httplib::Response& res) {
        try {
            int n;
            if (!parsePhiParams(req.body, n)) {
                res.set_content(createJsonResponse(false, 0, "缺少参数 n"), "application/json");
                return;
            }
            
            if (n <= 0) {
                res.set_content(createJsonResponse(false, 0, "n 必须为正整数"), "application/json");
                return;
            }
            
            int result = eulerPhi(n);
            
            stringstream ss;
            ss << "{";
            ss << "\"success\":true,";
            ss << "\"result\":" << result;
            ss << "}";
            res.set_content(ss.str(), "application/json");
        } catch (const exception& e) {
            res.set_content(createJsonResponse(false, 0, e.what()), "application/json");
        }
    });

    // ModPow API
    svr.Post("/modpow", [](const httplib::Request& req, httplib::Response& res) {
        try {
            long long n, m, b;
            if (!parseModPowParams(req.body, n, m, b)) {
                res.set_content(createJsonResponse(false, 0, "缺少参数 n, m, b"), "application/json");
                return;
            }
            
            if (b <= 0) {
                res.set_content(createJsonResponse(false, 0, "模数 b 必须为正整数"), "application/json");
                return;
            }
            
            if (m < 0) {
                res.set_content(createJsonResponse(false, 0, "指数 m 必须为非负整数"), "application/json");
                return;
            }
            
            long long result = modPow(n, m, b);
            
            stringstream ss;
            ss << "{";
            ss << "\"success\":true,";
            ss << "\"result\":" << result;
            ss << "}";
            res.set_content(ss.str(), "application/json");
        } catch (const exception& e) {
            res.set_content(createJsonResponse(false, 0, e.what()), "application/json");
        }
    });

    // Equation API
    svr.Post("/equation", [](const httplib::Request& req, httplib::Response& res) {
        try {
            int a, b, c;
            if (!parseEquationParams(req.body, a, b, c)) {
                res.set_content(createJsonResponse(false, 0, "缺少参数 a, b, c"), "application/json");
                return;
            }
            
            EquationResult result = solveIndefiniteEquation(a, b, c);
            
            if (result.hasSolution) {
                stringstream ss;
                ss << "{";
                ss << "\"success\":true,";
                ss << "\"hasSolution\":true,";
                ss << "\"x0\":" << result.x0 << ",";
                ss << "\"y0\":" << result.y0 << ",";
                ss << "\"gcd\":" << result.gcd << ",";
                ss << "\"a\":" << result.a << ",";
                ss << "\"b\":" << result.b << ",";
                ss << "\"c\":" << result.c;
                if (!result.message.empty()) {
                    ss << ",\"message\":\"" << result.message << "\"";
                }
                ss << "}";
                res.set_content(ss.str(), "application/json");
            } else {
                stringstream ss;
                ss << "{";
                ss << "\"success\":true,";
                ss << "\"hasSolution\":false,";
                ss << "\"message\":\"" << result.message << "\"";
                ss << "}";
                res.set_content(ss.str(), "application/json");
            }
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

    cout << "========================================" << endl;
    cout << "🚀 C++ Math Server v2.0" << endl;
    cout << "========================================" << endl;
    cout << "📐 GCD API:        POST /gcd" << endl;
    cout << "📏 LCM API:        POST /lcm" << endl;
    cout << "🔢 Prime API:      POST /prime" << endl;
    cout << "φ  Phi API:        POST /phi" << endl;
    cout << "⚡ ModPow API:     POST /modpow" << endl;
    cout << "📝 Equation API:   POST /equation" << endl;
    cout << "🧮 Calculator API: POST /calc" << endl;
    cout << "========================================" << endl;
    cout << "📍 监听地址: http://localhost:8080" << endl;
    cout << "🔄 Press Ctrl+C to stop" << endl;
    cout << "========================================" << endl;

    svr.listen("localhost", 8080);
    
    return 0;
}
