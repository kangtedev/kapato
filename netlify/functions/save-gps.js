const SUPABASE_URL = 'https://oaksqdfenopcywefbsvq.supabase.co'
const SUPABASE_KEY = 'eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9.eyJpc3MiOiJzdXBhYmFzZSIsInJlZiI6Im9ha3NxZGZlbm9wY3l3ZWZic3ZxIiwicm9sZSI6ImFub24iLCJpYXQiOjE3NTg5ODAzNDYsImV4cCI6MjA3NDU1NjM0Nn0.k9dXOwy7wJmgE7nnk1DdNzYswPyv2V-Y5U6CMTv4hZ0'

exports.handler = async (event) => {
  const headers = {
    'Access-Control-Allow-Origin': '*',
    'Content-Type': 'application/json'
  }

  if (event.httpMethod === 'OPTIONS') {
    return { statusCode: 200, headers, body: '' }
  }

  if (event.httpMethod !== 'POST') {
    return {
      statusCode: 405,
      headers,
      body: JSON.stringify({ error: 'POST only' })
    }
  }

  try {
    const { device_id, latitude, longitude } = JSON.parse(event.body)

    // Validate required fields
    if (!device_id || !latitude || !longitude) {
      return {
        statusCode: 400,
        headers,
        body: JSON.stringify({ error: 'Missing device_id, latitude, or longitude' })
      }
    }

    // Step 1: Get device info from devices table
    const deviceResponse = await fetch(
      `${SUPABASE_URL}/rest/v1/devices?device_id=eq.${device_id}&is_active=eq.true&select=*`,
      {
        headers: {
          'apikey': SUPABASE_KEY,
          'Authorization': `Bearer ${SUPABASE_KEY}`
        }
      }
    )

    if (!deviceResponse.ok) {
      throw new Error('Failed to fetch device info')
    }

    const devices = await deviceResponse.json()

    if (devices.length === 0) {
      return {
        statusCode: 404,
        headers,
        body: JSON.stringify({ error: 'Device not found or inactive' })
      }
    }

    const device = devices[0]

    // Step 2: Save GPS location
    const gpsEntry = {
      device_id: device_id,
      latitude: parseFloat(latitude),
      longitude: parseFloat(longitude),
      timestamp: new Date().toISOString()
    }

    const gpsResponse = await fetch(`${SUPABASE_URL}/rest/v1/gps_locations`, {
      method: 'POST',
      headers: {
        'Content-Type': 'application/json',
        'apikey': SUPABASE_KEY,
        'Authorization': `Bearer ${SUPABASE_KEY}`,
        'Prefer': 'return=representation'
      },
      body: JSON.stringify(gpsEntry)
    })

    if (!gpsResponse.ok) {
      const error = await gpsResponse.text()
      throw new Error(error)
    }

    const gpsData = await gpsResponse.json()

    // Step 3: Update device last_seen timestamp
    await fetch(`${SUPABASE_URL}/rest/v1/devices?device_id=eq.${device_id}`, {
      method: 'PATCH',
      headers: {
        'Content-Type': 'application/json',
        'apikey': SUPABASE_KEY,
        'Authorization': `Bearer ${SUPABASE_KEY}`
      },
      body: JSON.stringify({ last_seen: new Date().toISOString() })
    })

    // Step 4: Send SMS notification if enabled
    // TODO: Implement SMS sending logic here
    // if (device.notify_sms && device.phone_number) {
    //   await sendSMS(device.phone_number, latitude, longitude)
    // }

    // Step 5: Return success with device settings for ESP32
    return {
      statusCode: 200,
      headers,
      body: JSON.stringify({
        success: true,
        data: gpsData[0],
        device_settings: {
          update_interval: device.update_interval,
          notify_sms: device.notify_sms
        }
      })
    }
  } catch (error) {
    return {
      statusCode: 500,
      headers,
      body: JSON.stringify({ error: error.message })
    }
  }
}
