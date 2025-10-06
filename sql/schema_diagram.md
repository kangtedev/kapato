# Database Schema - GPS Tracker System

## Table Relationships

```
┌─────────────────┐
│     owners      │
│─────────────────│
│ owner_id (PK)   │
│ name            │
│ password        │
│ email           │
└────────┬────────┘
         │
         │ 1:N (one owner has many devices)
         │
┌────────▼────────────────┐
│      devices            │
│─────────────────────────│
│ device_id (PK)          │
│ owner_id (FK)           │
│ device_name             │
│ phone_number            │← SMS target
│ update_interval         │← Configurable remotely
│ notify_sms              │← Enable/disable SMS
│ is_active               │
│ firmware_version        │
│ last_seen               │
└────────┬────────────────┘
         │
         │ 1:N (one device has many GPS records)
         │
┌────────▼────────────────┐
│   gps_locations         │
│─────────────────────────│
│ id (PK)                 │
│ device_id (FK)          │
│ latitude                │
│ longitude               │
│ timestamp               │
│ created_at              │
└─────────────────────────┘
```

## Key Features

### Owners Table
- Basic authentication (owner_id + password)
- Can own multiple devices
- Removed phone from owners (moved to devices)

### Devices Table (NEW!)
- **Core configuration for each ESP32 tracker**
- Stores phone_number (SMS notification target)
- Configurable update_interval (no reflashing needed!)
- notify_sms flag (enable/disable notifications)
- is_active status (activate/deactivate devices)
- last_seen timestamp (monitor device health)
- firmware_version (track ESP32 code version)

### GPS Locations Table
- Simplified - only links to device_id
- All device info comes from devices table
- Faster queries (no joins with owners needed)

## Data Flow

### ESP32 Side:
```
ESP32 sends:
{
  "device_id": "ESP32-001",
  "latitude": 12.9716,
  "longitude": 77.5946
}
```

### Server Side (Netlify Function):
```
1. Receive GPS data
2. Query devices table WHERE device_id = 'ESP32-001'
3. Get: phone_number, notify_sms, update_interval
4. Save GPS to gps_locations table
5. If notify_sms = TRUE:
   → Send SMS to phone_number with location
6. Update devices.last_seen = NOW()
7. Return update_interval to ESP32
```

### ESP32 Response:
```
{
  "success": true,
  "update_interval": 60  ← ESP32 uses this for next cycle
}
```

## Sample Queries

### Get device info for ESP32:
```sql
SELECT device_name, phone_number, update_interval, notify_sms
FROM devices
WHERE device_id = 'ESP32-001' AND is_active = TRUE;
```

### Get latest location for a device:
```sql
SELECT latitude, longitude, created_at
FROM gps_locations
WHERE device_id = 'ESP32-001'
ORDER BY created_at DESC
LIMIT 1;
```

### Get all devices for an owner:
```sql
SELECT device_id, device_name, phone_number, is_active, last_seen
FROM devices
WHERE owner_id = 'OWNER001'
ORDER BY device_name;
```

### Check device health (not seen in 5 minutes):
```sql
SELECT device_id, device_name, last_seen
FROM devices
WHERE is_active = TRUE
  AND last_seen < NOW() - INTERVAL '5 minutes';
```

## Migration from Old Schema

If you already ran the old schema, run this to upgrade:

```sql
-- Drop old tables
DROP TABLE IF EXISTS gps_locations;
DROP TABLE IF EXISTS owners;

-- Then run the new simple_update.sql
```

## Testing

After running the SQL:
1. Check tables exist: `\dt` in psql
2. Verify sample data:
   ```sql
   SELECT * FROM owners;
   SELECT * FROM devices;
   ```
3. Test insert:
   ```sql
   INSERT INTO gps_locations (device_id, latitude, longitude)
   VALUES ('ESP32-001', 12.9716, 77.5946);
   ```
