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
    const device_id = event.queryStringParameters?.device_id
    const limit = event.queryStringParameters?.limit || '50'

    if (!device_id) {
      return {
        statusCode: 400,
        headers,
        body: JSON.stringify({ error: 'Missing device_id parameter' })
      }
    }

    // Fetch GPS history for this device
    const response = await fetch(
      `${SUPABASE_URL}/rest/v1/gps_locations?device_id=eq.${device_id}&order=created_at.desc&limit=${limit}`,
      {
        headers: {
          'apikey': SUPABASE_KEY,
          'Authorization': `Bearer ${SUPABASE_KEY}`
        }
      }
    )

    if (!response.ok) {
      throw new Error('Failed to fetch GPS history')
    }

    const locations = await response.json()

    return {
      statusCode: 200,
      headers,
      body: JSON.stringify({
        success: true,
        device_id: device_id,
        count: locations.length,
        locations: locations
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
