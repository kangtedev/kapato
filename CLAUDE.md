# Kapato Private Limited - Mithun Tracking Collar

## Overview
GPS tracking collar system for Mithun (semi-domesticated cattle) in Arunachal Pradesh. Helps farmers track and locate their Mithun herds in mountainous terrain.

**Company:** Kapato Private Limited
**Location:** Arunachal Pradesh, India
**Product:** GPS-enabled livestock tracking collar

## System Components

### Hardware
- **ESP32-C6** - Main microcontroller
- **EC200U CN** - 4G LTE module with GPS
- **Collar Housing** - Weatherproof case for Mithun collar
- **Battery** - Long-life power supply
- **SIM Card** - Airtel 4G (airtelgprs.com APN)

### Backend
- **Supabase** - PostgreSQL database
- **Netlify** - Serverless functions
- **GitHub** - Version control

### Frontend
- **Web App** - Real-time location tracking
- **SMS Alerts** - Location updates to farmer's phone

## Database Schema

### 1. Owners (Farmers)
- owner_id, name, phone_number, password

### 2. Devices (Collars)
- device_id, owner_id, device_name
- phone_number (for SMS alerts)
- update_interval (configurable remotely)
- notify_sms, is_active, last_seen

### 3. GPS Locations
- device_id, latitude, longitude, timestamp

## API Endpoints

```
POST /save-gps           - Collar sends GPS location
GET  /get-gps            - Farmer views location on web
GET  /get-device-info    - Collar fetches settings on startup
```

## How It Works

1. **Collar sends GPS** every 60 seconds (configurable)
2. **Server stores location** in database
3. **Updates last_seen** timestamp
4. **Sends SMS** to farmer's phone (if enabled)
5. **Farmer views location** on web or receives SMS link

## Key Features

✅ Real-time GPS tracking
✅ SMS location alerts
✅ Remote configuration (no need to catch Mithun to update settings)
✅ Multiple collars per farmer
✅ Battery monitoring
✅ Weatherproof design for Arunachal Pradesh climate

## Current Status

**Completed:**
- ✅ Database schema with owners, devices, gps_locations
- ✅ API endpoints (save-gps, get-gps, get-device-info)
- ✅ Web frontend for location display
- ✅ ESP32 firmware (basic GPS tracking)

**In Progress:**
- 🔄 Update ESP32 code to send device_id
- 🔄 SMS notification implementation
- 🔄 Multi-device frontend support

**Planned:**
- ⏳ SMS commands (locate, change interval)
- ⏳ Geofencing (alert when Mithun leaves area)
- ⏳ Battery alerts
- ⏳ Movement detection (sleep when stationary)

## Project Structure

```
kapato/
├── index.html              # Web tracking interface
├── netlify/functions/      # API endpoints
├── sql/                    # Database schema
├── audrino/                # ESP32 collar firmware
└── test/                   # API tests
```

## For Farmers

**Login:** Phone number + password
**View Location:** Visit website to see Mithun location on map
**SMS Alerts:** Receive location link via SMS
**Multiple Mithun:** Track multiple collars from one account

## Technical Details

**GPS Update Rate:** 60 seconds (adjustable remotely)
**Network:** 4G LTE (Airtel India)
**Location Accuracy:** ~5-10 meters (GPS dependent)
**Battery Life:** TBD (depends on update interval)
**Coverage:** Requires 4G signal in Arunachal Pradesh

## Repository
**GitHub:** https://github.com/kangtedev/kapato
**Live Site:** https://kapato.netlify.app

---

*Kapato Private Limited - Mithun Tracking Solutions for Arunachal Pradesh*
