const { getStore } = require('@netlify/blobs')

exports.handler = async (event, context) => {
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
    const { latitude, longitude } = JSON.parse(event.body)

    if (!latitude || !longitude) {
      return {
        statusCode: 400,
        headers,
        body: JSON.stringify({ error: 'Missing lat/lon' })
      }
    }

    const newEntry = {
      id: Date.now(),
      latitude: parseFloat(latitude),
      longitude: parseFloat(longitude),
      timestamp: new Date().toISOString()
    }

    // Store data in Netlify Blobs
    const store = getStore('gps-data')
    await store.set('latest', JSON.stringify(newEntry))

    console.log('GPS data saved:', newEntry)

    return {
      statusCode: 200,
      headers,
      body: JSON.stringify({
        success: true,
        data: newEntry
      })
    }
  } catch (error) {
    console.error('Error:', error)
    return {
      statusCode: 500,
      headers,
      body: JSON.stringify({ error: error.message })
    }
  }
}