const https = require('https')

exports.handler = async (event, context) => {
  const headers = {
    'Access-Control-Allow-Origin': '*',
    'Content-Type': 'application/json'
  }

  try {
    // Fetch from GitHub Gist if configured
    if (process.env.GIST_ID) {
      const data = await new Promise((resolve, reject) => {
        https.get({
          hostname: 'api.github.com',
          path: `/gists/${process.env.GIST_ID}`,
          headers: {
            'User-Agent': 'Netlify-Function'
          }
        }, (res) => {
          let body = ''
          res.on('data', chunk => body += chunk)
          res.on('end', () => {
            try {
              const gist = JSON.parse(body)
              const gpsData = gist.files['gps-data.json'].content
              resolve(gpsData)
            } catch (e) {
              reject(e)
            }
          })
        }).on('error', reject)
      })

      return {
        statusCode: 200,
        headers,
        body: data
      }
    }

    return {
      statusCode: 200,
      headers,
      body: JSON.stringify({ message: 'No GPS data yet. Configure GIST_ID in Netlify.' })
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
