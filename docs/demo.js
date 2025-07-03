// Mock LicenseCore++ Implementation for Demo
class LicenseCoreDemo {
    constructor() {
        this.secretKey = "demo-secret-key-2024";
        this.currentHwid = this.generateMockHwid();
        this.currentLicense = null;
    }

    generateMockHwid() {
        // Generate a realistic-looking hardware ID
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
            hash = hash & hash; // Convert to 32-bit integer
        }
        return Math.abs(hash).toString(16);
    }

    async hmacSha256(data, key) {
        // Mock HMAC-SHA256 implementation for demo
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

    async generateLicense(userId, features, expiryDays) {
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

        // Create JSON without signature for signing
        const dataToSign = JSON.stringify(licenseData, null, 2);
        const signature = await this.hmacSha256(dataToSign, this.secretKey);
        
        licenseData.hmac_signature = signature;
        
        this.currentLicense = licenseData;
        return JSON.stringify(licenseData, null, 2);
    }

    async validateLicense(licenseJson) {
        try {
            const license = JSON.parse(licenseJson);
            
            // Check required fields
            const required = ['user_id', 'license_id', 'expiry', 'hardware_hash', 'features', 'hmac_signature'];
            for (const field of required) {
                if (!license[field]) {
                    return { valid: false, error: `Missing required field: ${field}` };
                }
            }

            // Verify signature
            const signature = license.hmac_signature;
            const dataWithoutSignature = { ...license };
            delete dataWithoutSignature.hmac_signature;
            
            const dataToVerify = JSON.stringify(dataWithoutSignature, null, 2);
            const computedSignature = await this.hmacSha256(dataToVerify, this.secretKey);
            
            if (signature !== computedSignature) {
                return { valid: false, error: "Invalid license signature" };
            }

            // Check expiry
            const now = new Date();
            const expiry = new Date(license.expiry);
            if (now > expiry) {
                return { valid: false, error: "License has expired" };
            }

            // Check hardware fingerprint
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

    hasFeature(feature) {
        if (!this.currentLicense) return false;
        return this.currentLicense.features.includes(feature);
    }
}

// Initialize demo
const licenseCore = new LicenseCoreDemo();

// UI Functions
function updateHwidDisplay() {
    document.getElementById('currentHwid').textContent = licenseCore.currentHwid;
}

function generateNewHwid() {
    licenseCore.currentHwid = licenseCore.generateMockHwid();
    updateHwidDisplay();
    showStatus('info', '🔄 New hardware ID generated');
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
        showStatus('info', '🔄 Generating license...');
        const license = await licenseCore.generateLicense(userId, features, expiryDays);
        
        document.getElementById('licenseOutput').textContent = license;
        document.getElementById('copyBtn').disabled = false;
        document.getElementById('validateBtn').disabled = false;
        
        showStatus('success', '✅ License generated successfully!');
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
        showStatus('info', '🔄 Validating license...');
        const result = await licenseCore.validateLicense(licenseText);
        
        const resultDiv = document.getElementById('validationResult');
        
        if (result.valid) {
            resultDiv.innerHTML = `
                <div class="status success">
                    ✅ ${result.message}
                    <br><small>User: ${result.license.user_id} | Features: ${result.license.features.join(', ')}</small>
                </div>
            `;
        } else {
            resultDiv.innerHTML = `
                <div class="status error">
                    ❌ License validation failed: ${result.error}
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
    
    // Clear status after 3 seconds for non-error messages
    if (type !== 'error') {
        setTimeout(() => {
            if (resultDiv.innerHTML.includes(message)) {
                resultDiv.innerHTML = '';
            }
        }, 3000);
    }
}

// Advanced demo features
function simulateHardwareChange() {
    const originalHwid = licenseCore.currentHwid;
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
            
            const dataWithoutSignature = { ...license };
            delete dataWithoutSignature.hmac_signature;
            
            licenseCore.hmacSha256(JSON.stringify(dataWithoutSignature, null, 2), licenseCore.secretKey)
                .then(signature => {
                    license.hmac_signature = signature;
                    document.getElementById('licenseOutput').textContent = JSON.stringify(license, null, 2);
                    showStatus('info', '⏰ License set to expired - try validating now!');
                });
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
document.addEventListener('DOMContentLoaded', function() {
    updateHwidDisplay();
    
    // Feature selection
    document.querySelectorAll('.feature-option').forEach(option => {
        option.addEventListener('click', function() {
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
    outputSection.appendChild(demoScenarios);
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