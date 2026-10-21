const express = require("express");
const { execFile } = require("child_process");
const app = express();

app.use(express.static("./public"));

// ---- Low-level: call the course-provided ./C++/gpio binary ----
// Slide 19 shows: sudo ./C++/gpio <LED> <POWER>
// Run it by hand first to confirm what values it expects (e.g. LED 1-4, POWER 0/1).
// Easiest: start this server with `sudo node server.js` so no password prompt appears.
function setLED(led, power) {
  const p = execFile("sudo", ["./C++/gpio", String(led), String(power)]);
  p.stdout.on("data", (d) => console.log(`stdout: ${d}`));
  p.stderr.on("data", (d) => console.error(`stderr: ${d}`));
}

const LEDS = [1, 2, 3, 4];
let timers = []; // active blink / running-light timers
let direction = 1; // 1 = LED1->LED4, -1 = reverse

function stopAll() {
  timers.forEach(clearInterval);
  timers = [];
}
function allOff() {
  LEDS.forEach((l) => setLED(l, 0));
}

// ---- Item 2: manual on/off ----
// GET /index?LED=1,3&POWER=1
app.get("/index", (req, res) => {
  stopAll();
  const leds = String(req.query.LED || "").split(",").filter(Boolean);
  const power = req.query.POWER === "1" ? 1 : 0;
  leds.forEach((l) => setLED(l, power));
  res.send(leds.map((l) => `LED${l} ${power ? "ON" : "OFF"}`).join("<br>") || "No LED selected");
});

// ---- Item 3: Mode_Shine — alternate two groups N times ----
// GET /shine?times=5
app.get("/shine", (req, res) => {
  stopAll();
  allOff();
  const times = Math.max(0, parseInt(req.query.times, 10) || 0);
  let count = 0;
  const groupA = [1, 3];
  const groupB = [2, 4];
  const t = setInterval(() => {
    if (count >= times * 2) {
      clearInterval(t);
      allOff();
      return;
    }
    const on = count % 2 === 0 ? groupA : groupB;
    const off = count % 2 === 0 ? groupB : groupA;
    off.forEach((l) => setLED(l, 0));
    on.forEach((l) => setLED(l, 1));
    count++;
  }, 500);
  timers.push(t);
  res.send(`Mode_Shine_Times: ${times}`);
});

// ---- Item 4: running light ----
let pos = 0;
app.get("/running/on", (req, res) => {
  stopAll();
  allOff();
  const t = setInterval(() => {
    LEDS.forEach((l) => setLED(l, 0));
    setLED(LEDS[pos], 1);
    pos = (pos + direction + LEDS.length) % LEDS.length;
  }, 400);
  timers.push(t);
  res.send("Running Light ON");
});

app.get("/running/off", (req, res) => {
  stopAll();
  allOff();
  res.send("Running Light OFF");
});

// Flips direction on the fly; the interval keeps running with the new direction
app.get("/running/reverse", (req, res) => {
  direction = -direction;
  res.send(`Direction: ${direction === 1 ? "LED1→LED4" : "LED4→LED1"}`);
});

const PORT = process.env.PORT || 8080;
app.listen(PORT, () => console.log(`Server is running on port ${PORT}.`));
