const https = require('https')

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

    // Store in GitHub Gist or use environment variable
    // For now, just return the data and log it
    console.log('GPS data received:', JSON.stringify(newEntry))

    // Store in Netlify environment (you'll need to set GIST_TOKEN in Netlify)
    if (process.env.GIST_ID && process.env.GIST_TOKEN) {
      const gistData = JSON.stringify({
        files: {
          'gps-data.json': {
            content: JSON.stringify(newEntry)
          }
        }
      })

      await new Promise((resolve, reject) => {
        const req = https.request({
          hostname: 'api.github.com',
          path: `/gists/${process.env.GIST_ID}`,
          method: 'PATCH',
          headers: {
            'Authorization': `token ${process.env.GIST_TOKEN}`,
            'User-Agent': 'Netlify-Function',
            'Content-Type': 'application/json',
            'Content-Length': Buffer.byteLength(gistData)
          }
        }, (res) => {
          res.on('data', () => {})
          res.on('end', resolve)
        })
        req.on('error', reject)
        req.write(gistData)
        req.end()
      })
    }

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