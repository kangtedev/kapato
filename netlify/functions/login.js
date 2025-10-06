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
    const { phone_number, password } = JSON.parse(event.body)

    if (!phone_number || !password) {
      return {
        statusCode: 400,
        headers,
        body: JSON.stringify({ error: 'Missing phone_number or password' })
      }
    }

    // Fetch owner from database
    const response = await fetch(
      `${SUPABASE_URL}/rest/v1/owners?phone_number=eq.${phone_number}&password=eq.${password}&select=*`,
      {
        headers: {
          'apikey': SUPABASE_KEY,
          'Authorization': `Bearer ${SUPABASE_KEY}`
        }
      }
    )

    if (!response.ok) {
      throw new Error('Failed to fetch owner')
    }

    const owners = await response.json()

    if (owners.length === 0) {
      return {
        statusCode: 401,
        headers,
        body: JSON.stringify({ error: 'Invalid phone number or password' })
      }
    }

    const owner = owners[0]

    // Return owner info (excluding password)
    return {
      statusCode: 200,
      headers,
      body: JSON.stringify({
        success: true,
        owner: {
          owner_id: owner.owner_id,
          name: owner.name,
          phone_number: owner.phone_number
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
