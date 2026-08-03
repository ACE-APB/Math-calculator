const { app, BrowserWindow } = require('electron');
const { exec } = require('child_process');
const path = require('path');
const fs = require('fs');

let mainWindow;
let cppProcess;

// 启动C++服务器
function startCppServer() {
    const serverPath = path.join(__dirname, 'math_server.exe');
    
    // 检查exe是否存在
    if (!fs.existsSync(serverPath)) {
        console.log('⚠️ C++服务器未编译，使用JavaScript引擎');
        return false;
    }
    
    console.log('🚀 启动C++服务器...');
    cppProcess = exec(serverPath);
    
    cppProcess.stdout.on('data', (data) => {
        console.log(`[C++ Server] ${data}`);
    });
    
    cppProcess.stderr.on('data', (data) => {
        console.error(`[C++ Server Error] ${data}`);
    });
    
    // 等待服务器启动
    return new Promise((resolve) => {
        setTimeout(() => resolve(true), 1000);
    });
}

function createWindow() {
    mainWindow = new BrowserWindow({
        width: 800,
        height: 700,
        minWidth: 600,
        minHeight: 500,
        webPreferences: {
            nodeIntegration: true,
            contextIsolation: false
        },
        title: '多功能数学工具'
    });

    mainWindow.loadFile(path.join(__dirname, 'index.html'));
    
    // 如果想调试，取消下面这行的注释
    // mainWindow.webContents.openDevTools();

    mainWindow.on('closed', () => {
        mainWindow = null;
        if (cppProcess) {
            cppProcess.kill();
        }
    });
}

// 应用启动
app.whenReady().then(async () => {
    // 尝试启动C++服务器
    await startCppServer();
    
    // 创建窗口
    createWindow();
});

app.on('window-all-closed', () => {
    if (process.platform !== 'darwin') {
        app.quit();
    }
});

// 当应用激活时（macOS）
app.on('activate', () => {
    if (BrowserWindow.getAllWindows().length === 0) {
        createWindow();
    }
});