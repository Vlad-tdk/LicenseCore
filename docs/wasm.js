// LicenseCore++ Demo - Hybrid JavaScript/WASM Implementation
class LicenseCoreDemo {
    constructor() {
        this.secretKey = "demo-secret-key-2024";
        this.useWasm = true;
        this.wasmModule = null;
        this.wasmManager = null;
        this.currentHwid = null;
        this.currentLicense = null;

        // Try to load WASM module
        this.initializeWasm();

        // Fallback to JavaScript implementation
        if (!this.useWasm) {
            this.currentHwid = this.generateMockHwid();
        }
    }

    async initializeWasm() {
        // Show loading status
        this.showLoadingStatus();
        
        try {
            // Check if WASM files exist
            const wasmResponse = await fetch('./license_core.wasm');
            const jsResponse = await fetch('./license_core.js');

            if (wasmResponse.ok && jsResponse.ok) {
                console.log('🔧 Loading WASM module...');

                // Load the WASM module correctly
                const script = document.createElement('script');
                script.src = './license_core.js';
                
                await new Promise((resolve, reject) => {
                    script.onload = resolve;
                    script.onerror = reject;
                    document.head.appendChild(script);
                });
                
                // Get the module from global scope
                if (typeof LicenseCoreModule !== 'undefined') {
                    this.wasmModule = await LicenseCoreModule();
                    this.wasmManager = new this.wasmModule.LicenseCoreWasm(this.secretKey);

                    this.useWasm = true;
                    this.currentHwid = this.wasmManager.getCurrentHwid();

                    console.log('✅ WASM module loaded successfully!');
                    console.log('🔐 Using real C++ implementation with HMAC-SHA256');
                    console.log('🎯 Hardware ID:', this.currentHwid);
                    console.log('🛡️ Cryptographic security: Web Crypto API + HMAC-SHA256');

                    // Update UI to show WASM is active
                    this.showWasmStatus(true);
                } else {
                    throw new Error('LicenseCoreModule not found in global scope');
                }

            } else {
                console.log('ℹ️ WASM files not found, using JavaScript fallback');
                this.useWasm = false;
                this.currentHwid = this.generateMockHwid();
                this.showWasmStatus(false);
            }
        } catch (error) {
            console.log('⚠️ WASM loading failed, using JavaScript fallback:', error.message);
            this.useWasm = false;
            this.currentHwid = this.generateMockHwid();
            this.showWasmStatus(false);
        }
    }

    showLoadingStatus() {
        const header = document.querySelector('.header');
        if (header) {
            const statusDiv = document.createElement('div');
            statusDiv.className = 'wasm-status loading';
            statusDiv.innerHTML = `
                <span class="wasm-indicator" style="animation: pulse-yellow 1s infinite;"></span>
                <strong>Loading WASM...</strong> - Checking for C++ module
            `;
            header.appendChild(statusDiv);
        }
    }

    showWasmStatus(isWasm) {
        // Add status indicator to the page
        const header = document.querySelector('.header');
        if (header) {
            // Remove existing status
            const existingStatus = header.querySelector('.wasm-status');
            if (existingStatus) {
                existingStatus.remove();
            }
            
            const statusDiv = document.createElement('div');
            statusDiv.className = `wasm-status ${isWasm ? '' : 'js-fallback'}`;
            
            const indicator = document.createElement('span');
            indicator.className = `wasm-indicator ${isWasm ? '' : 'js-mode'}`;
            
            statusDiv.innerHTML = `
                ${indicator.outerHTML}
                <strong>${isWasm ? 'Real HMAC-SHA256' : 'JavaScript Demo'}</strong> - 
                ${isWasm ? 'Web Crypto API + C++ WASM' : 'Simulation Mode'}
            `;
            
            header.appendChild(statusDiv);
            
            // Add performance info for WASM
            if (isWasm) {
                setTimeout(() => {
                    const perfInfo = document.createElement('div');
                    perfInfo.style.cssText = `
                        margin-top: 0.5rem;
                        font-size: 0.8rem;
                        opacity: 0.8;
                        color: #155724;
                    `;
                    perfInfo.innerHTML = `
                        ⚡ ${this.wasmModule ? 'Native speed' : 'Loading...'} • 
                        🔒 Hardware-bound • 
                        🎯 Zero JavaScript overhead
                    `;
                    statusDiv.appendChild(perfInfo);
                }, 100);
            }
        }
    }

    // WASM Implementation
    generateMockHwidWasm() {
        if (this.wasmManager) {
            this.wasmManager.generateNewHwid();
            return this.wasmManager.getCurrentHwid();
        }
        return this.generateMockHwid();
    }

    async generateLicenseWasm(userId, features, expiryDays) {
        if (!this.wasmManager) {
            throw new Error('WASM module not available');
        }

        // Convert JavaScript array to WASM vector
        const wasmFeatures = new this.wasmModule.VectorString();
        features.forEach(feature => wasmFeatures.push_back(feature));

        const license = this.wasmManager.generateLicense(userId, wasmFeatures, expiryDays);

        // Clean up WASM vector
        wasmFeatures.delete();

        return license;
    }

    async validateLicenseWasm(licenseJson) {
        if (!this.wasmManager) {
            throw new Error('WASM module not available');
        }

        const result = this.wasmManager.validateLicense(licenseJson);

        return {
            valid: result.valid,
            error: result.error_message,
            license: {
                user_id: result.user_id,
                features: result.features
            },
            message: result.valid ? "License is valid and properly signed!" : result.error_message
        };
    }

    // JavaScript Fallback Implementation (existing code)
    generateMockHwid() {
        const components = [
            this.hashString("CPU-" + navigator.userAgent),
            this.hashString("MAC-" + Math.random().toString(36)),
            this.hashString("VOL-" + Date.now().toString())
        ];
        return components.join("").substring(0, 32);
    }

    hashString(str) {
        let hash = 0;
        for (let i = 0; i < str.length; i++) {
            const char = str.charCodeAt(i);
            hash = ((hash << 5) - hash) + char;
            hash = hash & hash;
        }
        return Math.abs(hash).toString(16);
    }

    async hmacSha256(data, key) {
        const encoder = new TextEncoder();
        const keyData = encoder.encode(key);
        const dataBuffer = encoder.encode(data);

        const cryptoKey = await crypto.subtle.importKey(
            'raw',
            keyData,
            { name: 'HMAC', hash: 'SHA-256' },
            false,
            ['sign']
        );

        const signature = await crypto.subtle.sign('HMAC', cryptoKey, dataBuffer);
        return Array.from(new Uint8Array(signature))
            .map(b => b.toString(16).padStart(2, '0'))
            .join('');
    }

    formatDate(date) {
        return date.toISOString().replace(/\.\d{3}Z$/, 'Z');
    }

    async generateLicenseJS(userId, features, expiryDays) {
        const now = new Date();
        const expiry = expiryDays === -1 ?
            new Date('2099-12-31T23:59:59Z') :
            new Date(now.getTime() + expiryDays * 24 * 60 * 60 * 1000);

        const licenseData = {
            user_id: userId,
            license_id: `lic-${Date.now()}`,
            expiry: this.formatDate(expiry),
            issued_at: this.formatDate(now),
            hardware_hash: this.currentHwid,
            features: features,
            version: 1
        };

        const dataToSign = JSON.stringify(licenseData, null, 2);
        const signature = await this.hmacSha256(dataToSign, this.secretKey);

        licenseData.hmac_signature = signature;

        this.currentLicense = licenseData;
        return JSON.stringify(licenseData, null, 2);
    }

    async validateLicenseJS(licenseJson) {
        try {
            const license = JSON.parse(licenseJson);

            const required = ['user_id', 'license_id', 'expiry', 'hardware_hash', 'features', 'hmac_signature'];
            for (const field of required) {
                if (!license[field]) {
                    return { valid: false, error: `Missing required field: ${field}` };
                }
            }

            const signature = license.hmac_signature;
            const dataWithoutSignature = { ...license };
            delete dataWithoutSignature.hmac_signature;

            const dataToVerify = JSON.stringify(dataWithoutSignature, null, 2);
            const computedSignature = await this.hmacSha256(dataToVerify, this.secretKey);

            if (signature !== computedSignature) {
                return { valid: false, error: "Invalid license signature" };
            }

            const now = new Date();
            const expiry = new Date(license.expiry);
            if (now > expiry) {
                return { valid: false, error: "License has expired" };
            }

            if (license.hardware_hash !== this.currentHwid) {
                return { valid: false, error: "Hardware fingerprint mismatch" };
            }

            return {
                valid: true,
                license: license,
                message: "License is valid and properly signed!"
            };

        } catch (error) {
            return { valid: false, error: `JSON parsing error: ${error.message}` };
        }
    }

    // Unified API (automatically chooses WASM or JS)
    generateNewHwid() {
        if (this.useWasm) {
            this.currentHwid = this.generateMockHwidWasm();
        } else {
            this.currentHwid = this.generateMockHwid();
        }
        return this.currentHwid;
    }

    async generateLicense(userId, features, expiryDays) {
        if (this.useWasm) {
            return await this.generateLicenseWasm(userId, features, expiryDays);
        } else {
            return await this.generateLicenseJS(userId, features, expiryDays);
        }
    }

    async validateLicense(licenseJson) {
        if (this.useWasm) {
            return await this.validateLicenseWasm(licenseJson);
        } else {
            return await this.validateLicenseJS(licenseJson);
        }
    }

    hasFeature(feature) {
        if (!this.currentLicense) return false;
        if (this.useWasm) {
            // For WASM, we need to check the last validation result
            return this.currentLicense.features && this.currentLicense.features.includes(feature);
        } else {
            return this.currentLicense.features.includes(feature);
        }
    }

    // Performance comparison
    async performanceTest() {
        if (!this.useWasm) {
            console.log('⚠️ WASM not available for performance testing');
            return;
        }

        console.log('🏁 Performance Test: WASM vs JavaScript');

        const iterations = 100;
        const testFeatures = ['basic', 'premium', 'api'];

        // Test JavaScript implementation
        console.time('JavaScript Implementation');
        for (let i = 0; i < iterations; i++) {
            await this.generateLicenseJS(`user-${i}`, testFeatures, 365);
        }
        console.timeEnd('JavaScript Implementation');

        // Test WASM implementation
        console.time('WASM Implementation');
        for (let i = 0; i < iterations; i++) {
            await this.generateLicenseWasm(`user-${i}`, testFeatures, 365);
        }
        console.timeEnd('WASM Implementation');
    }
}

// Initialize demo with hybrid support
const licenseCore = new LicenseCoreDemo();

// UI Functions (same as before)
function updateHwidDisplay() {
    document.getElementById('currentHwid').textContent = licenseCore.currentHwid || 'Loading...';
}

function generateNewHwid() {
    licenseCore.generateNewHwid();
    updateHwidDisplay();
    showStatus('info', licenseCore.useWasm ?
        '🔧 New hardware ID generated (WASM)' :
        '🔄 New hardware ID generated (JavaScript)');
}

function getSelectedFeatures() {
    const selected = document.querySelectorAll('.feature-option.selected');
    return Array.from(selected).map(el => el.dataset.feature);
}

async function generateLicense() {
    const userId = document.getElementById('userId').value || 'demo-user';
    const expiryDays = parseInt(document.getElementById('expiryDays').value);
    const features = getSelectedFeatures();

    if (features.length === 0) {
        showStatus('error', '❌ Please select at least one feature');
        return;
    }

    try {
        showStatus('info', licenseCore.useWasm ?
            '🔧 Generating license with WASM...' :
            '🔄 Generating license with JavaScript...');

        const license = await licenseCore.generateLicense(userId, features, expiryDays);

        document.getElementById('licenseOutput').textContent = license;
        document.getElementById('copyBtn').disabled = false;
        document.getElementById('validateBtn').disabled = false;

        showStatus('success', licenseCore.useWasm ?
            '✅ License generated with C++ WASM!' :
            '✅ License generated with JavaScript!');
    } catch (error) {
        showStatus('error', `❌ Error generating license: ${error.message}`);
    }
}

function copyLicense() {
    const licenseText = document.getElementById('licenseOutput').textContent;
    navigator.clipboard.writeText(licenseText).then(() => {
        showStatus('success', '📋 License copied to clipboard!');
    });
}

async function validateLicense() {
    const licenseText = document.getElementById('licenseOutput').textContent;

    if (!licenseText || licenseText.includes('Click "Generate License"')) {
        showStatus('error', '❌ No license to validate');
        return;
    }

    try {
        showStatus('info', licenseCore.useWasm ?
            '🔧 Validating with WASM...' :
            '🔄 Validating with JavaScript...');

        const result = await licenseCore.validateLicense(licenseText);

        const resultDiv = document.getElementById('validationResult');
        const implementation = licenseCore.useWasm ? 'C++ WASM' : 'JavaScript';

        if (result.valid) {
            resultDiv.innerHTML = `
                <div class="status success">
                    ✅ ${result.message} (${implementation})
                    <br><small>User: ${result.license.user_id} | Features: ${Array.isArray(result.license.features) ? result.license.features.join(', ') : 'N/A'}</small>
                </div>
            `;
        } else {
            resultDiv.innerHTML = `
                <div class="status error">
                    ❌ License validation failed (${implementation}): ${result.error}
                </div>
            `;
        }
    } catch (error) {
        showStatus('error', `❌ Validation error: ${error.message}`);
    }
}

function showStatus(type, message) {
    const resultDiv = document.getElementById('validationResult');
    resultDiv.innerHTML = `<div class="status ${type}">${message}</div>`;

    if (type !== 'error') {
        setTimeout(() => {
            if (resultDiv.innerHTML.includes(message)) {
                resultDiv.innerHTML = '';
            }
        }, 3000);
    }
}

// Performance test function
async function runPerformanceTest() {
    if (licenseCore.useWasm) {
        showStatus('info', '🏁 Running performance test...');
        await licenseCore.performanceTest();
        showStatus('success', '✅ Performance test completed - check console for results');
    } else {
        showStatus('info', 'ℹ️ Performance test requires WASM module');
    }
}

// Advanced demo features
function simulateHardwareChange() {
    generateNewHwid();
    setTimeout(() => {
        showStatus('info', '💡 Try validating the license now - it should fail due to hardware mismatch!');
    }, 1000);
}

function simulateExpiredLicense() {
    const licenseText = document.getElementById('licenseOutput').textContent;
    if (licenseText && !licenseText.includes('Click "Generate License"')) {
        try {
            const license = JSON.parse(licenseText);
            license.expiry = new Date(Date.now() - 24 * 60 * 60 * 1000).toISOString();

            // Re-sign with appropriate method
            if (licenseCore.useWasm) {
                // For WASM, just modify and show - signature will be invalid
                document.getElementById('licenseOutput').textContent = JSON.stringify(license, null, 2);
                showStatus('info', '⏰ License set to expired (WASM) - try validating now!');
            } else {
                // For JS, re-sign properly
                const dataWithoutSignature = { ...license };
                delete dataWithoutSignature.hmac_signature;

                licenseCore.hmacSha256(JSON.stringify(dataWithoutSignature, null, 2), licenseCore.secretKey)
                    .then(signature => {
                        license.hmac_signature = signature;
                        document.getElementById('licenseOutput').textContent = JSON.stringify(license, null, 2);
                        showStatus('info', '⏰ License set to expired (JavaScript) - try validating now!');
                    });
            }
        } catch (error) {
            showStatus('error', 'Could not simulate expired license');
        }
    }
}

function tamperLicense() {
    const licenseText = document.getElementById('licenseOutput').textContent;
    if (licenseText && !licenseText.includes('Click "Generate License"')) {
        try {
            const license = JSON.parse(licenseText);
            license.features.push('enterprise');
            license.user_id = 'hacker';

            document.getElementById('licenseOutput').textContent = JSON.stringify(license, null, 2);
            showStatus('info', '🕵️ License tampered with - try validating to see security in action!');
        } catch (error) {
            showStatus('error', 'Could not tamper with license');
        }
    }
}

// Event listeners
document.addEventListener('DOMContentLoaded', function () {
    // Wait for potential WASM loading
    setTimeout(() => {
        updateHwidDisplay();
    }, 100);

    // Feature selection
    document.querySelectorAll('.feature-option').forEach(option => {
        option.addEventListener('click', function () {
            this.classList.toggle('selected');
        });
    });

    // Add demo scenario buttons
    const demoScenarios = document.createElement('div');
    demoScenarios.className = 'demo-scenarios';
    demoScenarios.innerHTML = `
        <h3>🎭 Demo Scenarios</h3>
        <div class="scenario-buttons">
            <button class="btn-scenario" onclick="simulateHardwareChange()">🖥️ Hardware Change</button>
            <button class="btn-scenario" onclick="simulateExpiredLicense()">⏰ Expired License</button>
            <button class="btn-scenario" onclick="tamperLicense()">🕵️ Tamper License</button>
        </div>
    `;

    const outputSection = document.querySelector('.output-section');
    if (outputSection) {
        outputSection.appendChild(demoScenarios);
    }

    // Add performance test button if WASM is available
    setTimeout(() => {
        if (licenseCore.useWasm) {
            const scenarioButtons = document.querySelector('.scenario-buttons');
            if (scenarioButtons) {
                const perfButton = document.createElement('button');
                perfButton.className = 'btn-scenario';
                perfButton.textContent = '🏁 Performance Test';
                perfButton.onclick = runPerformanceTest;
                scenarioButtons.appendChild(perfButton);
            }
        }
    }, 500);
});

// Add CSS for scenario buttons
const scenarioStyles = document.createElement('style');
scenarioStyles.textContent = `
    .demo-scenarios {
        margin-top: 2rem;
        padding-top: 2rem;
        border-top: 2px solid #e1e5e9;
    }

    .demo-scenarios h3 {
        margin-bottom: 1rem;
        color: #333;
        font-size: 1.2rem;
    }

    .scenario-buttons {
        display: flex;
        gap: 0.5rem;
        flex-wrap: wrap;
    }

    .btn-scenario {
        background: #6c757d;
        color: white;
        border: none;
        padding: 0.5rem 1rem;
        border-radius: 5px;
        font-size: 0.9rem;
        cursor: pointer;
        transition: all 0.3s ease;
        flex: 1;
        min-width: 120px;
    }

    .btn-scenario:hover {
        background: #5a6268;
        transform: translateY(-1px);
    }
`;
document.head.appendChild(scenarioStyles);