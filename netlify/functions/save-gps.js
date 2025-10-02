const fs = require('fs').promises
const path = require('path')

exports.handler = async (event) => {
  const headers = {
    'Access-Control-Allow-Origin': '*',
    'Content-Type': 'application/json'
  }

  if (event.httpMethod === 'OPTIONS') {
    return { statusCode: 200, headers }
  }

  if (event.httpMethod !== 'POST') {
    return { statusCode: 405, headers, body: JSON.stringify({ error: 'POST only' }) }
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

    // Create new entry
    const newEntry = {
      id: Date.now(),
      latitude: parseFloat(latitude),
      longitude: parseFloat(longitude),
      timestamp: new Date().toISOString()
    }

    // Note: Netlify functions are stateless, so this uses a simple approach
    // For production, use a proper database or Netlify Blobs
    
    return {
      statusCode: 200,
      headers,
      body: JSON.stringify({ 
        success: true, 
        data: newEntry,
        note: 'Data received. For persistent storage, integrate a database service.'
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