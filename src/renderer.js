let currentMode = 'gcd';

// 切换模式
function switchMode(mode) {
    currentMode = mode;
    
    // 更新按钮状态
    document.querySelectorAll('.menu-btn').forEach(btn => {
        btn.classList.remove('active');
        if (btn.dataset.mode === mode) {
            btn.classList.add('active');
        }
    });
    
    // 显示对应的内容
    document.querySelectorAll('.mode-content').forEach(el => {
        el.classList.add('hidden');
    });
    document.getElementById(mode + 'Mode').classList.remove('hidden');
    
    // 清空结果
    document.getElementById('resultValue').textContent = '等待计算...';
    document.getElementById('resultValue').className = 'value';
    
    updateStatus('切换到 ' + getModeName(mode));
}

function getModeName(mode) {
    const names = {
        'gcd': 'GCD 最大公约数',
        'lcm': 'LCM 最小公倍数',
        'calc': '代数计算器'
    };
    return names[mode] || mode;
}

// 更新状态栏
function updateStatus(message, isError = false) {
    const status = document.getElementById('statusBar');
    status.textContent = '🔄 ' + message;
    status.style.background = isError ? '#ffebee' : '#e8f5e9';
    status.style.color = isError ? '#c62828' : '#2e7d32';
}

// 显示结果
function showResult(value, isError = false) {
    const resultDiv = document.getElementById('resultValue');
    resultDiv.textContent = value;
    resultDiv.className = 'value' + (isError ? ' error' : '');
}

// 调用后端
async function callBackend(endpoint, data) {
    try {
        const response = await fetch('http://localhost:8080/' + endpoint, {
            method: 'POST',
            headers: {
                'Content-Type': 'application/json',
            },
            body: JSON.stringify(data)
        });
        
        if (!response.ok) {
            throw new Error('HTTP ' + response.status);
        }
        
        return await response.json();
    } catch (error) {
        console.log('C++后端未连接，使用JavaScript引擎');
        return simulateCalculation(endpoint, data);
    }
}

// JavaScript 模拟计算（备用引擎）
function simulateCalculation(endpoint, data) {
    if (endpoint === 'gcd') {
        const nums = data.numbers.map(Number);
        const result = gcdMultiple(nums);
        return { success: true, result: result };
    } else if (endpoint === 'lcm') {
        const nums = data.numbers.map(Number);
        const result = lcmMultiple(nums);
        return { success: true, result: result };
    } else if (endpoint === 'calc') {
        try {
            const result = Function('"use strict"; return (' + data.expression + ')')();
            return { success: true, result: result };
        } catch (e) {
            return { success: false, error: e.message };
        }
    }
}

// JavaScript GCD 算法
function gcd(a, b) {
    a = Math.abs(a);
    b = Math.abs(b);
    while (b !== 0) {
        [a, b] = [b, a % b];
    }
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

// 计算 GCD
async function calculateGCD() {
    const input = document.getElementById('gcdInput').value;
    const numbers = input.trim().split(/\s+/).map(Number);
    
    if (numbers.some(isNaN)) {
        showResult('请输入有效的整数！', true);
        updateStatus('输入错误', true);
        return;
    }
    
    updateStatus('正在计算 GCD...');
    
    const result = await callBackend('gcd', { numbers: numbers });
    if (result.success) {
        showResult(result.result);
        updateStatus('计算完成！');
    } else {
        showResult('错误：' + result.error, true);
        updateStatus('计算失败', true);
    }
}

// 计算 LCM
async function calculateLCM() {
    const input = document.getElementById('lcmInput').value;
    const numbers = input.trim().split(/\s+/).map(Number);
    
    if (numbers.some(isNaN)) {
        showResult('请输入有效的整数！', true);
        updateStatus('输入错误', true);
        return;
    }
    
    updateStatus('正在计算 LCM...');
    
    const result = await callBackend('lcm', { numbers: numbers });
    if (result.success) {
        showResult(result.result);
        updateStatus('计算完成！');
    } else {
        showResult('错误：' + result.error, true);
        updateStatus('计算失败', true);
    }
}

// 计算表达式
async function calculateExpression() {
    const expression = document.getElementById('calcInput').value.trim();
    
    if (!expression) {
        showResult('请输入表达式！', true);
        updateStatus('输入错误', true);
        return;
    }
    
    updateStatus('正在计算表达式...');
    
    const result = await callBackend('calc', { expression: expression });
    if (result.success) {
        const value = result.result;
        let display = value.toString();
        if (Number.isInteger(value)) {
            display = value.toString();
        } else {
            display = value.toFixed(4);
        }
        showResult(display);
        updateStatus('计算完成！');
    } else {
        showResult('错误：' + result.error, true);
        updateStatus('计算失败', true);
    }
}

// 事件绑定 - 点击切换模式
document.querySelectorAll('.menu-btn').forEach(btn => {
    btn.addEventListener('click', function() {
        switchMode(this.dataset.mode);
    });
});

// 初始化
document.addEventListener('DOMContentLoaded', function() {
    updateStatus('就绪，使用 C++ 后端引擎（未连接则自动使用JS引擎）');
});