"""
Debug script to test fingerprint verification
Run this to check if enrollment and verification are working
"""

import requests
import time

API_BASE = "http://localhost:8000"

def test_health():
    """Check if API is running"""
    print("\n=== Testing API Health ===")
    try:
        response = requests.get(f"{API_BASE}/health")
        if response.status_code == 200:
            data = response.json()
            print(f"✓ API is healthy")
            print(f"  Arduino connected: {data.get('arduino_connected')}")
            return True
        else:
            print(f"✗ API returned status {response.status_code}")
            return False
    except Exception as e:
        print(f"✗ Cannot connect to API: {e}")
        print(f"  Make sure Python API is running on port 8000")
        return False

def test_enrollment(fingerprint_id):
    """Test fingerprint enrollment"""
    print(f"\n=== Testing Fingerprint Enrollment (ID: {fingerprint_id}) ===")
    print("Instructions:")
    print("  1. Place your finger on the AS608 sensor when prompted")
    print("  2. Remove your finger when prompted")
    print("  3. Place the SAME finger again when prompted")
    print("\nStarting enrollment in 3 seconds...")
    time.sleep(3)
    
    try:
        response = requests.post(
            f"{API_BASE}/api/fingerprint/enroll",
            json={"fingerprintId": fingerprint_id}
        )
        
        print(f"\nResponse Status: {response.status_code}")
        print(f"Response Body: {response.text}")
        
        if response.status_code == 200:
            data = response.json()
            if data.get('success'):
                print(f"✓ Enrollment successful!")
                print(f"  Fingerprint ID: {data.get('fingerprintId')}")
                return True
            else:
                print(f"✗ Enrollment failed: {data.get('error')}")
                return False
        else:
            print(f"✗ Server error: {response.text}")
            return False
            
    except Exception as e:
        print(f"✗ Request failed: {e}")
        return False

def test_verification(fingerprint_id):
    """Test fingerprint verification"""
    print(f"\n=== Testing Fingerprint Verification (ID: {fingerprint_id}) ===")
    print("Instructions:")
    print("  Place the enrolled finger on the AS608 sensor when prompted")
    print("\nStarting verification in 3 seconds...")
    time.sleep(3)
    
    try:
        response = requests.post(
            f"{API_BASE}/api/fingerprint/verify",
            json={"fingerprintId": fingerprint_id}
        )
        
        print(f"\nResponse Status: {response.status_code}")
        print(f"Response Body: {response.text}")
        
        if response.status_code == 200:
            data = response.json()
            is_valid = data.get('isValid', False)
            
            if is_valid:
                print(f"✓ Verification successful!")
                print(f"  Matched ID: {data.get('fingerprintId')}")
                print(f"  Confidence: {data.get('confidence')}")
                return True
            else:
                print(f"✗ Verification failed")
                print(f"  Error: {data.get('error', 'Unknown')}")
                if 'matchedId' in data:
                    print(f"  Wrong finger detected - Matched ID: {data['matchedId']}")
                    print(f"  Expected ID: {data['expectedId']}")
                return False
        else:
            print(f"✗ Server error: {response.text}")
            return False
            
    except Exception as e:
        print(f"✗ Request failed: {e}")
        return False

def main():
    print("=" * 60)
    print("FINGERPRINT VERIFICATION DEBUG TOOL")
    print("=" * 60)
    
    # Test 1: Check API health
    if not test_health():
        print("\n❌ Cannot proceed - API is not running")
        print("Start the API with: python app.py")
        return
    
    # Get fingerprint ID from user
    print("\n" + "=" * 60)
    fingerprint_id = input("Enter a fingerprint ID to test (1-127): ").strip()
    
    try:
        fingerprint_id = int(fingerprint_id)
        if fingerprint_id < 1 or fingerprint_id > 127:
            print("Invalid ID. Must be between 1 and 127.")
            return
    except ValueError:
        print("Invalid input. Must be a number.")
        return
    
    # Test 2: Enroll fingerprint
    print("\n" + "=" * 60)
    enroll_choice = input("Do you want to enroll a NEW fingerprint? (y/n): ").strip().lower()
    
    if enroll_choice == 'y':
        enrolled = test_enrollment(fingerprint_id)
        if not enrolled:
            print("\n❌ Enrollment failed. Cannot proceed to verification.")
            return
        
        print("\nWaiting 2 seconds before verification test...")
        time.sleep(2)
    
    # Test 3: Verify fingerprint
    print("\n" + "=" * 60)
    verified = test_verification(fingerprint_id)
    
    # Final summary
    print("\n" + "=" * 60)
    print("TEST SUMMARY")
    print("=" * 60)
    
    if verified:
        print("✓ All tests passed!")
        print("  Fingerprint system is working correctly.")
    else:
        print("✗ Verification failed!")
        print("\nTroubleshooting steps:")
        print("  1. Check if AS608 sensor is properly connected to Arduino")
        print("  2. Check if Arduino is connected to Raspberry Pi/PC")
        print("  3. Verify the fingerprint was enrolled successfully")
        print("  4. Try enrolling the fingerprint again")
        print("  5. Check Python API logs for detailed error messages")

if __name__ == "__main__":
    try:
        main()
    except KeyboardInterrupt:
        print("\n\nTest interrupted by user.")

