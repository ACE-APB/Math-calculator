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
