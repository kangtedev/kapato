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

  if (event.httpMethod !== 'GET') {
    return {
      statusCode: 405,
      headers,
      body: JSON.stringify({ error: 'GET only' })
    }
  }

  try {
    const owner_id = event.queryStringParameters?.owner_id

    if (!owner_id) {
      return {
        statusCode: 400,
        headers,
        body: JSON.stringify({ error: 'Missing owner_id parameter' })
      }
    }

    // Fetch all devices for this owner
    const devicesResponse = await fetch(
      `${SUPABASE_URL}/rest/v1/devices?owner_id=eq.${owner_id}&select=*&order=created_at.desc`,
      {
        headers: {
          'apikey': SUPABASE_KEY,
          'Authorization': `Bearer ${SUPABASE_KEY}`
        }
      }
    )

    if (!devicesResponse.ok) {
      throw new Error('Failed to fetch devices')
    }

    const devices = await devicesResponse.json()

    // For each device, get latest GPS location
    const devicesWithLocation = await Promise.all(
      devices.map(async (device) => {
        const gpsResponse = await fetch(
          `${SUPABASE_URL}/rest/v1/gps_locations?device_id=eq.${device.device_id}&order=created_at.desc&limit=1`,
          {
            headers: {
              'apikey': SUPABASE_KEY,
              'Authorization': `Bearer ${SUPABASE_KEY}`
            }
          }
        )

        let latestLocation = null
        if (gpsResponse.ok) {
          const locations = await gpsResponse.json()
          if (locations.length > 0) {
            latestLocation = locations[0]
          }
        }

        return {
          ...device,
          latest_location: latestLocation
        }
      })
    )

    return {
      statusCode: 200,
      headers,
      body: JSON.stringify({
        success: true,
        devices: devicesWithLocation
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
