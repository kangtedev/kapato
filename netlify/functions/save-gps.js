const fs = require('fs').promises
const path = require('path')

exports.handler = async (event) => {
  const headers = {
    'Access-Control-Allow-Origin': '*',
    'Content-Type': 'application/json'
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

    const newEntry = {
      latitude: parseFloat(latitude),
      longitude: parseFloat(longitude),
      timestamp: new Date().toISOString()
    }

    const filePath = path.join(__dirname, '../../data/gps-data.json')
    await fs.writeFile(filePath, JSON.stringify(newEntry, null, 2))

    return {
      statusCode: 200,
      headers,
      body: JSON.stringify({ success: true, data: newEntry })
    }
  } catch (error) {
    return {
      statusCode: 500,
      headers,
      body: JSON.stringify({ error: error.message })
    }
  }
}
