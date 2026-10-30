/* Lightweight Node.js bridge for the Airport Ground Operations IPC demo.
 * No npm packages are required. Run from Ubuntu/WSL after compiling the C programs.
 */
const http = require('http');
const fs = require('fs');
const readline = require('readline');
const { URL } = require('url');

const PORT = Number(process.env.PORT || 3001);
const COMMAND_FIFO = '/tmp/airport_frontend_cmd_fifo';
const EVENTS_FIFO = '/tmp/airport_events_fifo';
const clients = new Set();

function sendSse(res, event) {
  res.write(`data: ${JSON.stringify(event)}\n\n`);
}

function broadcast(event) {
  for (const res of clients) sendSse(res, event);
}

function writeCommand(command) {
  // The C controller opens this FIFO O_RDWR, so this write will have a reader.
  fs.writeFileSync(COMMAND_FIFO, `${command}\n`);
}

function readRequestBody(req) {
  return new Promise((resolve, reject) => {
    let body = '';
    req.on('data', chunk => {
      body += chunk;
      if (body.length > 16_384) {
        reject(new Error('Request body too large'));
        req.destroy();
      }
    });
    req.on('end', () => {
      try { resolve(JSON.parse(body || '{}')); }
      catch { reject(new Error('Invalid JSON body')); }
    });
    req.on('error', reject);
  });
}

function reply(res, status, data) {
  res.writeHead(status, {
    'Content-Type': 'application/json; charset=utf-8',
    'Access-Control-Allow-Origin': '*',
    'Access-Control-Allow-Headers': 'Content-Type',
    'Access-Control-Allow-Methods': 'GET,POST,OPTIONS',
  });
  res.end(JSON.stringify(data));
}

const server = http.createServer(async (req, res) => {
  const url = new URL(req.url, `http://${req.headers.host || 'localhost'}`);
  if (req.method === 'OPTIONS') return reply(res, 204, {});

  if (req.method === 'GET' && url.pathname === '/api/events') {
    res.writeHead(200, {
      'Content-Type': 'text/event-stream',
      'Cache-Control': 'no-cache',
      'Connection': 'keep-alive',
      'Access-Control-Allow-Origin': '*',
    });
    res.write(': connected to airport IPC bridge\n\n');
    clients.add(res);
    req.on('close', () => clients.delete(res));
    return;
  }

  try {
    if (req.method === 'POST' && url.pathname === '/api/flights') {
      const body = await readRequestBody(req);
      const flight = String(body.flight || '').trim().toUpperCase();
      const time = String(body.time || 'MANUAL').trim();
      if (!/^[A-Z0-9-]{2,31}$/.test(flight)) return reply(res, 400, { error: 'Flight ID may contain only letters, numbers and hyphens (2–31 characters).' });
      if (!/^[A-Za-z0-9: _-]{1,31}$/.test(time)) return reply(res, 400, { error: 'Invalid time label.' });
      writeCommand(`ADD|${flight}|${time}`);
      return reply(res, 202, { accepted: true, flight, time });
    }

    if (req.method === 'POST' && url.pathname === '/api/emergency') {
      const body = await readRequestBody(req);
      const message = String(body.message || 'Emergency vehicle approaching Gate B')
        .replace(/[|\r\n\0]/g, ' ').trim().slice(0, 140);
      writeCommand(`EMERGENCY|${message}`);
      return reply(res, 202, { accepted: true, message });
    }

    if (req.method === 'POST' && url.pathname === '/api/status') {
      writeCommand('STATUS');
      return reply(res, 202, { accepted: true });
    }

    return reply(res, 404, { error: 'Not found' });
  } catch (error) {
    return reply(res, 500, { error: error.message || 'Bridge error' });
  }
});

// A FIFO reader can receive EOF whenever the C writer closes its descriptor.
// Reopen it after EOF/errors so the bridge remains subscribed to future events.
function watchEventsFifo() {
  fs.access(EVENTS_FIFO, fs.constants.F_OK, error => {
    if (error) return setTimeout(watchEventsFifo, 1000);
    let stream;
    try { stream = fs.createReadStream(EVENTS_FIFO, { encoding: 'utf8' }); }
    catch { return setTimeout(watchEventsFifo, 1000); }
    const lines = readline.createInterface({ input: stream, crlfDelay: Infinity });
    lines.on('line', line => {
      if (!line.trim()) return;
      try { broadcast(JSON.parse(line)); }
      catch { broadcast({ type: 'bridge', status: 'PARSE_ERROR', message: line }); }
    });
    let reopened = false;
    const reopen = () => {
      if (reopened) return;
      reopened = true;
      lines.close();
      setTimeout(watchEventsFifo, 250);
    };
    stream.on('end', reopen);
    stream.on('error', reopen);
    stream.on('close', reopen);
  });
}

server.listen(PORT, '0.0.0.0', () => {
  console.log(`[BRIDGE] HTTP API listening on http://localhost:${PORT}`);
  console.log('[BRIDGE] Start ./controller in this same WSL environment to receive IPC events.');
  watchEventsFifo();
});
