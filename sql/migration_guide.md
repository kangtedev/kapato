# Database Migration Guide

## Overview
This guide helps you upgrade from the basic `gps_locations` table to the enhanced schema.

## Migration Options

### Option 1: Fresh Start (No Data Loss Risk)
Best if you're still in development or don't need old data.

```sql
-- Rename old table as backup
ALTER TABLE public.gps_locations RENAME TO gps_locations_old;

-- Run enhanced_table.sql to create new structure

-- Verify new table works, then drop old one
-- DROP TABLE public.gps_locations_old;
```

### Option 2: Preserve Existing Data
Migrate your existing GPS data to the new schema.

```sql
-- 1. Create new table (run enhanced_table.sql)

-- 2. Migrate existing data
INSERT INTO public.gps_locations (
  device_id,
  device_name,
  latitude,
  longitude,
  location_source,
  created_at
)
SELECT
  'ESP32-DEFAULT',              -- Set default device ID
  'Legacy Device',              -- Set default name
  latitude::DOUBLE PRECISION,
  longitude::DOUBLE PRECISION,
  'GPS',                        -- Assume GPS source
  created_at
FROM public.gps_locations_old;

-- 3. Verify migration
SELECT COUNT(*) FROM gps_locations_old;
SELECT COUNT(*) FROM gps_locations;

-- 4. Drop old table once verified
-- DROP TABLE public.gps_locations_old;
```

### Option 3: Side-by-Side (Testing)
Keep both tables during testing phase.

```sql
-- Create enhanced table with different name
CREATE TABLE public.gps_locations_v2 (...);

-- Update your Arduino code to post to new endpoint
-- Update Netlify functions to write to gps_locations_v2
-- Once stable, rename tables
```

## Data That EC200U CN Can Provide

### Available from AT Commands:
- ✅ **GPS Coordinates** - `AT+QGPSLOC=2`
- ✅ **Speed** - From `AT+QGPSLOC=2` response
- ✅ **Heading** - Course over ground
- ✅ **Altitude** - Meters above sea level
- ✅ **HDOP** - Horizontal accuracy
- ✅ **Satellites** - Number in view
- ✅ **GPS Time** - UTC timestamp
- ✅ **Signal Strength** - `AT+CSQ` (0-31 + 99)
- ✅ **Network Type** - `AT+QNWINFO` (LTE/4G/3G)
- ✅ **Cell ID** - `AT+QENG="servingcell"`

### Available from ESP32-C6:
- ✅ **MAC Address** - Use as device_id
- ✅ **WiFi RSSI** - If using WiFi
- ✅ **Battery Voltage** - ADC reading
- ⚠️ **Movement Detection** - Needs accelerometer (not built-in)

### Calculated/Logic-based:
- ✅ **is_moving** - Calculate from speed threshold
- ✅ **location_source** - Track fallback logic
- ✅ **data_quality** - Based on HDOP/satellites

## Field Mapping for Arduino Code

```cpp
// What EC200U AT+QGPSLOC=2 returns:
// +QGPSLOC: <UTC>,<latitude>,<longitude>,<hdop>,<altitude>,<fix>,<cog>,<spkm>,<spkn>,<date>,<nsat>

// Example response:
// +QGPSLOC: 103433.0,12.9716,77.5946,1.2,920.3,3,65.4,12.5,6.8,240125,8

// Parse into:
float latitude    = 12.9716;      // Column: latitude
float longitude   = 77.5946;      // Column: longitude
float hdop        = 1.2;          // Column: hdop (1.2 = excellent)
float altitude    = 920.3;        // Column: altitude (meters)
int fixType       = 3;            // Column: fix_type (3 = "3D")
float heading     = 65.4;         // Column: heading (degrees)
float speedKm     = 12.5;         // Column: speed (km/h)
int satellites    = 8;            // Column: satellites
String gpsTime    = "103433.0";   // Column: gps_timestamp
String date       = "240125";     // Parse to date
```

## Next Steps

1. **Choose migration strategy** (recommend Option 1 for testing)
2. **Run SQL in Supabase dashboard** or use migration tool
3. **Update Arduino code** to send new fields
4. **Update Netlify functions** to handle new schema
5. **Update frontend** to display additional data

## Testing Checklist

- [ ] Backup existing data
- [ ] Run enhanced_table.sql in Supabase
- [ ] Verify table structure with `\d gps_locations`
- [ ] Test insert with sample data
- [ ] Verify indexes with `\di`
- [ ] Check views are created
- [ ] Update Arduino code
- [ ] Test end-to-end data flow
