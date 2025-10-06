
-- Create owners table for simple authentication
CREATE TABLE public.owners (
  id BIGSERIAL PRIMARY KEY,
  owner_id VARCHAR(50) UNIQUE NOT NULL,
  name VARCHAR(100) NOT NULL,
  phone_number VARCHAR(20) UNIQUE NOT NULL,        -- Login with phone number
  password TEXT NOT NULL,
  created_at TIMESTAMP WITH TIME ZONE DEFAULT NOW()
) TABLESPACE pg_default;

-- Create devices table (core configuration for each GPS tracker)
CREATE TABLE public.devices (
  id BIGSERIAL PRIMARY KEY,
  device_id VARCHAR(50) UNIQUE NOT NULL,
  owner_id VARCHAR(50) NOT NULL,
  device_name VARCHAR(100) NOT NULL,
  phone_number VARCHAR(20) NOT NULL,              -- SMS notification target
  update_interval INTEGER DEFAULT 60,             -- Seconds between GPS updates
  notify_sms BOOLEAN DEFAULT TRUE,                -- Enable/disable SMS notifications
  is_active BOOLEAN DEFAULT TRUE,                 -- Device active status
  last_seen TIMESTAMP WITH TIME ZONE,             -- Last time device sent data
  created_at TIMESTAMP WITH TIME ZONE DEFAULT NOW(),

  -- Foreign key to link to owners table
  CONSTRAINT fk_device_owner FOREIGN KEY (owner_id) REFERENCES public.owners(owner_id) ON DELETE CASCADE
) TABLESPACE pg_default;

-- Create GPS locations table (simplified - only links to device)
CREATE TABLE public.gps_locations (
  id BIGSERIAL PRIMARY KEY,
  device_id VARCHAR(50) NOT NULL,
  latitude NUMERIC(10, 8) NOT NULL,
  longitude NUMERIC(11, 8) NOT NULL,
  timestamp TEXT NULL,
  created_at TIMESTAMP WITH TIME ZONE DEFAULT NOW(),

  -- Foreign key to link to devices table
  CONSTRAINT fk_location_device FOREIGN KEY (device_id) REFERENCES public.devices(device_id) ON DELETE CASCADE
) TABLESPACE pg_default;

-- Create indexes for better performance
CREATE INDEX idx_devices_owner ON public.devices(owner_id);
CREATE INDEX idx_devices_active ON public.devices(is_active);
CREATE INDEX idx_gps_device ON public.gps_locations(device_id);
CREATE INDEX idx_gps_created ON public.gps_locations(created_at DESC);
CREATE INDEX idx_gps_device_created ON public.gps_locations(device_id, created_at DESC);

-- Insert sample owner
INSERT INTO public.owners (owner_id, name, phone_number, password) VALUES
  ('OWNER001', 'Admin User', '9876543210', 'admin123');

-- Insert sample device
INSERT INTO public.devices (device_id, owner_id, device_name, phone_number, update_interval, notify_sms) VALUES
  ('ESP32-001', 'OWNER001', 'Test Tracker', '9876543210', 60, TRUE);

-- Example: Insert sample GPS data for testing
-- INSERT INTO public.gps_locations (device_id, latitude, longitude) VALUES
--   ('ESP32-001', 12.9716, 77.5946);
