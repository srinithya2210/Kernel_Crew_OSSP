SKYPORT frontend update

1. Keep your existing src/App.css and src/index.css unchanged.
2. Replace src/App.jsx with the App.jsx in this package.
3. Copy Dashboard.css into src/Dashboard.css.
4. Keep the Node bridge running in WSL (node bridge/server.js) and the C controller running.
5. Run the React app from the frontend folder with npm run dev.

The frontend uses http://localhost:3001 and GET /api/events as a Server-Sent Events stream.
The event stream only shows events received after the browser connects; it does not replay earlier events.
