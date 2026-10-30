import { useEffect, useState } from "react";
import "./App.css";
import "./Dashboard.css";

const API_BASE = "http://localhost:3001";
const MAX_EVENTS = 150;

function App() {
  const [dark, setDark] = useState(() => {
    try {
      return localStorage.getItem("airport-theme") === "dark";
    } catch {
      return false;
    }
  });
  const [page, setPage] = useState("home");
  const [connection, setConnection] = useState("connecting");
  const [events, setEvents] = useState([]);
  const [flights, setFlights] = useState([]);
  const [gates, setGates] = useState([
    { name: "A", status: "UNKNOWN", flight: "" },
    { name: "B", status: "UNKNOWN", flight: "" },
    { name: "C", status: "UNKNOWN", flight: "" },
  ]);
  const [waiting, setWaiting] = useState([]);
  const [announcements, setAnnouncements] = useState([]);
  const [baggage, setBaggage] = useState([]);
  const [flightCode, setFlightCode] = useState("");
  const [flightTime, setFlightTime] = useState("17:30");
  const [emergencyMessage, setEmergencyMessage] = useState(
    "Emergency vehicle approaching Gate B"
  );
  const [notice, setNotice] = useState("");
  const [busy, setBusy] = useState(false);

  useEffect(() => {
    document.documentElement.setAttribute("data-theme", dark ? "dark" : "light");
    try {
      localStorage.setItem("airport-theme", dark ? "dark" : "light");
    } catch {
      // Theme still works if browser storage is unavailable.
    }
  }, [dark]);

  useEffect(() => {
    const source = new EventSource(`${API_BASE}/api/events`);

    source.onopen = () => setConnection("connected");
    source.onerror = () => setConnection("disconnected");
    source.onmessage = (message) => {
      try {
        const event = JSON.parse(message.data);
        setEvents((previous) => [event, ...previous].slice(0, MAX_EVENTS));

        if (event.type === "flight" && event.flight) {
          setFlights((previous) => {
            const existing = previous.find((item) => item.flight === event.flight);
            const next = {
              flight: event.flight,
              status: event.status || existing?.status || "UNKNOWN",
              gate: event.gate || existing?.gate || "—",
              message: event.message || "",
              timestamp: event.timestamp || Math.floor(Date.now() / 1000),
            };
            return [next, ...previous.filter((item) => item.flight !== event.flight)].slice(0, 100);
          });

          if (event.status === "WAITLISTED") {
            setWaiting((previous) => previous.includes(event.flight) ? previous : [...previous, event.flight]);
          } else if (["APPROVED", "COMPLETED", "CANCELLED"].includes(event.status)) {
            setWaiting((previous) => previous.filter((flight) => flight !== event.flight));
          }
        }

        if (event.type === "queue" && event.flight && event.status === "PROMOTED") {
          setWaiting((previous) => previous.filter((flight) => flight !== event.flight));
        }

        if (event.type === "gate" && event.gate) {
          setGates((previous) => previous.map((gate) => {
            if (gate.name !== event.gate) return gate;
            if (event.status === "AVAILABLE") return { ...gate, status: "AVAILABLE", flight: "" };
            if (event.status === "OCCUPIED") return { ...gate, status: "OCCUPIED", flight: event.flight || gate.flight };
            return { ...gate, status: event.status || gate.status, flight: event.flight || gate.flight };
          }));
        }

        if (event.type === "announcement") {
          setAnnouncements((previous) => [event, ...previous].slice(0, 20));
        }
        if (event.type === "baggage") {
          setBaggage((previous) => [event, ...previous.filter((item) => item.flight !== event.flight)].slice(0, 20));
        }
      } catch {
        // Ignore malformed event data rather than breaking the dashboard.
      }
    };

    return () => source.close();
  }, []);

  async function postToBridge(path, body = {}) {
    const response = await fetch(`${API_BASE}${path}`, {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify(body),
    });
    const result = await response.json().catch(() => ({}));
    if (!response.ok) throw new Error(result.error || `Request failed (${response.status})`);
    return result;
  }

  async function addFlight(event) {
    event.preventDefault();
    if (!flightCode.trim()) {
      setNotice("Enter a flight number first.");
      return;
    }
    setBusy(true);
    setNotice("");
    try {
      const result = await postToBridge("/api/flights", {
        flight: flightCode.trim().toUpperCase(),
        time: flightTime || "MANUAL",
      });
      setNotice(`Request accepted for ${result.flight}. Waiting for the C controller event…`);
      setFlightCode("");
    } catch (error) {
      setNotice(`${error.message}. Check that the WSL bridge is running on port 3001.`);
    } finally {
      setBusy(false);
    }
  }

  async function sendEmergency() {
    setBusy(true);
    setNotice("");
    try {
      const result = await postToBridge("/api/emergency", { message: emergencyMessage });
      setNotice(`Emergency request sent: ${result.message}`);
    } catch (error) {
      setNotice(`${error.message}. Check the WSL bridge and controller.`);
    } finally {
      setBusy(false);
    }
  }

  async function requestGateStatus() {
    setBusy(true);
    setNotice("");
    try {
      await postToBridge("/api/status");
      setNotice("Gate status request sent to the C controller.");
    } catch (error) {
      setNotice(`${error.message}. Check the WSL bridge and controller.`);
    } finally {
      setBusy(false);
    }
  }

  const connected = connection === "connected";
  const availableCount = gates.filter((gate) => gate.status === "AVAILABLE").length;
  const occupiedCount = gates.filter((gate) => gate.status === "OCCUPIED").length;
  const activeFlights = flights.filter((flight) => flight.status !== "COMPLETED").length;
  const formatTime = (timestamp) => {
    if (!timestamp) return "—";
    return new Date(timestamp * 1000).toLocaleTimeString([], { hour: "2-digit", minute: "2-digit", second: "2-digit" });
  };

  return (
    <div className="app">
      <header className="navbar">
        <button className="brand" onClick={() => setPage("home")}>
          <span className="brand-icon">✈</span>
          <span><strong>SKYPORT</strong><small>GROUND OPERATIONS</small></span>
        </button>
        <div className="nav-right">
          <span className="system-status">
            <span className={`status-dot ${connected ? "" : "status-dot-off"}`} />
            {connected ? "IPC BRIDGE CONNECTED" : connection === "connecting" ? "CONNECTING TO BRIDGE" : "BRIDGE DISCONNECTED"}
          </span>
          <button className="theme-toggle" onClick={() => setDark(!dark)} aria-label="Toggle color theme">
            {dark ? "☀️ Light Mode" : "🌙 Dark Mode"}
          </button>
        </div>
      </header>

      {page === "home" && (
        <main>
          <section className="hero">
            <div className="hero-copy">
              <div className="eyebrow"><span className="status-dot" /> AIRPORT MANAGEMENT SYSTEM</div>
              <h1>Every flight.<br /><span>Every connection.</span><br />Under control.</h1>
              <p>A unified airport ground operations platform that coordinates flights, gates, baggage and announcements through real-time inter-process communication.</p>
              <div className="hero-actions">
                <button className="primary-btn" onClick={() => setPage("passenger")}>Passenger View <span>→</span></button>
                <button className="secondary-btn" onClick={() => setPage("control")}><span>⌘</span> System Control Center</button>
              </div>
              <div className="hero-note"><span className={`status-dot ${connected ? "" : "status-dot-off"}`} /> LINUX IPC · C PROCESSES · PIPE & FIFO</div>
            </div>
            <div className="hero-visual">
              <div className="orbit orbit-one" /><div className="orbit orbit-two" />
              <div className="radar-center"><span className="radar-plane">✈</span></div>
              <div className="floating-card flight-card"><span className="mini-label">FLIGHT OPERATIONS</span><strong>{flights[0]?.flight || "AI-502"}</strong><span className="flight-route">LIVE FLIGHT EVENT</span><span className="flight-badge">● {flights[0]?.status || "AWAITING EVENTS"}</span></div>
              <div className="floating-card gate-card"><span className="mini-label">GATE STATUS</span><strong>{occupiedCount} / 3 OCCUPIED</strong><span className="gate-description">{availableCount} gate(s) available</span></div>
              <div className="visual-caption"><span className={`status-dot ${connected ? "" : "status-dot-off"}`} /> OPERATIONS OVERVIEW</div>
            </div>
          </section>
          <section className="features">
            <div className="section-heading"><div><span className="eyebrow">ONE SYSTEM. TWO PERSPECTIVES.</span><h2>Welcome to Skyport</h2></div><p>Passenger information on one side.<br />System-level communication on the other.</p></div>
            <div className="feature-grid">
              <button className="feature-card" onClick={() => setPage("passenger")}><span className="feature-icon">✈</span><span className="feature-number">01 / PASSENGER</span><h3>Passenger View</h3><p>Flight information, gate assignments, baggage updates and airport announcements.</p><span className="feature-link">Explore flights <span>↗</span></span></button>
              <button className="feature-card" onClick={() => setPage("control")}><span className="feature-icon">⌘</span><span className="feature-number">02 / SYSTEM</span><h3>Control Center</h3><p>Monitor C processes, inspect PIPE/FIFO events and observe airport operations.</p><span className="feature-link">Open control center <span>↗</span></span></button>
            </div>
          </section>
        </main>
      )}

      {page === "passenger" && (
        <main className="dashboard-page">
          <button className="back-btn" onClick={() => setPage("home")}>← Back to Home</button>
          <div className="dashboard-heading"><div><span className="eyebrow">PASSENGER SERVICES</span><h1>Flight information</h1><p>Live updates received from the airport's Linux IPC processes.</p></div><span className={`connection-pill ${connected ? "is-connected" : ""}`}><span className={`status-dot ${connected ? "" : "status-dot-off"}`} />{connected ? "LIVE CONNECTION" : "NOT CONNECTED"}</span></div>
          <div className="metric-grid">
            <Metric label="FLIGHT RECORDS" value={flights.length} detail="Events received" />
            <Metric label="WAITING FLIGHTS" value={waiting.length} detail="Awaiting a gate" />
            <Metric label="GATES AVAILABLE" value={availableCount} detail={`of ${gates.length} tracked gates`} />
            <Metric label="ACTIVE / NOT COMPLETED" value={activeFlights} detail="Based on latest flight events" />
          </div>
          <section className="panel"><div className="panel-heading"><div><h2>Flight board</h2><p>Updates as the C controller publishes events.</p></div><button className="small-action" onClick={requestGateStatus} disabled={busy}>Refresh gate status</button></div>
            {flights.length === 0 ? <EmptyState connected={connected} text="No flight events received yet. Keep the controller and bridge running, then add a flight from the Control Center." /> : <div className="table-wrap"><table className="data-table"><thead><tr><th>Flight</th><th>Status</th><th>Gate</th><th>Latest update</th><th>Time</th></tr></thead><tbody>{flights.map((flight) => <tr key={flight.flight}><td className="flight-id">{flight.flight}</td><td><StatusTag status={flight.status} /></td><td>{flight.gate || "—"}</td><td>{flight.message || "—"}</td><td>{formatTime(flight.timestamp)}</td></tr>)}</tbody></table></div>}
          </section>
          <div className="two-column-panels">
            <section className="panel"><div className="panel-heading"><div><h2>Baggage updates</h2><p>Messages from the baggage process.</p></div></div>{baggage.length ? <div className="update-list">{baggage.map((item, index) => <div className="update-row" key={`${item.flight}-${item.timestamp}-${index}`}><span className="update-icon">▣</span><div><strong>{item.flight || "Flight"} · {item.status}</strong><p>{item.message}</p></div><small>{item.gate ? `Gate ${item.gate}` : ""}</small></div>)}</div> : <EmptyState connected={connected} text="Baggage updates will appear here when the baggage process reports an event." />}</section>
            <section className="panel"><div className="panel-heading"><div><h2>Announcements</h2><p>Published arrival announcements.</p></div></div>{announcements.length ? <div className="update-list">{announcements.map((item, index) => <div className="update-row" key={`${item.flight}-${item.timestamp}-${index}`}><span className="update-icon">◖</span><div><strong>{item.flight || "Airport"} · {item.status}</strong><p>{item.message}</p></div><small>{item.gate ? `Gate ${item.gate}` : ""}</small></div>)}</div> : <EmptyState connected={connected} text="Airport announcements will appear here when published by the C process." />}</section>
          </div>
        </main>
      )}

      {page === "control" && (
        <main className="dashboard-page">
          <button className="back-btn" onClick={() => setPage("home")}>← Back to Home</button>
          <div className="dashboard-heading"><div><span className="eyebrow">LINUX IPC MONITOR</span><h1>System Control Center</h1><p>Submit commands to the Node bridge and monitor events emitted by the C processes.</p></div><span className={`connection-pill ${connected ? "is-connected" : ""}`}><span className={`status-dot ${connected ? "" : "status-dot-off"}`} />{connected ? "BRIDGE CONNECTED" : connection === "connecting" ? "CONNECTING" : "BRIDGE OFFLINE"}</span></div>
          {notice && <div className="notice-banner" role="status"><span>{notice}</span><button onClick={() => setNotice("")} aria-label="Dismiss message">×</button></div>}
          <div className="control-grid">
            <section className="panel command-panel"><div className="panel-heading"><div><h2>Add a flight</h2><p>Send a request to the C controller through the bridge.</p></div><span className="panel-symbol">✈</span></div><form onSubmit={addFlight} className="command-form"><label htmlFor="flight-code">Flight number</label><input id="flight-code" value={flightCode} onChange={(event) => setFlightCode(event.target.value)} placeholder="e.g. AI-777" maxLength={31} required /><label htmlFor="flight-time">Arrival time</label><input id="flight-time" type="time" value={flightTime} onChange={(event) => setFlightTime(event.target.value)} /><button className="primary-btn form-submit" type="submit" disabled={busy || !connected}>{busy ? "Sending…" : "Submit flight request →"}</button></form><p className="helper-text">A successful HTTP response means the bridge accepted the command. Watch the event log below to confirm the controller's response.</p></section>
            <section className="panel command-panel"><div className="panel-heading"><div><h2>Emergency control</h2><p>Send an emergency message through the IPC bridge.</p></div><span className="panel-symbol emergency-symbol">!</span></div><label htmlFor="emergency-message">Emergency message</label><textarea id="emergency-message" rows="3" value={emergencyMessage} onChange={(event) => setEmergencyMessage(event.target.value)} maxLength={140} /><button className="danger-btn" onClick={sendEmergency} disabled={busy || !connected}>Send emergency request</button><p className="helper-text">The resulting action depends on the emergency handling implemented in your running C controller.</p></section>
          </div>
          <section className="panel gate-panel"><div className="panel-heading"><div><h2>Gate monitor</h2><p>Gate state is updated from incoming controller events.</p></div><button className="small-action" onClick={requestGateStatus} disabled={busy || !connected}>Request status</button></div><div className="gate-grid">{gates.map((gate) => <div className={`gate-tile ${gate.status === "AVAILABLE" ? "gate-available" : gate.status === "OCCUPIED" ? "gate-occupied" : "gate-unknown"}`} key={gate.name}><div className="gate-tile-top"><span>GATE {gate.name}</span><span className="gate-state-dot" /></div><strong>{gate.status === "UNKNOWN" ? "Awaiting status" : gate.status}</strong><small>{gate.flight ? `Flight ${gate.flight}` : gate.status === "AVAILABLE" ? "Ready for assignment" : "No current flight reported"}</small></div>)}</div><div className="queue-line"><div><span className="queue-number">{waiting.length}</span><span><strong>Waiting queue</strong><small>Flights currently reported as waitlisted</small></span></div><div className="queue-chips">{waiting.length ? waiting.map((flight) => <span className="queue-chip" key={flight}>{flight}</span>) : <span className="muted-copy">No waiting flights reported yet</span>}</div></div></section>
          <section className="panel"><div className="panel-heading"><div><h2>IPC event stream</h2><p>Newest events first · maximum {MAX_EVENTS} events shown in this session.</p></div><span className="event-count">{events.length} events</span></div>{events.length === 0 ? <EmptyState connected={connected} text="Waiting for events from /tmp/airport_events_fifo. Existing events from before this page connected are not replayed." /> : <div className="event-list">{events.map((event, index) => <div className="event-row" key={`${event.timestamp || "time"}-${index}`}><span className={`event-type event-${String(event.type || "system").toLowerCase()}`}>{event.type || "system"}</span><div className="event-content"><strong>{event.flight || event.gate ? [event.flight, event.gate ? `Gate ${event.gate}` : ""].filter(Boolean).join(" · ") : "System event"}</strong><p>{event.message || "No message supplied"}</p></div><div className="event-meta"><StatusTag status={event.status || "INFO"} /><time>{formatTime(event.timestamp)}</time></div></div>)}</div>}</section>
        </main>
      )}

      <footer className="footer"><span>SKYPORT © 2026</span><span>AIRPORT GROUND OPERATIONS</span><span>POWERED BY LINUX IPC</span></footer>
    </div>
  );
}

function Metric({ label, value, detail }) {
  return <div className="metric-card"><span>{label}</span><strong>{value}</strong><small>{detail}</small></div>;
}

function EmptyState({ connected, text }) {
  return <div className="empty-state"><span className={`status-dot ${connected ? "" : "status-dot-off"}`} /><p>{text}</p></div>;
}

function StatusTag({ status }) {
  const normalized = String(status || "INFO").toLowerCase().replace(/[^a-z0-9]+/g, "-");
  return <span className={`status-tag tag-${normalized}`}>{status || "INFO"}</span>;
}

export default App;
