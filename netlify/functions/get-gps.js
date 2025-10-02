const fs = require('fs')
const path = require('path')

exports.handler = async () => {
  const headers = {
    'Access-Control-Allow-Origin': '*',
    'Content-Type': 'application/json'
  }

  try {
    const filePath = path.join(__dirname, '../../data/gps-data.json')
    const data = fs.readFileSync(filePath, 'utf8')

    return {
      statusCode: 200,
      headers,
      body: data
    }
  } catch (error) {
    return {
      statusCode: 200,
      headers,
      body: JSON.stringify({ message: 'No GPS data yet' })
    }
  }
}
