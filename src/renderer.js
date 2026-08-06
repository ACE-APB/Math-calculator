let currentMode = 'gcd';

function switchMode(mode) {
    currentMode = mode;
    document.querySelectorAll('.menu-btn').forEach(btn => {
        btn.classList.remove('active');
        if (btn.dataset.mode === mode) btn.classList.add('active');
    });
    document.querySelectorAll('.mode-content').forEach(el => el.classList.add('hidden'));
    document.getElementById(mode + 'Mode').classList.remove('hidden');
    document.getElementById('resultValue').textContent = '等待计算...';
    document.getElementById('resultValue').className = 'value';
    updateStatus('切换到 ' + getModeName(mode));
}

function getModeName(mode) {
    const names = {
        'gcd': 'GCD',
        'lcm': 'LCM',
        'prime': '质数',
        'phi': '欧拉函数',
        'modpow': '模幂',
        'equation': '不定方程',
        'calc': '计算器'
    };
    return names[mode] || mode;
}

function updateStatus(message, isError = false) {
    const status = document.getElementById('statusBar');
    status.textContent = '🔄 ' + message;
    status.style.background = isError ? '#ffebee' : '#e8f5e9';
    status.style.color = isError ? '#c62828' : '#2e7d32';
}

function showResult(value, isError = false, className = '') {
    const resultDiv = document.getElementById('resultValue');
    resultDiv.textContent = value;
    resultDiv.className = 'value' + (isError ? ' error' : '') + (className ? ' ' + className : '');
}

// ==================== 后端调用 ====================
async function callBackend(endpoint, data) {
    try {
        const response = await fetch('http://localhost:8080/' + endpoint, {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify(data)
        });
        if (!response.ok) throw new Error('HTTP ' + response.status);
        return await response.json();
    } catch (error) {
        return simulateCalculation(endpoint, data);
    }
}

function simulateCalculation(endpoint, data) {
    if (endpoint === 'gcd') {
        const nums = data.numbers.map(Number);
        return { success: true, result: gcdMultiple(nums) };
    } else if (endpoint === 'lcm') {
        const nums = data.numbers.map(Number);
        return { success: true, result: lcmMultiple(nums) };
    } else if (endpoint === 'prime') {
        const nums = data.numbers.map(Number);
        if (nums.length === 1) {
            return { success: true, result: isPrime(nums[0]) };
        } else {
            return { success: true, result: filterPrimes(nums) };
        }
    } else if (endpoint === 'phi') {
        return { success: true, result: eulerPhi(data.n) };
    } else if (endpoint === 'modpow') {
        return { success: true, result: modPow(data.n, data.m, data.b) };
    } else if (endpoint === 'equation') {
        const result = solveEquation(data.a, data.b, data.c);
        return { success: true, ...result };
    } else if (endpoint === 'calc') {
        try {
            const result = Function('"use strict"; return (' + data.expression + ')')();
            return { success: true, result: result };
        } catch (e) {
            return { success: false, error: e.message };
        }
    }
}

// ==================== GCD ====================
function gcd(a, b) {
    a = Math.abs(a); b = Math.abs(b);
    while (b !== 0) { [a, b] = [b, a % b]; }
    return a;
}

function gcdMultiple(nums) {
    if (nums.length === 0) return 0;
    let result = nums[0];
    for (let i = 1; i < nums.length; i++) {
        result = gcd(result, nums[i]);
        if (result === 1) break;
    }
    return result;
}

// ==================== LCM ====================
function lcm(a, b) {
    if (a === 0 || b === 0) return 0;
    return Math.abs(a * b) / gcd(a, b);
}

function lcmMultiple(nums) {
    if (nums.length === 0) return 0;
    let result = nums[0];
    for (let i = 1; i < nums.length; i++) {
        result = lcm(result, nums[i]);
    }
    return result;
}

// ==================== 质数 ====================
function isPrime(num) {
    if (num < 2) return false;
    if (num === 2) return true;
    if (num % 2 === 0) return false;
    for (let i = 3; i * i <= num; i += 2) {
        if (num % i === 0) return false;
    }
    return true;
}

function filterPrimes(nums) {
    return nums.filter(n => isPrime(n));
}

// ==================== 欧拉函数 ====================

// 辗转相除法求最大公约数（欧拉专用）
function gcdForPhi(a, b) {
    a = Math.abs(a);
    b = Math.abs(b);
    if (a < b) [a, b] = [b, a];
    while (b !== 0) {
        [a, b] = [b, a % b];
    }
    return a;
}

// 欧拉函数 - 模拟法
function eulerPhi(n) {
    if (n <= 0) return 0;
    if (n === 1) return 1;
    
    let count = 0;
    for (let i = 1; i <= n; i++) {
        if (gcdForPhi(i, n) === 1) {
            count++;
        }
    }
    return count;
}

// 欧拉函数 - 优化版
function eulerPhiFast(n) {
    if (n <= 0) return 0;
    if (n === 1) return 1;
    
    let result = n;
    let num = n;
    
    for (let p = 2; p * p <= num; p++) {
        if (num % p === 0) {
            while (num % p === 0) {
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

// 生成欧拉函数详细过程
function generatePhiDetails(n) {
    let lines = [];
    lines.push('📐 φ(' + n + ') = 1 到 ' + n + ' 中与 ' + n + ' 互质的数的个数');
    lines.push('');
    
    if (n <= 0) {
        lines.push('❌ n 必须为正整数');
        return lines;
    }
    
    let coprimeNumbers = [];
    for (let i = 1; i <= n; i++) {
        if (gcdForPhi(i, n) === 1) {
            coprimeNumbers.push(i);
        }
    }
    
    lines.push('与 ' + n + ' 互质的数：');
    lines.push('  ' + coprimeNumbers.join(', '));
    lines.push('');
    
    lines.push('个数：' + coprimeNumbers.length);
    lines.push('');
    
    let resultFast = eulerPhiFast(n);
    lines.push('💡 验证（质因数分解法）：');
    lines.push('  φ(' + n + ') = ' + resultFast);
    
    return lines;
}

// ==================== 模幂运算 ====================
function fastPow(base, exp, mod) {
    if (mod === 1) return 0;
    base = ((base % mod) + mod) % mod;
    let result = 1;
    while (exp > 0) {
        if (exp & 1) {
            result = (result * base) % mod;
        }
        base = (base * base) % mod;
        exp >>= 1;
    }
    return result;
}

function findCycle(n, b) {
    if (b === 1) return 0;
    if (gcd(n, b) !== 1) return -1;
    n = ((n % b) + b) % b;
    let current = 1;
    for (let i = 1; i <= b; i++) {
        current = (current * n) % b;
        if (current === 1) {
            return i;
        }
    }
    return -1;
}

function modPow(n, m, b) {
    if (b === 1) return 0;
    if (m === 0) return 1 % b;
    return fastPow(n, m, b);
}

function generateModPowDetails(n, m, b) {
    let lines = [];
    lines.push('📐 ' + n + '^' + m + ' ≡ x (mod ' + b + ')');
    lines.push('');
    
    if (b === 1) {
        lines.push('💡 任何数 mod 1 = 0');
        return lines;
    }
    
    let base = ((n % b) + b) % b;
    lines.push('第1步：底数取模');
    lines.push('  ' + n + ' mod ' + b + ' = ' + base);
    lines.push('');
    
    let result = fastPow(n, m, b);
    lines.push('第2步：快速幂计算');
    lines.push('  ' + n + '^' + m + ' ≡ ' + result + ' (mod ' + b + ')');
    lines.push('');
    
    let cycle = findCycle(n, b);
    if (cycle !== -1 && m > cycle) {
        lines.push('💡 循环节：');
        lines.push('  ' + n + '^' + cycle + ' ≡ 1 (mod ' + b + ')');
        let exp = m % cycle;
        if (exp === 0) exp = cycle;
        lines.push('  ' + n + '^' + m + ' = ' + n + '^(' + cycle + '×' + Math.floor(m/cycle) + ' + ' + exp + ')');
        lines.push('  ≡ ' + n + '^' + exp + ' (mod ' + b + ')');
        let result2 = fastPow(n, exp, b);
        lines.push('  = ' + result2);
    } else if (cycle === -1) {
        lines.push('💡 注意：gcd(' + n + ', ' + b + ') ≠ 1');
        lines.push('  没有循环节，直接使用快速幂');
    }
    
    lines.push('');
    lines.push('✅ 结果：x = ' + result);
    return lines;
}

// ==================== 不定方程 ====================
function exgcd(a, b) {
    if (b === 0) {
        return { gcd: Math.abs(a), x: 1, y: 0 };
    }
    const result = exgcd(b, a % b);
    return {
        gcd: result.gcd,
        x: result.y,
        y: result.x - Math.floor(a / b) * result.y
    };
}

function solveEquation(a, b, c) {
    if (a === 0 && b === 0) {
        if (c === 0) return { hasSolution: true, message: '方程有无数解' };
        return { hasSolution: false, message: '方程无解' };
    }
    if (a === 0) {
        if (c % b === 0) return { hasSolution: true, x0: 0, y0: c / b, gcd: Math.abs(b), a: a, b: b, c: c };
        return { hasSolution: false, message: '方程无解（b 不能整除 c）' };
    }
    if (b === 0) {
        if (c % a === 0) return { hasSolution: true, x0: c / a, y0: 0, gcd: Math.abs(a), a: a, b: b, c: c };
        return { hasSolution: false, message: '方程无解（a 不能整除 c）' };
    }
    let absA = Math.abs(a), absB = Math.abs(b);
    let result = exgcd(absA, absB);
    let gcd = result.gcd;
    if (c % gcd !== 0) {
        return { hasSolution: false, message: '方程无解（' + c + ' 不能被 ' + gcd + ' 整除）' };
    }
    let x = result.x, y = result.y;
    if (a < 0) x = -x;
    if (b < 0) y = -y;
    return { hasSolution: true, x0: x * (c / gcd), y0: y * (c / gcd), gcd: gcd, a: a, b: b, c: c };
}

function formatEquationResult(result) {
    if (!result.hasSolution) return '❌ ' + result.message;
    if (result.message) return result.message;
    let a = result.a, b = result.b, c = result.c;
    let x0 = result.x0, y0 = result.y0, gcd = result.gcd;
    let output = '📐 方程：' + a + 'x + ' + b + 'y = ' + c + '\n';
    output += '🔢 gcd(' + a + ', ' + b + ') = ' + gcd + '\n\n';
    output += '📌 特解：\n  x₀ = ' + x0 + '\n  y₀ = ' + y0 + '\n\n';
    output += '✅ 验证：\n  ' + a + '×' + x0 + ' + ' + b + '×' + y0 + ' = ' + (a*x0 + b*y0) + '\n\n';
    let t1 = b / gcd, t2 = a / gcd;
    if (t1 < 0) { t1 = -t1; t2 = -t2; }
    output += '📝 通解：\n  x = ' + x0 + ' + ' + t1 + 't\n  y = ' + y0 + ' - ' + t2 + 't\n  其中 t 为任意整数\n\n';
    output += '📊 几组具体解：\n';
    for (let t = -3; t <= 3; t++) {
        let x = x0 + t1 * t, y = y0 - t2 * t;
        output += '  t=' + t + ': x=' + x + ', y=' + y;
        output += '  ✅ ' + a + '×' + x + ' + ' + b + '×' + y + ' = ' + (a*x + b*y) + '\n';
    }
    return output;
}

// ==================== 计算函数 ====================
async function calculateGCD() {
    const input = document.getElementById('gcdInput').value;
    const numbers = input.trim().split(/\s+/).map(Number);
    if (numbers.some(isNaN) || numbers.length === 0) {
        showResult('请输入有效的整数！', true);
        return;
    }
    updateStatus('正在计算 GCD...');
    const result = await callBackend('gcd', { numbers: numbers });
    if (result.success) { showResult(result.result); updateStatus('计算完成！'); }
    else { showResult('错误：' + result.error, true); updateStatus('计算失败', true); }
}

async function calculateLCM() {
    const input = document.getElementById('lcmInput').value;
    const numbers = input.trim().split(/\s+/).map(Number);
    if (numbers.some(isNaN) || numbers.length === 0) {
        showResult('请输入有效的整数！', true);
        return;
    }
    updateStatus('正在计算 LCM...');
    const result = await callBackend('lcm', { numbers: numbers });
    if (result.success) { showResult(result.result); updateStatus('计算完成！'); }
    else { showResult('错误：' + result.error, true); updateStatus('计算失败', true); }
}

async function calculatePrime() {
    const input = document.getElementById('primeInput').value;
    const numbers = input.trim().split(/\s+/).map(Number);
    if (numbers.some(isNaN) || numbers.length === 0) {
        showResult('请输入有效的整数！', true);
        return;
    }
    updateStatus('正在判断质数...');
    const result = await callBackend('prime', { numbers: numbers });
    if (result.success) {
        if (numbers.length === 1) {
            showResult(result.result ? '✅ ' + numbers[0] + ' 是质数' : '❌ ' + numbers[0] + ' 不是质数',
                false, result.result ? 'prime-true' : 'prime-false');
        } else {
            const primes = result.result;
            showResult(primes.length === 0 ? '这些数中没有质数 😅' : '质数：' + primes.join(', ') + ' ✅');
        }
        updateStatus('计算完成！');
    } else {
        showResult('错误：' + result.error, true);
        updateStatus('计算失败', true);
    }
}

// 欧拉函数
async function calculatePhi() {
    const input = document.getElementById('phiInput').value.trim();
    
    if (!input) {
        showResult('请输入 n！', true);
        return;
    }
    
    const n = parseInt(input);
    
    if (isNaN(n)) {
        showResult('请输入有效的整数！', true);
        return;
    }
    
    if (n <= 0) {
        showResult('请输入正整数！', true);
        return;
    }
    
    updateStatus('正在计算 φ(' + n + ')...');
    
    try {
        const result = await callBackend('phi', { n: n });
        if (result.success) {
            const details = generatePhiDetails(n);
            showResult(details.join('\n'));
            updateStatus('计算完成！');
        } else {
            showResult('错误：' + result.error, true);
            updateStatus('计算失败', true);
        }
    } catch (error) {
        const details = generatePhiDetails(n);
        showResult(details.join('\n'));
        updateStatus('使用 JavaScript 引擎计算完成');
    }
}

// 模幂
async function calculateModPow() {
    const nInput = document.getElementById('modpowN').value.trim();
    const mInput = document.getElementById('modpowM').value.trim();
    const bInput = document.getElementById('modpowB').value.trim();
    
    if (!nInput || !mInput || !bInput) {
        showResult('请完整填写 n、m、b！', true);
        return;
    }
    
    const n = parseInt(nInput);
    const m = parseInt(mInput);
    const b = parseInt(bInput);
    
    if (isNaN(n) || isNaN(m) || isNaN(b)) {
        showResult('请输入有效的整数！', true);
        return;
    }
    
    if (b <= 0) {
        showResult('模数 b 必须为正整数！', true);
        return;
    }
    
    if (m < 0) {
        showResult('指数 m 必须为非负整数！', true);
        return;
    }
    
    updateStatus('正在计算模幂...');
    
    try {
        const result = await callBackend('modpow', { n: n, m: m, b: b });
        if (result.success) {
            const details = generateModPowDetails(n, m, b);
            showResult(details.join('\n'));
            updateStatus('计算完成！');
        } else {
            showResult('错误：' + result.error, true);
            updateStatus('计算失败', true);
        }
    } catch (error) {
        const details = generateModPowDetails(n, m, b);
        showResult(details.join('\n'));
        updateStatus('使用 JavaScript 引擎计算完成');
    }
}

// 不定方程
async function calculateEquation() {
    const input = document.getElementById('equationInput').value.trim();
    if (!input) { showResult('请输入方程！', true); return; }
    updateStatus('正在求解不定方程...');
    try {
        let s = input.replace(/\s/g, '');
        let eqPos = s.indexOf('=');
        if (eqPos === -1) throw new Error('未找到 "=" 符号');
        let left = s.substring(0, eqPos), right = s.substring(eqPos + 1);
        let c = parseInt(right);
        if (isNaN(c)) throw new Error('等号右侧不是整数');
        let terms = [], current = '';
        for (let i = 0; i < left.length; i++) {
            let ch = left[i];
            if ((ch === '+' || ch === '-') && i > 0) { terms.push(current); current = ch; }
            else current += ch;
        }
        if (current) terms.push(current);
        if (terms.length === 0) terms = [left];
        let a = 0, b = 0;
        for (let term of terms) {
            let xPos = term.indexOf('x'), yPos = term.indexOf('y');
            if (xPos !== -1) {
                let coefStr = term.substring(0, xPos);
                if (coefStr === '' || coefStr === '+') a = 1;
                else if (coefStr === '-') a = -1;
                else a = parseInt(coefStr);
                if (isNaN(a)) throw new Error('x 的系数无效');
            } else if (yPos !== -1) {
                let coefStr = term.substring(0, yPos);
                if (coefStr === '' || coefStr === '+') b = 1;
                else if (coefStr === '-') b = -1;
                else b = parseInt(coefStr);
                if (isNaN(b)) throw new Error('y 的系数无效');
            }
        }
        const result = await callBackend('equation', { a: a, b: b, c: c });
        if (result.success) {
            showResult(format
