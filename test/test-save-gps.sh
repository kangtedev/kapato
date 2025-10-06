#!/bin/bash

# Test script for save-gps Netlify function
# Usage: bash test-save-gps.sh

echo "=== Testing save-gps function ==="
echo ""

# Test 1: Valid request with device_id
echo "Test 1: Valid GPS data with device_id"
curl -X POST https://kapato.netlify.app/.netlify/functions/save-gps \
  -H "Content-Type: application/json" \
  -d '{
    "device_id": "ESP32-001",
    "latitude": 12.9716,
    "longitude": 77.5946
  }'
echo -e "\n"

# Test 2: Missing device_id
echo "Test 2: Missing device_id (should fail)"
curl -X POST https://kapato.netlify.app/.netlify/functions/save-gps \
  -H "Content-Type: application/json" \
  -d '{
    "latitude": 12.9716,
    "longitude": 77.5946
  }'
echo -e "\n"

# Test 3: Invalid device_id
echo "Test 3: Invalid device_id (should fail)"
curl -X POST https://kapato.netlify.app/.netlify/functions/save-gps \
  -H "Content-Type: application/json" \
  -d '{
    "device_id": "INVALID-DEVICE",
    "latitude": 12.9716,
    "longitude": 77.5946
  }'
echo -e "\n"

# Test 4: Missing latitude
echo "Test 4: Missing latitude (should fail)"
curl -X POST https://kapato.netlify.app/.netlify/functions/save-gps \
  -H "Content-Type: application/json" \
  -d '{
    "device_id": "ESP32-001",
    "longitude": 77.5946
  }'
echo -e "\n"

echo "=== Tests complete ==="
