from flask import Flask, jsonify, render_template_string
import sqlite3

DB_PATH = "/opt/bosporus/bosporus.db"
app = Flask(__name__)

PAGE = """
<!DOCTYPE html>
<html>
<head>
  <title>Bosphorus Dashboard</title>
  <script src="https://cdn.jsdelivr.net/npm/chart.js"></script>
</head>
<body>
  <h1>Bosphorus — Sensor Readings</h1>
  <canvas id="chart" width="800" height="400"></canvas>
  <script>
    async function loadData() {
      const res = await fetch('/api/readings');
      const data = await res.json();
      const labels = data.map(r => new Date(r.timestamp * 1000).toLocaleTimeString());
      const temps = data.map(r => r.temperature);
      const hums = data.map(r => r.humidity);

      new Chart(document.getElementById('chart'), {
        type: 'line',
        data: {
          labels: labels,
          datasets: [
            { label: 'Temperature (°C)', data: temps, borderColor: 'red', fill: false },
            { label: 'Humidity (%)', data: hums, borderColor: 'blue', fill: false }
          ]
        }
      });
    }
    loadData();
  </script>
</body>
</html>
"""

@app.route("/")
def index():
    return render_template_string(PAGE)

@app.route("/api/readings")
def readings():
    conn = sqlite3.connect(DB_PATH)
    conn.row_factory = sqlite3.Row
    rows = conn.execute(
        "SELECT timestamp, temperature, humidity FROM readings ORDER BY id DESC LIMIT 50"
    ).fetchall()
    conn.close()
    return jsonify([dict(r) for r in reversed(rows)])

if __name__ == "__main__":
    app.run(host="0.0.0.0", port=5000)