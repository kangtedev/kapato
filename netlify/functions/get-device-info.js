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
    // Get device_id from query parameter
    const device_id = event.queryStringParameters?.device_id

    if (!device_id) {
      return {
        statusCode: 400,
        headers,
        body: JSON.stringify({ error: 'Missing device_id parameter' })
      }
    }

    // Fetch device info from devices table
    const response = await fetch(
      `${SUPABASE_URL}/rest/v1/devices?device_id=eq.${device_id}&is_active=eq.true&select=*`,
      {
        headers: {
          'apikey': SUPABASE_KEY,
          'Authorization': `Bearer ${SUPABASE_KEY}`
        }
      }
    )

    if (!response.ok) {
      throw new Error('Failed to fetch device info')
    }

    const devices = await response.json()

    if (devices.length === 0) {
      return {
        statusCode: 404,
        headers,
        body: JSON.stringify({ error: 'Device not found or inactive' })
      }
    }

    const device = devices[0]

    // Return device settings for ESP32
    return {
      statusCode: 200,
      headers,
      body: JSON.stringify({
        success: true,
        device_id: device.device_id,
        device_name: device.device_name,
        phone_number: device.phone_number,
        update_interval: device.update_interval,
        notify_sms: device.notify_sms,
        is_active: device.is_active,
        last_seen: device.last_seen
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
