"""
Test script for the Solar Charging Station Python API
Run this to test endpoints without the Blazor app
"""

import requests
import json
import time

BASE_URL = "http://localhost:5000"

def print_header(text):
    print("\n" + "="*50)
    print(f"  {text}")
    print("="*50)

def test_health():
    print_header("Testing Health Endpoint")
    response = requests.get(f"{BASE_URL}/health")
    print(f"Status: {response.status_code}")
    print(f"Response: {json.dumps(response.json(), indent=2)}\n")

def test_relay(slot_number, state):
    print_header(f"Testing Relay - Slot {slot_number}")
    response = requests.post(f"{BASE_URL}/api/relay", json={
        'slotNumber': slot_number,
        'state': state
    })
    print(f"Status: {response.status_code}")
    print(f"Action: Turn {'ON' if state else 'OFF'}")
    print(f"Response: {json.dumps(response.json(), indent=2)}\n")

def test_solenoid(slot_number, lock):
    print_header(f"Testing Solenoid - Slot {slot_number}")
    response = requests.post(f"{BASE_URL}/api/solenoid", json={
        'slotNumber': slot_number,
        'lock': lock
    })
    print(f"Status: {response.status_code}")
    print(f"Action: {'LOCK' if lock else 'UNLOCK'}")
    print(f"Response: {json.dumps(response.json(), indent=2)}\n")

def test_uv_light(slot_number, state):
    print_header(f"Testing UV Light - Slot {slot_number}")
    response = requests.post(f"{BASE_URL}/api/uv-light", json={
        'slotNumber': slot_number,
        'state': state
    })
    print(f"Status: {response.status_code}")
    print(f"Action: Turn {'ON' if state else 'OFF'}")
    print(f"Response: {json.dumps(response.json(), indent=2)}\n")

def test_fingerprint_enroll(fingerprint_id):
    print_header(f"Testing Fingerprint Enrollment - ID {fingerprint_id}")
    print("Follow the prompts from Arduino:")
    print("1. Place your finger on the sensor")
    print("2. Remove your finger")
    print("3. Place the same finger again")
    print("\nStarting enrollment...\n")
    
    response = requests.post(f"{BASE_URL}/api/fingerprint/enroll", json={
        'fingerprintId': fingerprint_id
    })
    print(f"Status: {response.status_code}")
    print(f"Response: {json.dumps(response.json(), indent=2)}\n")
    
    if response.status_code == 200:
        print("✓ Fingerprint enrolled successfully!")
    else:
        print("✗ Enrollment failed!")

def test_fingerprint_verify(fingerprint_id):
    print_header(f"Testing Fingerprint Verification - ID {fingerprint_id}")
    print("Place your enrolled finger on the sensor...\n")
    
    response = requests.post(f"{BASE_URL}/api/fingerprint/verify", json={
        'fingerprintId': fingerprint_id
    })
    print(f"Status: {response.status_code}")
    result = response.json()
    print(f"Response: {json.dumps(result, indent=2)}\n")
    
    if result.get('isValid'):
        print(f"✓ Fingerprint verified!")
        print(f"  Matched ID: {result.get('fingerprintId')}")
        print(f"  Confidence: {result.get('confidence')}%")
    else:
        print("✗ Fingerprint not recognized!")

def test_coin_slot():
    print_header("Testing Coin Slot Reader")
    response = requests.get(f"{BASE_URL}/api/coin-slot")
    print(f"Status: {response.status_code}")
    result = response.json()
    print(f"Response: {json.dumps(result, indent=2)}")
    print(f"Coin Value Detected: ₱{result.get('value', 0)}\n")

def test_complete_workflow():
    """Test complete charging workflow for Phone Slot 4"""
    print("\n" + "#"*50)
    print("  COMPLETE WORKFLOW TEST - PHONE SLOT 4")
    print("#"*50)
    
    slot = 4
    fingerprint_id = 40
    
    # Step 1: Enroll fingerprint
    print("\n[Step 1] Enrolling fingerprint...")
    test_fingerprint_enroll(fingerprint_id)
    time.sleep(2)
    
    # Step 2: Turn on relay
    print("\n[Step 2] Turning on power relay...")
    test_relay(slot, True)
    time.sleep(1)
    
    # Step 3: Turn on UV light
    print("\n[Step 3] Starting UV sanitization...")
    test_uv_light(slot, True)
    print("Waiting 15 seconds for UV sanitization...")
    time.sleep(15)
    test_uv_light(slot, False)
    
    # Step 4: Lock solenoid
    print("\n[Step 4] Locking slot...")
    test_solenoid(slot, True)
    time.sleep(1)
    
    # Step 5: Simulate charging time
    print("\n[Step 5] Charging in progress...")
    print("Simulating 10 seconds of charging...")
    time.sleep(10)
    
    # Step 6: Verify fingerprint to unlock
    print("\n[Step 6] Verifying fingerprint to unlock...")
    test_fingerprint_verify(fingerprint_id)
    time.sleep(1)
    
    # Step 7: Unlock and power off
    print("\n[Step 7] Unlocking slot...")
    test_solenoid(slot, False)
    time.sleep(1)
    
    print("\n[Step 8] Turning off power...")
    test_relay(slot, False)
    
    print("\n" + "#"*50)
    print("  WORKFLOW COMPLETE!")
    print("#"*50 + "\n")

def main():
    print("""
╔════════════════════════════════════════════════════╗
║   Solar Charging Station - API Test Suite        ║
╚════════════════════════════════════════════════════╝

Make sure the Python API is running:
  python app.py

Then choose a test:
""")
    
    print("1. Health Check")
    print("2. Test Relay (Slot 1)")
    print("3. Test Solenoid (Slot 4)")
    print("4. Test UV Light (Slot 4)")
    print("5. Test Fingerprint Enrollment (ID 1)")
    print("6. Test Fingerprint Verification (ID 1)")
    print("7. Test Coin Slot Reader")
    print("8. Complete Workflow Test (Phone Slot 4)")
    print("9. Run All Basic Tests")
    print("0. Exit")
    
    choice = input("\nEnter choice (0-9): ")
    
    try:
        if choice == '1':
            test_health()
        elif choice == '2':
            test_relay(1, True)
            time.sleep(2)
            test_relay(1, False)
        elif choice == '3':
            test_solenoid(4, True)
            time.sleep(2)
            test_solenoid(4, False)
        elif choice == '4':
            test_uv_light(4, True)
            time.sleep(2)
            test_uv_light(4, False)
        elif choice == '5':
            test_fingerprint_enroll(1)
        elif choice == '6':
            test_fingerprint_verify(1)
        elif choice == '7':
            test_coin_slot()
        elif choice == '8':
            test_complete_workflow()
        elif choice == '9':
            test_health()
            test_relay(1, True)
            time.sleep(1)
            test_relay(1, False)
            test_solenoid(4, True)
            time.sleep(1)
            test_solenoid(4, False)
            test_uv_light(4, True)
            time.sleep(1)
            test_uv_light(4, False)
            test_coin_slot()
        elif choice == '0':
            print("Exiting...")
            return
        else:
            print("Invalid choice!")
    except requests.exceptions.ConnectionError:
        print("\n❌ Error: Could not connect to API!")
        print("Make sure the Python API is running: python app.py")
    except Exception as e:
        print(f"\n❌ Error: {e}")

if __name__ == '__main__':
    while True:
        main()
        input("\nPress Enter to continue...")

