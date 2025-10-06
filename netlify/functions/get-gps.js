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

  try {
    // Get optional device_id from query parameter
    const device_id = event.queryStringParameters?.device_id

    let query = `${SUPABASE_URL}/rest/v1/gps_locations?order=created_at.desc&limit=1`

    // If device_id provided, filter by it
    if (device_id) {
      query = `${SUPABASE_URL}/rest/v1/gps_locations?device_id=eq.${device_id}&order=created_at.desc&limit=1`
    }

    const response = await fetch(query, {
      headers: {
        'apikey': SUPABASE_KEY,
        'Authorization': `Bearer ${SUPABASE_KEY}`
      }
    })

    if (!response.ok) {
      throw new Error('Failed to fetch data')
    }

    const data = await response.json()

    if (data.length === 0) {
      return {
        statusCode: 200,
        headers,
        body: JSON.stringify({ message: 'No GPS data yet' })
      }
    }

    const gpsLocation = data[0]

    // Fetch device info to include device_name and phone_number
    const deviceResponse = await fetch(
      `${SUPABASE_URL}/rest/v1/devices?device_id=eq.${gpsLocation.device_id}&select=device_name,phone_number,owner_id`,
      {
        headers: {
          'apikey': SUPABASE_KEY,
          'Authorization': `Bearer ${SUPABASE_KEY}`
        }
      }
    )

    let deviceInfo = {}
    if (deviceResponse.ok) {
      const devices = await deviceResponse.json()
      if (devices.length > 0) {
        deviceInfo = devices[0]
      }
    }

    return {
      statusCode: 200,
      headers,
      body: JSON.stringify({
        ...gpsLocation,
        device_name: deviceInfo.device_name || 'Unknown Device',
        phone_number: deviceInfo.phone_number || null,
        owner_id: deviceInfo.owner_id || null
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
