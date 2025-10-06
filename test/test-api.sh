#!/bin/bash

# API Test Script for GPS Tracker
# Tests all three endpoints

echo "======================================="
echo "GPS Tracker API Test Suite"
echo "======================================="
echo ""

# Test 1: Get device info
echo "Test 1: Get device info for ESP32-001"
echo "GET /.netlify/functions/get-device-info?device_id=ESP32-001"
curl -s "https://kapato.netlify.app/.netlify/functions/get-device-info?device_id=ESP32-001" | jq '.'
echo ""
echo "---"
echo ""

# Test 2: Save GPS data
echo "Test 2: Save GPS data for ESP32-001"
echo "POST /.netlify/functions/save-gps"
curl -s -X POST https://kapato.netlify.app/.netlify/functions/save-gps \
  -H "Content-Type: application/json" \
  -d '{
    "device_id": "ESP32-001",
    "latitude": 12.9716,
    "longitude": 77.5946
  }' | jq '.'
echo ""
echo "---"
echo ""

# Test 3: Get latest GPS (all devices)
echo "Test 3: Get latest GPS location (all devices)"
echo "GET /.netlify/functions/get-gps"
curl -s "https://kapato.netlify.app/.netlify/functions/get-gps" | jq '.'
echo ""
echo "---"
echo ""

# Test 4: Get latest GPS for specific device
echo "Test 4: Get latest GPS for ESP32-001"
echo "GET /.netlify/functions/get-gps?device_id=ESP32-001"
curl -s "https://kapato.netlify.app/.netlify/functions/get-gps?device_id=ESP32-001" | jq '.'
echo ""
echo "---"
echo ""

# Test 5: Invalid device
echo "Test 5: Get device info for invalid device (should fail)"
echo "GET /.netlify/functions/get-device-info?device_id=INVALID"
curl -s "https://kapato.netlify.app/.netlify/functions/get-device-info?device_id=INVALID" | jq '.'
echo ""
echo "---"
echo ""

echo "======================================="
echo "All tests complete!"
echo "======================================="
