"""
Solar Charging Station - Python API Bridge
Communicates between Blazor app and Arduino Mega
"""

from flask import Flask, request, jsonify
from flask_cors import CORS
import serial
import json
import time

app = Flask(__name__)
CORS(app)

# Configure serial connection to Arduino
# Update COM port for your setup (e.g., '/dev/ttyACM0' on Raspberry Pi)
# ARDUINO_PORT = 'COM3'  # Windows
ARDUINO_PORT = '/dev/ttyACM0'  # Raspberry Pi
BAUD_RATE = 9600

try:
    arduino = serial.Serial(ARDUINO_PORT, BAUD_RATE, timeout=1)
    print(f"\n{'='*60}")
    print(f"ARDUINO CONNECTION")
    print(f"{'='*60}")
    print(f"Connected to Arduino on {ARDUINO_PORT}")
    print(f"Waiting for Arduino to initialize...\n")
    
    # Read Arduino startup messages for 3 seconds
    startup_timeout = time.time() + 3
    while time.time() < startup_timeout:
        if arduino.in_waiting > 0:
            try:
                line = arduino.readline().decode('utf-8', errors='ignore').strip()
                if line:
                    # Try to parse as JSON for pretty printing
                    try:
                        data = json.loads(line)
                        if 'status' in data:
                            print(f"📟 Arduino: {data['status']}")
                        elif 'info' in data:
                            print(f"ℹ️  Info: {data['info']}")
                        elif 'help' in data:
                            print(f"💡 Help: {data['help']}")
                        else:
                            print(f"📟 Arduino: {line}")
                    except json.JSONDecodeError:
                        # Not JSON, just print it
                        print(f"📟 Arduino: {line}")
            except Exception as e:
                pass  # Ignore decoding errors
        time.sleep(0.1)
    
    print(f"{'='*60}")
    print(f"✓ Arduino initialization complete!")
    print(f"{'='*60}\n")
    
except Exception as e:
    print(f"\n{'='*60}")
    print(f"⚠️  WARNING: Could not connect to Arduino")
    print(f"{'='*60}")
    print(f"Error: {e}")
    print(f"Port: {ARDUINO_PORT}")
    print(f"\nThe API will run in SIMULATION mode.")
    print(f"All hardware commands will be simulated (not secure!).")
    print(f"{'='*60}\n")
    arduino = None

def send_arduino_command(command, data, timeout=10):
    """Send command to Arduino and get response"""
    if arduino is None:
        print(f"⚠ Simulating Arduino command: {command} with data: {data}")
        return {"success": True, "simulated": True}
    
    try:
        message = json.dumps({"command": command, "data": data})
        print(f"→ Sending to Arduino: {message}")
        arduino.write((message + '\n').encode())
        
        # For fingerprint enrollment, we need to wait longer and handle multiple responses
        if command == 'FINGERPRINT_ENROLL':
            return handle_enrollment_response(timeout)
        
        # For fingerprint verification, we need to wait for sensor scan
        if command == 'FINGERPRINT_VERIFY':
            return handle_verification_response(timeout)
        
        # Standard response handling
        time.sleep(0.15)  # Slightly longer delay for reliability
        
        if arduino.in_waiting > 0:
            response = arduino.readline().decode().strip()
            print(f"← Received from Arduino: {response}")
            
            try:
                return json.loads(response)
            except json.JSONDecodeError:
                print(f"⚠ Non-JSON response: {response}")
                return {"success": True}
        
        return {"success": True}
    except Exception as e:
        print(f"❌ Arduino communication error: {e}")
        return {"success": False, "error": str(e)}

def handle_enrollment_response(timeout=10):
    """
    Handle multi-step enrollment response from Arduino
    AS608 enrollment sends multiple status updates
    """
    start_time = time.time()
    final_result = None
    
    print("\n--- AS608 Enrollment Process ---")
    
    while (time.time() - start_time) < timeout:
        if arduino.in_waiting > 0:
            try:
                response = arduino.readline().decode().strip()
                print(f"← {response}")
                
                result = json.loads(response)
                
                # Status updates during enrollment
                if 'status' in result:
                    print(f"   Status: {result['status']}")
                
                # Final result
                if 'success' in result:
                    final_result = result
                    
                    # If success is True and we have a message or fingerprintId, enrollment is complete
                    if result.get('success') and ('message' in result or 'fingerprintId' in result):
                        print("--- Enrollment Complete ---\n")
                        return final_result
                    elif not result.get('success'):
                        # Enrollment failed
                        print("--- Enrollment Failed ---\n")
                        return final_result
                        
            except json.JSONDecodeError:
                # Non-JSON response, might be debug output
                continue
        
        time.sleep(0.1)
    
    print("--- Enrollment Timeout ---\n")
    return final_result if final_result else {"success": False, "error": "Timeout"}

def handle_verification_response(timeout=10):
    """
    Handle fingerprint verification response from Arduino
    AS608 verification may send status updates before final result
    """
    start_time = time.time()
    final_result = None
    
    print("\n--- AS608 Verification Process ---")
    
    while (time.time() - start_time) < timeout:
        if arduino.in_waiting > 0:
            try:
                response = arduino.readline().decode().strip()
                print(f"← {response}")
                
                result = json.loads(response)
                
                # Status updates during verification
                if 'status' in result:
                    print(f"   Status: {result['status']}")
                
                # Final result
                if 'success' in result or 'isValid' in result:
                    final_result = result
                    print("--- Verification Complete ---\n")
                    return final_result
                        
            except json.JSONDecodeError:
                # Non-JSON response, might be debug output
                continue
        
        time.sleep(0.1)
    
    print("--- Verification Timeout ---\n")
    return final_result if final_result else {"success": True, "isValid": False, "error": "Timeout"}

@app.route('/api/relay', methods=['POST'])
def control_relay():
    """Control relay for slot power"""
    data = request.json
    slot_number = data.get('slotNumber')
    state = data.get('state')
    
    print(f"Relay control - Slot {slot_number}: {'ON' if state else 'OFF'}")
    
    result = send_arduino_command('RELAY', {
        'slot': slot_number,
        'state': state
    })
    
    return jsonify(result), 200 if result.get('success') else 500

@app.route('/api/solenoid', methods=['POST'])
def control_solenoid():
    """Control solenoid lock"""
    data = request.json
    slot_number = data.get('slotNumber')
    lock_state = data.get('locked')
    duration = data.get('duration', 0)  # Duration in seconds, default 0 (permanent)
    
    if duration > 0:
        print(f"Solenoid control - Slot {slot_number}: {'LOCK' if lock_state else 'UNLOCK'} for {duration} seconds")
    else:
        print(f"Solenoid control - Slot {slot_number}: {'LOCK' if lock_state else 'UNLOCK'}")
    
    result = send_arduino_command('SOLENOID', {
        'slot': slot_number,
        'lock': lock_state,
        'duration': duration
    })
    
    return jsonify(result), 200 if result.get('success') else 500

@app.route('/api/uv-light', methods=['POST'])
def control_uv_light():
    """Control UV light for phone sanitization"""
    data = request.json
    slot_number = data.get('slotNumber')
    state = data.get('state')
    
    print(f"UV Light control - Slot {slot_number}: {'ON' if state else 'OFF'}")
    
    result = send_arduino_command('UV_LIGHT', {
        'slot': slot_number,
        'state': state
    })
    
    return jsonify(result), 200 if result.get('success') else 500

@app.route('/api/fingerprint/verify', methods=['POST'])
def verify_fingerprint():
    """
    Verify fingerprint against AS608 database
    Returns: { "isValid": true/false, "fingerprintId": matched_id, "confidence": score }
    """
    data = request.json
    expected_id = data.get('fingerprintId')
    
    print(f"\n{'='*50}")
    print(f"FINGERPRINT VERIFICATION REQUEST")
    print(f"{'='*50}")
    print(f"Expected Fingerprint ID: {expected_id}")
    print(f"Request from: {request.remote_addr}")
    print(f"Timestamp: {time.strftime('%Y-%m-%d %H:%M:%S')}")
    print("Waiting for finger scan on AS608 sensor...")
    print(f"{'='*50}\n")
    
    result = send_arduino_command('FINGERPRINT_VERIFY', {
        'id': expected_id
    }, timeout=10)
    
    print(f"\n{'='*50}")
    print(f"ARDUINO RESPONSE")
    print(f"{'='*50}")
    print(f"Raw result: {result}")
    print(f"{'='*50}\n")
    
    # Simulate successful verification for demo (when no Arduino connected)
    if result.get('simulated'):
        print("⚠ WARNING: Running in SIMULATION mode (no Arduino)")
        print("⚠ Returning simulated success for testing")
        result['isValid'] = True
        result['fingerprintId'] = expected_id
        result['confidence'] = 95
    
    is_valid = result.get('isValid', False)
    matched_id = result.get('fingerprintId', 0)
    confidence = result.get('confidence', 0)
    error_msg = result.get('error', '')
    
    print(f"\n{'='*50}")
    print(f"VERIFICATION RESULT")
    print(f"{'='*50}")
    
    if is_valid:
        print(f"✓ SUCCESS: Fingerprint matched!")
        print(f"  Matched ID: {matched_id}")
        print(f"  Expected ID: {expected_id}")
        print(f"  Confidence: {confidence}")
        print(f"  Match: {'CORRECT' if matched_id == expected_id else 'WRONG FINGER'}")
    else:
        print(f"✗ FAILED: Fingerprint not matched")
        print(f"  Expected ID: {expected_id}")
        print(f"  Error: {error_msg or 'No match found'}")
        if 'matchedId' in result:
            print(f"  Wrong finger detected - Matched ID: {result['matchedId']}")
    
    print(f"{'='*50}\n")
    
    return jsonify({
        'isValid': is_valid,
        'fingerprintId': matched_id,
        'confidence': confidence,
        'error': error_msg
    }), 200

@app.route('/api/coin-slot', methods=['GET'])
def get_coin_value():
    """
    Get coin slot value - called by UI every 2 seconds for real-time detection
    Returns: { "value": coin_amount, "timestamp": detection_time }
    """
    result = send_arduino_command('READ_COIN', {}, timeout=2)
    
    coin_value = result.get('value', 0)
    timestamp = result.get('timestamp', 0)
    
    # Log only when coin is detected (avoid spam)
    if coin_value > 0:
        print(f"💰 Coin detected: ₱{coin_value:.2f} (Timestamp: {timestamp})")
    
    # Simulate coin value for demo/testing (when no Arduino)
    if result.get('simulated'):
        # Return 0 normally (no simulation) - user can use "Simulate" button in UI
        coin_value = 0
    
    return jsonify({
        'value': coin_value,
        'timestamp': timestamp
    }), 200

@app.route('/api/solenoid/unlock-temp', methods=['POST'])
def unlock_temp():
    """
    Temporarily unlock solenoid for 2 seconds (for device access during charging)
    Sends pulse to unlock, waits 2 seconds, then automatically re-locks
    """
    data = request.json
    slot_number = data.get('slotNumber')
    
    print(f"Temporary unlock - Slot {slot_number}")
    
    result = send_arduino_command('UNLOCK_TEMP', {
        'slot': slot_number
    })
    
    return jsonify(result), 200 if result.get('success') else 500

@app.route('/api/fingerprint/enroll', methods=['POST'])
def enroll_fingerprint():
    """
    Enroll new fingerprint on AS608 sensor
    Multi-step process:
    1. Place finger (first scan)
    2. Remove finger
    3. Place same finger (second scan)
    4. Create and store template
    """
    data = request.json
    fingerprint_id = data.get('userId', data.get('fingerprintId', 1))
    
    print(f"\n{'='*50}")
    print(f"FINGERPRINT ENROLLMENT REQUEST")
    print(f"{'='*50}")
    print(f"Fingerprint ID: {fingerprint_id}")
    print(f"Request from: {request.remote_addr}")
    print(f"Timestamp: {time.strftime('%Y-%m-%d %H:%M:%S')}")
    print("Starting AS608 enrollment process...")
    print(f"{'='*50}\n")
    
    result = send_arduino_command('FINGERPRINT_ENROLL', {
        'userId': fingerprint_id
    }, timeout=30)  # Longer timeout for enrollment
    
    print(f"\n{'='*50}")
    print(f"ENROLLMENT RESULT")
    print(f"{'='*50}")
    print(f"Raw result: {result}")
    print(f"{'='*50}\n")
    
    if result.get('success'):
        print(f"✓ SUCCESS: Fingerprint {fingerprint_id} enrolled!")
        print(f"=== Enrollment Complete ===\n")
        return jsonify({
            'success': True,
            'fingerprintId': fingerprint_id,
            'message': 'Fingerprint enrolled successfully'
        }), 200
    else:
        error_msg = result.get('message', result.get('error', 'Unknown error'))
        hint = result.get('hint', '')
        print(f"✗ FAILED: Enrollment unsuccessful")
        print(f"  Error: {error_msg}")
        if hint:
            print(f"  Hint: {hint}")
        print(f"=== Enrollment Failed ===\n")
        
        response_data = {
            'success': False,
            'error': error_msg,
            'fingerprintId': fingerprint_id
        }
        if hint:
            response_data['hint'] = hint
        
        return jsonify(response_data), 500

@app.route('/health', methods=['GET'])
def health_check():
    """Health check endpoint"""
    return jsonify({
        'status': 'healthy',
        'arduino_connected': arduino is not None
    }), 200

@app.route('/api/fingerprint/delete-all', methods=['POST'])
def delete_all_fingerprints():
    """Delete all fingerprints manually"""
    print("\n⚠️ Attempting to delete all fingerprints...")
    
    # Delete fingerprints one by one (IDs 1-127)
    deleted_count = 0
    for fid in range(1, 128):
        result = send_arduino_command('FINGERPRINT_DELETE', {
            'fingerprintId': fid
        })
        if result.get('success'):
            deleted_count += 1
            print(f"✓ Deleted fingerprint ID {fid}")
    
    print(f"✓ Deleted {deleted_count} fingerprints total\n")
    
    return jsonify({
        'success': True,
        'deleted_count': deleted_count,
        'message': f'Deleted {deleted_count} fingerprints'
    }), 200

if __name__ == '__main__':
    print("\n" + "="*60)
    print("SOLAR CHARGING STATION - Python API")
    print("="*60)
    print(f"Port: 8000")
    print(f"Arduino: {'Connected ✓' if arduino else 'Simulated mode ⚠️'}")
    print("="*60 + "\n")
    
    if arduino:
        print("Ready to receive commands from Blazor app!")
        print("Watch this terminal for real-time Arduino communication.\n")
    else:
        print("⚠️  Running in SIMULATION mode!")
        print("Connect Arduino and restart for hardware control.\n")
    
    app.run(host='127.0.0.1', port=8000, debug=True)

