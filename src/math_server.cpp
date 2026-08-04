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

// ==================== GCD ====================

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

// ==================== 质数判断 ====================

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

// ==================== JSON处理 ====================

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

// ================================================================
// ==================== 🎯 二元一次不定方程 ====================
// ================================================================

// 扩展欧几里得算法：
// 求 ax + by = gcd(a, b) 的一组整数解
// 
// 原理：利用欧几里得算法（辗转相除法）的逆向推导
// 
// 举例：求 3x + 5y = 1 的解
// 第一步：5 = 1×3 + 2
// 第二步：3 = 1×2 + 1
// 第三步：2 = 2×1 + 0
// 
// 逆向推导：
// 1 = 3 - 1×2
//   = 3 - 1×(5 - 1×3)
//   = 3 - 1×5 + 1×3
//   = 2×3 - 1×5
// 所以：2×3 + (-1)×5 = 1
// 得到：x = 2, y = -1
//
// 参数：a, b（系数），x, y（通过引用返回解）
// 返回：gcd(a, b)
int exgcd(int a, int b, int &x, int &y) {
    // 基本情况：b == 0 时，gcd(a, 0) = a
    // 此时：a×1 + 0×0 = a，所以 x = 1, y = 0
    if (b == 0) {
        x = 1;
        y = 0;
        return a;
    }
    
    // 递归调用：求 b 和 a%b 的解
    // 为什么是 a%b？因为欧几里得算法：gcd(a, b) = gcd(b, a%b)
    int x1, y1;
    int gcd = exgcd(b, a % b, x1, y1);
    
    // 回溯时更新 x 和 y
    // 已知：b×x1 + (a%b)×y1 = gcd
    // 因为：a%b = a - (a/b)×b
    // 所以：b×x1 + (a - (a/b)×b)×y1 = gcd
    //      b×x1 + a×y1 - (a/b)×b×y1 = gcd
    //      a×y1 + b×(x1 - (a/b)×y1) = gcd
    // 对比：a×x + b×y = gcd
    // 得到：x = y1, y = x1 - (a/b)×y1
    x = y1;
    y = x1 - (a / b) * y1;
    
    return gcd;
}

// 求解二元一次不定方程：ax + by = c
// 
// 核心定理：方程 ax + by = c 有整数解
//           ⇔ gcd(a, b) 能整除 c
//
// 步骤：
// 1. 用扩展欧几里得求出 ax + by = gcd(a, b) 的特解
// 2. 将特解乘以 c/gcd，得到原方程的特解
// 3. 通解 = 特解 + 通解项
//
// 通解公式：
//   x = x₀ + (b/gcd) × t
//   y = y₀ - (a/gcd) × t
//   其中 t 为任意整数
//
// 举例：3x + 5y = 7
// 1. gcd(3, 5) = 1，能整除 7
// 2. 3x + 5y = 1 的特解：x = 2, y = -1
// 3. 原方程特解：x₀ = 2×7 = 14, y₀ = -1×7 = -7
// 4. 通解：x = 14 + 5t, y = -7 - 3t
struct EquationResult {
    bool hasSolution;   // 是否有解
    int x0, y0;         // 特解
    int gcd;            // gcd(a, b)
    int a, b, c;        // 原方程系数
    string message;     // 错误信息
};

EquationResult solveIndefiniteEquation(int a, int b, int c) {
    EquationResult result;
    result.a = a;
    result.b = b;
    result.c = c;
    
    // ========== 特殊情况处理 ==========
    
    // 情况1：a = 0 且 b = 0
    // 方程变成：0x + 0y = c
    // 如果 c = 0，则任何整数都是解
    // 如果 c ≠ 0，则无解
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
    
    // 情况2：a = 0，方程变成：0x + by = c
    // 即：by = c，需要 b 能整除 c
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
    
    // 情况3：b = 0，方程变成：ax + 0y = c
    // 即：ax = c，需要 a 能整除 c
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
    
    // ========== 一般情况：a ≠ 0 且 b ≠ 0 ==========
    
    // 使用扩展欧几里得求 ax + by = gcd(a, b) 的特解
    // 注意：exgcd 需要正数，所以取绝对值
    int absA = abs(a);
    int absB = abs(b);
    int x, y;
    int gcd = exgcd(absA, absB, x, y);
    
    // 判断是否有解：c 必须能被 gcd 整除
    // 这是数论中的核心定理！
    if (c % gcd != 0) {
        result.hasSolution = false;
        result.message = "方程无解（" + to_string(c) + " 不能被 " + to_string(gcd) + " 整除）";
        return result;
    }
    
    // 如果原系数是负数，调整符号
    // 因为 exgcd 是用正数算的，现在要对应回原方程
    if (a < 0) x = -x;
    if (b < 0) y = -y;
    
    // 计算原方程的特解
    // 基本原理：如果 ax₀ + by₀ = gcd(a, b)
    // 那么：a×x₀×(c/gcd) + b×y₀×(c/gcd) = c
    // 所以：x₀ = x × (c/gcd)，y₀ = y × (c/gcd)
    result.hasSolution = true;
    result.x0 = x * (c / gcd);
    result.y0 = y * (c / gcd);
    result.gcd = gcd;
    
    return result;
}

// ================================================================
// ==================== 主函数 ====================
// ================================================================

int main() {
    httplib::Server svr;

    // CORS支持（允许网页跨域访问）
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

    // Equation API - 求解不定方程
    svr.Post("/equation", [](const httplib::Request& req, httplib::Response& res) {
        try {
            // 解析 JSON 中的 a, b, c
            size_t aPos = req.body.find("\"a\":");
            size_t bPos = req.body.find("\"b\":");
            size_t cPos = req.body.find("\"c\":");
            
            if (aPos == string::npos || bPos == string::npos || cPos == string::npos) {
                res.set_content(createJsonResponse(false, 0, "缺少参数 a, b, c"), "application/json");
                return;
            }
            
            // 提取数值（简化处理，实际应该用完整的 JSON 解析）
            int a = stoi(req.body.substr(aPos + 4));
            int b = stoi(req.body.substr(bPos + 4));
            int c = stoi(req.body.substr(cPos + 4));
            
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
    cout << "🚀 C++ Math Server 已启动" << endl;
    cout << "========================================" << endl;
    cout << "📐 GCD API: POST /gcd" << endl;
    cout << "📏 LCM API: POST /lcm" << endl;
    cout << "🔢 Prime API: POST /prime" << endl;
    cout << "📝 Equation API: POST /equation" << endl;
    cout << "🧮 Calculator API: POST /calc" << endl;
    cout << "========================================" << endl;
    cout << "📍 监听地址: http://localhost:8080" << endl;
    cout << "🔄 Press Ctrl+C to stop" << endl;
    cout << "========================================" << endl;

    svr.listen("localhost", 8080);
    
    return 0;
}
