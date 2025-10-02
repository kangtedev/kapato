const { getStore } = require('@netlify/blobs')

exports.handler = async (event, context) => {
  const headers = {
    'Access-Control-Allow-Origin': '*',
    'Content-Type': 'application/json'
  }

  try {
    const store = getStore('gps-data')
    const data = await store.get('latest')

    if (!data) {
      return {
        statusCode: 200,
        headers,
        body: JSON.stringify({ message: 'No GPS data yet' })
      }
    }

    return {
      statusCode: 200,
      headers,
      body: data
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
