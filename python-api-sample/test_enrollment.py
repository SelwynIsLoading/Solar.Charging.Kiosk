#!/usr/bin/env python3
"""
Quick test script for AS608 fingerprint enrollment
Tests the enrollment process step by step
"""

import requests
import json
import time
import sys

BASE_URL = "http://localhost:5000"

def enroll_fingerprint(fingerprint_id):
    """
    Test fingerprint enrollment with detailed output
    """
    print(f"""
╔═══════════════════════════════════════════════════╗
║   AS608 Fingerprint Enrollment Test              ║
║   Fingerprint ID: {fingerprint_id:3d}                            ║
╚═══════════════════════════════════════════════════╝

Instructions:
1. Make sure AS608 sensor is connected to Arduino
2. Arduino should be connected via USB
3. Python API should be running (python app.py)

Starting enrollment in 3 seconds...
""")
    
    time.sleep(3)
    
    try:
        print("📡 Sending enrollment request to API...")
        response = requests.post(
            f"{BASE_URL}/api/fingerprint/enroll",
            json={'fingerprintId': fingerprint_id},
            timeout=30
        )
        
        print(f"\n📨 Response Status: {response.status_code}")
        result = response.json()
        print(f"📨 Response Data:")
        print(json.dumps(result, indent=2))
        
        if result.get('success'):
            print(f"""
╔═══════════════════════════════════════════════════╗
║   ✓ ENROLLMENT SUCCESSFUL!                       ║
║                                                   ║
║   Fingerprint ID: {result.get('fingerprintId', fingerprint_id):3d}                           ║
║   Status: Ready to use                           ║
╚═══════════════════════════════════════════════════╝

Next steps:
1. Test verification with: python test_verification.py {fingerprint_id}
2. Or run verification test from menu
""")
            return True
        else:
            print(f"""
╔═══════════════════════════════════════════════════╗
║   ✗ ENROLLMENT FAILED                            ║
║                                                   ║
║   Error: {result.get('error', 'Unknown error'):<42s}║
╚═══════════════════════════════════════════════════╝

Troubleshooting:
- Check Arduino connection
- Verify AS608 sensor is connected properly
- Clean sensor window
- Ensure finger is dry and clean
""")
            return False
            
    except requests.exceptions.ConnectionError:
        print(f"""
❌ ERROR: Cannot connect to API at {BASE_URL}

Make sure:
1. Python API is running: python app.py
2. Port 5000 is not blocked
3. Check firewall settings
""")
        return False
    except requests.exceptions.Timeout:
        print("""
⏱️ TIMEOUT: Enrollment took too long

Possible issues:
- AS608 sensor not responding
- Arduino not connected
- Finger not placed on sensor
""")
        return False
    except Exception as e:
        print(f"\n❌ Unexpected error: {e}")
        return False

def verify_fingerprint(fingerprint_id):
    """
    Test fingerprint verification
    """
    print(f"""
╔═══════════════════════════════════════════════════╗
║   AS608 Fingerprint Verification Test           ║
║   Expected ID: {fingerprint_id:3d}                            ║
╚═══════════════════════════════════════════════════╝

Place your finger on the sensor...
""")
    
    try:
        response = requests.post(
            f"{BASE_URL}/api/fingerprint/verify",
            json={'fingerprintId': fingerprint_id},
            timeout=10
        )
        
        result = response.json()
        print(f"\n📨 Response:")
        print(json.dumps(result, indent=2))
        
        if result.get('isValid'):
            print(f"""
╔═══════════════════════════════════════════════════╗
║   ✓ FINGERPRINT VERIFIED!                        ║
║                                                   ║
║   Matched ID: {result.get('fingerprintId', 0):3d}                             ║
║   Confidence: {result.get('confidence', 0):3d}%                              ║
╚═══════════════════════════════════════════════════╝
""")
        else:
            print(f"""
╔═══════════════════════════════════════════════════╗
║   ✗ VERIFICATION FAILED                          ║
║                                                   ║
║   Error: {result.get('error', 'No match found'):<42s}║
╚═══════════════════════════════════════════════════╝
""")
            
    except Exception as e:
        print(f"\n❌ Error: {e}")

if __name__ == '__main__':
    print("""
╔═══════════════════════════════════════════════════╗
║   Solar Charging Station                         ║
║   AS608 Fingerprint Test Utility                 ║
╚═══════════════════════════════════════════════════╝
""")
    
    # Get fingerprint ID from command line or prompt
    if len(sys.argv) > 1:
        try:
            fp_id = int(sys.argv[1])
        except ValueError:
            print("Invalid fingerprint ID. Using default: 1")
            fp_id = 1
    else:
        fp_id_input = input("Enter fingerprint ID to enroll (1-300) [default: 1]: ")
        fp_id = int(fp_id_input) if fp_id_input.strip() else 1
    
    print("\nWhat would you like to test?")
    print("1. Enroll Fingerprint")
    print("2. Verify Fingerprint")
    print("3. Both (Enroll then Verify)")
    
    choice = input("\nChoice (1-3): ")
    
    if choice == '1':
        enroll_fingerprint(fp_id)
    elif choice == '2':
        verify_fingerprint(fp_id)
    elif choice == '3':
        if enroll_fingerprint(fp_id):
            print("\n⏳ Waiting 2 seconds before verification...\n")
            time.sleep(2)
            verify_fingerprint(fp_id)
    else:
        print("Invalid choice!")

