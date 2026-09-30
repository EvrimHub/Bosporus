# Progress log – Bosporus

This dated log documents the process and the key steps, and the progress of the project along with its artifacts.

---

## *10.07.2026* – Phase 1: Sensor node soldering & first readings

### What I did

**HW Setup**

Soldered the two 15-pin header strips onto the Arduino Nano ESP32 (first solder joints
in a while — a bit rough around the edges, but electrically sound). Wired the DHT22 to
the board on a breadboard (VCC → 3V3, DATA → D2, GND → GND).

**SW Setup**
- Set up PlatformIO in VS Code
- Added the DHT library
- Added the code to `src/main.cpp`
- Flashed a first build to read temperature and humidity

**Artifacts**

![Hardware Setup](images/setup-esp32-with-DHT22.jpeg)

Serial monitor output, confirming the sensor is being read correctly:

![Serial monitor output](images/sensor-data-output.png)

**What went wrong / what I learned**

- First soldering attempt in a long time — needed a refresher on heating the joint (not
  the solder) before feeding solder in.
- Initial header pins were too thick for the breadboard when inserted at an angle;
  fixed by inserting straight down.
- Hit a `'Serial' does not name a type` compile error caused by a mismatched brace, not
  an actual `Serial` problem — a good reminder that C++ error messages sometimes point
  at the *symptom* location, not the *cause* location.

**Result**

✅ End-to-end proof that the sensor node hardware and firmware work: soldering → wiring
→ code → live sensor readings.

---

## *16.07.2026 – 05.08.2026* – Phase 2: Embedded Linux gateway

### What I did

**HW Setup**

**Computer**
MacBook Air, Apple M3

**Raspberry Pi 4 Model B**

For this project, I wanted to learn embedded Linux in a realistic context — and IoT
projects are a natural fit for that, because they almost always call for a gateway in
the design. Choosing to build one gave me a concrete reason to get hands-on with
embedded Linux, rather than learning it in the abstract.

A gateway earns its place in an IoT architecture for three main reasons:

1. **Centralized connectivity** — sensors don't each need their own network hardware;
   the gateway handles that once, for all of them. The Raspberry Pi covers this well,
   with WiFi, Ethernet (useful as a stable fallback), and Bluetooth built in.
2. **Centralized security** — patching, firewalling, and access control happen on one
   device instead of being replicated (and potentially neglected) across every sensor
   node — which is both more secure and cheaper to maintain at scale.
3. **A proper OS for real services** — running things like Mosquitto, a database, and
   Grafana requires a full filesystem, networking stack, and process management —
   capabilities a microcontroller doesn't have, but a real OS does.

I chose the Raspberry Pi specifically as my learning vehicle for embedded Linux.
Compared to alternatives like the Orange Pi or BeagleBone Black, it has the strongest
documentation and the largest community — which matters most when you're learning,
since it means more tutorials and faster troubleshooting when something goes wrong.

**MicroSD:** Samsung EVO Plus (128 GB, microSDXC, U3, UHS-I)

The Raspberry Pi has no internal storage of its own — no built-in flash, no eMMC. The
microSD card is therefore the Pi's only storage device, holding the OS, all files, and
logs for as long as it runs. Every time the Pi boots, reads a file, or writes a log, it
does so on this card.

**Card Reader:** StarTech USB 3.0 card reader with USB-C

I ordered this card reader with USB-C so that I can read the SD card with my MacBook
Air, because my MacBook Air does not have a built-in interface capable of reading SD
cards.

**Ethernet adapter:** Belkin USB-C to Gigabit Ethernet Adapter

The Ethernet adapter is the backup for a stable connection if WiFi or mDNS doesn't
work. `bosporus.local` resolves to an IP address via mDNS. If this fails, I need to
connect by IP address directly, using a wired connection, which is more predictable.

**SW Setup**

### Step 1: Prove the Pi works with standard Raspberry Pi OS

*Installation and configuration of Raspberry Pi Imager*

I downloaded `imager_2.0.10.dmg` from raspberrypi.com/software and installed it into
the Applications folder on my MacBook Air. I then launched the Imager and used the
following configuration:
- Selected **Raspberry Pi 4**
- Selected **Raspberry Pi OS (other)**, then **Raspberry Pi OS Lite (64-bit)**
- Confirmed the SD card size — 119.4 GB
- Ticked "Exclude system drives" to prevent accidentally erasing my Mac's startup disk
- Set **Hostname:** `bosporus`, **Time zone:** Europe/Zurich, **Keyboard layout:**
  Swiss German (ch), then a username and password, and enabled **SSH**

*Writing the standard Linux OS onto the SD card and first boot*
- Inserted the SD card into the Raspberry Pi's card slot
- Powered the Raspberry Pi — RED LED lit solid, GREEN LED blinking (indicating active boot)
- Attempted to access the Raspberry Pi with `ssh boncuk@bosporus.local`

### Step 2: Configure and build the Buildroot image, flash it onto the Raspberry Pi

Plan: Docker Desktop → Buildroot source → configure → build → flash → boot my own image.

**Installing Docker Desktop**

Docker is the environment used to build the Buildroot image, since Buildroot requires
a Linux machine and my Mac isn't one. Docker Desktop works by running a small Linux
virtual machine in the background — it's just a convenient way to get a Linux computer
on my Mac. Professional embedded workflows run on actual Linux machines; building
Linux images belongs on Linux. Docker-on-Mac is a practical personal-development
bridge to get there without owning a Linux machine, not a professional release process.

- Installed Docker Desktop from docker.com/products/docker-desktop → "Download for Mac – Apple Silicon"
- Installed `Docker.dmg` into Applications
- Confirmed with `docker run --rm hello-world` → resulted in "Hello from Docker!"

**Get the Buildroot source**

This is done inside a Linux container, not directly on my Mac:

```bash
docker run -it --rm -v $(pwd)/buildroot-workspace:/home/builder -w /home/builder ubuntu bash
```
- Starts a fresh Ubuntu Linux container
- `-v $(pwd)/buildroot-workspace:/home/builder` creates a **bind mount** — a two-way
  link between `buildroot-workspace` on my Mac's real filesystem and `/home/builder`
  inside the Linux container, so files persist even after the container closes
- `-w /home/builder` drops me into a bash shell inside Linux, in that directory

Installed the build dependencies, since Buildroot needs a fairly long list of standard
Linux build tools:
```bash
apt-get update && apt-get install -y build-essential git wget cpio unzip rsync bc python3 libncurses-dev
```

Cloned Buildroot itself:
```bash
git clone https://github.com/buildroot/buildroot.git
```
`git clone` downloads a complete copy of a Git repository. Buildroot's GitHub repo *is*
Buildroot — a large collection of Makefiles, package recipes, board-specific
defconfigs, and Kconfig menu definitions. This pulled the actual Buildroot source tree.
(Note: this GitHub repo is a read-only mirror — Buildroot's actual development happens
on GitLab, via a mailing-list patch review process, not GitHub pull requests.)

**Configure Buildroot**
- `make raspberrypi4_defconfig` — copies a pre-made template configuration into the
  project as the active `.config`. For the Pi 4, this template already contains the
  correct CPU architecture, bootloader, kernel version, and device tree — my starting
  point, pre-filled with sensible settings for this exact board.
- `make menuconfig` — opens a text-based, keyboard-navigated menu. Added the four
  packages the gateway needs on top of the baseline: Mosquitto, Dropbear + OpenSSH,
  Python3, and SQLite.
  - Target packages → Networking applications → `mosquitto`, `dropbear`, `openssh` (Space to select each)
  - Target packages → Interpreter languages and scripting → `python3`
  - Target packages → Libraries → Database → `sqlite`
  - Exit and save (Esc, Esc) — writes the selections into `.config`

**Build**

Ran `make` — first attempt failed almost immediately:
```
You must install '/usr/bin/file' on your build machine
```
Fixed with `apt-get install -y file`, then reran `make`.

Second attempt got much further (downloading and cross-compiling the toolchain,
kernel, bootloader, and every selected package — over an hour of work), then failed
differently:
```
chmod: changing permissions of '.../host-m4-1.4.21/bootstrap': Permission denied
```
This is a known Docker Desktop-on-Mac limitation: Docker doesn't talk to a bind-mounted
folder directly the way real Linux would. It goes through a translation layer called
**VirtioFS** that bridges macOS's filesystem to the Linux container, and that layer has
a specific bug where files extracted with certain permission bits can't be `chmod`'d
afterward — even as root. Buildroot does this exact "extract, then chmod" pattern for
every package, so this was going to keep recurring.

**Fix:** stopped using a bind mount, switched to a Docker-managed **named volume**
instead (which doesn't go through that same host-translation layer):
```bash
docker volume create buildroot-vol
docker run -it --name buildroot-builder -v buildroot-vol:/home/builder -w /home/builder ubuntu:24.04 bash
```
Redid the setup in this fresh container (dependencies, clone, defconfig, menuconfig
with the same four packages), then ran `make` again — completed successfully this time.

Confirmed the build output:
```bash
ls -lh output/images/
```
→ `sdcard.img` (153M), `zImage`, `rootfs.ext2`/`rootfs.ext4`, four `bcm2711-*.dtb`
variants, `boot.vfat`, `genimage.cfg` — every piece Buildroot is supposed to produce.

**Checking login before flashing — this saved me from being locked out**

Since a named volume isn't directly browsable from Finder, getting the file onto my
Mac needs an explicit copy step:
```bash
docker cp buildroot-builder:/home/builder/buildroot/output/images/sdcard.img ~/bosporus-sdcard.img
```
(First attempt at this failed with `bash: docker: command not found` — I'd
accidentally run it *inside* the container. `docker` is a Mac-side command; it manages
containers from the outside, so this has to run from a normal Mac terminal, not from
within the container itself.)

Before flashing, checked whether login would even be possible:
```bash
cat output/target/etc/shadow | head -3
```
```
root::::::::
daemon:*:::::::
bin:*:::::::
```
The empty second field on `root` confirmed no password was set at all. Dropbear (the
SSH server) specifically refuses network login with a blank password even though the
account itself has none — so flashing as-is would have locked me out with no serial
console as a fallback. Fixed via:
```
make menuconfig → System configuration → Root password → make
```

**Flashing and first boot of the custom image**
- Got the updated image onto my Mac with `docker cp` (run correctly this time, from a
  normal Mac terminal) and confirmed with `ls -lh ~/bosporus-sdcard.img`
- Raspberry Pi Imager → Choose OS → scrolled to the bottom → **"Use custom"** →
  selected `~/bosporus-sdcard.img`
- Choose Storage → SD card → Write
- Inserted the SD card into the Pi, then powered it on
- LEDs: RED solid, GREEN blinking every 3–4 seconds — a slow, regular "heartbeat"
  pattern, different from the rapid flickering seen during standard-OS boot, suggesting
  the kernel had fully booted (heartbeat LED only activates once Linux is running)

**Connecting — the actual troubleshooting stretch**

`ssh root@bosporus.local` hung indefinitely. Diagnosed as: the custom image has no
WiFi configured at all — unlike Raspberry Pi Imager, `menuconfig` never asks for WiFi
credentials, and I never added `wpa_supplicant` or any WiFi firmware package. The Pi
had booted but never joined the network.

First fix attempt: connected the Pi directly to my Mac via the Ethernet adapter —
still no connection. Cause: no DHCP server in a direct Pi-to-Mac link (a router
normally provides DHCP; my Mac doesn't). **Actual fix:** connected the Pi's Ethernet
port to the router instead, which got it a real IP via DHCP: `192.168.1.169`.

```bash
ssh root@192.168.1.169
```
connected and prompted for a password — entering the password I'd set (`bosporus74`)
resulted in `Connection closed by 192.168.1.169 port 22`. Retyped manually (not pasted)
— got `Permission denied, please try again` three times in a row.

Debugged systematically rather than continuing to guess:
- `grep ROOT_PASSWD .config` → confirmed `bosporus74` was genuinely saved
- `cat output/target/etc/shadow | head -1` → confirmed a real password hash existed
- `cat output/target/etc/init.d/S50dropbear` → checked for a `-w` flag (which would
  disable root login entirely, regardless of password) — confirmed absent

Reset to a deliberately simple password (`test1234`) to rule out a typing/Caps-Lock
issue, rebuilt, re-copied, reflashed. Still got "Permission denied." Checked
`ls -lh ~/bosporus-sdcard.img` — confirmed the image file itself was genuinely fresh.

Noticed the Pi's IP address hadn't changed across reflashes, and initially treated that
as suspicious — but the IP is assigned by the router based on the Pi's **MAC address**,
not its software, so an unchanged IP proves nothing about which image is actually
running.

Final `ssh root@192.168.1.169` attempt triggered:
```
WARNING: REMOTE HOST IDENTIFICATION HAS CHANGED!
```
Correctly recognized this as **expected**, not an attack: a fresh filesystem generates
fresh SSH host keys, so this was actual proof the newest image was finally running.
Cleared the stale entry with `ssh-keygen -R 192.168.1.169`, reconnected, accepted the
new host key, entered `test1234` —

**successful login to my own custom-built Buildroot Linux system.**

**Artifacts**

![Custom Buildroot image build output](images/buildroot-output-images.png)

![Successful SSH login to custom image](images/buildroot-ssh-login.png)

**What went wrong / what I learned**

- While flashing the standard Raspberry Pi OS: SSH was refused because the server
  wasn't reachable — fixed with the boot-partition `touch .../ssh` trick. A password
  paste attempt also caused a silent connection failure; typing manually resolved it.
- `make` failed first on a missing `file` package — fixed with `apt-get install -y file`.
- `make` failed a second time on `chmod: Permission denied` during package extraction —
  a known Docker Desktop-on-Mac VirtioFS bind-mount bug. Fixed by switching from a bind
  mount to a Docker-managed named volume.
- `docker cp` doesn't run inside a container — it's a Mac-side command for moving files
  across the container boundary, and has to run from a normal Mac terminal.
- A blank root password blocks SSH login entirely (Dropbear refuses blank-password
  logins over the network), even though the same account would work fine on a local
  console — worth checking `/etc/shadow` **before** flashing, not after being locked out.
- Buildroot's `menuconfig` has no WiFi setup step — unlike Raspberry Pi Imager, WiFi
  has to be deliberately configured, or wired Ethernet has to be used instead.
- A direct Ethernet cable between two devices doesn't give either one an IP address
  without a DHCP server somewhere in the loop (normally the router's job).
- An SD card's IP address is tied to the Pi's hardware (MAC address), not to whatever
  OS/image happens to be on the card — it does not change across reflashes, and is not
  evidence of which image is currently running.
- A "REMOTE HOST IDENTIFICATION HAS CHANGED" SSH warning isn't automatically suspicious
  — a freshly flashed system genuinely does generate new host keys, and `ssh-keygen -R`
  is the correct, safe way to clear the outdated entry once you know why it changed.

**Result**

✅ Custom Buildroot image built from source and successfully booted on the Raspberry Pi 4,
with SSH access confirmed via a manually set root password — the core Phase 2
milestone from the project plan: *"Custom Linux image boots on the Pi."*

**Verification**

`uname -a` prints the kernel name, kernel release version, hostname, build
date/time, CPU architecture, and operating system family — confirms this is
genuinely my own build, not standard Raspberry Pi OS. `which <program>` prints
the exact path where a program lives on disk — no output would mean it isn't
installed; a real path is proof the package made it into the image.

```
# uname -a
Linux buildroot 6.12.61-v7l #1 SMP Tue Jul 28 18:07:57 CEST 2026 armv7l GNU/Linux
# which mosquitto
/usr/sbin/mosquitto
# which python3
/usr/bin/python3
# which sqlite3
/usr/bin/sqlite3
```

✅ All three packages confirmed present and correctly installed.

---

## *11.08.2026 – 13.08.2026* – Phase 3: Integration

### What I did

**Understanding Mosquitto**

Mosquitto is one specific piece of software that implements the MQTT (Message
Queuing Telemetry Transport) protocol. It's lightweight and suitable for use
on everything from low-power single-board computers to full servers, sending
small messages between devices.

**Confirming the broker is running**

Tried starting it manually:
```bash
mosquitto -v &
```
This failed with "Address already in use" — turned out Mosquitto was already
running automatically, started at boot by Buildroot's init script:
```bash
ps | grep mosquitto
```
```
176 nobody   /usr/sbin/mosquitto -c /etc/mosquitto/mosquitto.conf
```

**Checking whether it accepts connections from other devices**

```bash
cat /etc/mosquitto/mosquitto.conf
```
I was specifically looking for a `listener` line and an `allow_anonymous`
setting — these determine whether Mosquitto would accept a connection from
the ESP32 at all, or only from processes running locally on the Pi itself.

```bash
grep -E '^(listener|allow_anonymous)' /etc/mosquitto/mosquitto.conf
```
Empty result — no active `listener` or `allow_anonymous` line exists anywhere
in the file, only commented-out defaults and documentation. This matched the
"Starting in local only mode" warning from the manual start attempt: without
an explicit listener, Mosquitto only accepts local connections.

**Opening it up to the network**

First attempt: created a separate config file at `/etc/mosquitto/conf.d/bosporus.conf`
with a `listener 1883 0.0.0.0` and `allow_anonymous true` line, and restarted
Mosquitto pointing at it directly. This worked, but only as a one-off — the
file wasn't being loaded automatically (the main config's `include_dir` option
that would pull in `conf.d/*.conf` files is commented out by default).

**Fix:** appended the same two lines directly to the end of the main config
file instead, so Buildroot's existing boot script (which already points at
`mosquitto.conf`) picks them up automatically:
```bash
cat >> /etc/mosquitto/mosquitto.conf << 'EOF'

listener 1883 0.0.0.0
allow_anonymous true
EOF
```
Restarted and confirmed:
```bash
ps | grep mosquitto
kill <PID>
mosquitto -d -c /etc/mosquitto/mosquitto.conf
netstat -tuln | grep 1883
```
```
tcp        0      0 0.0.0.0:1883            0.0.0.0:*               LISTEN
```
`0.0.0.0:1883` confirms Mosquitto is listening on all network interfaces —
reachable from other devices on the network, including the ESP32.

**Testing from the Mac**

Installed the MQTT command-line client tools (not a broker — just `mosquitto_pub`
and `mosquitto_sub`, for testing):
```bash
brew install mosquitto
```
In one Mac terminal, subscribed to a test topic; in a second tab, published a message:
```bash
mosquitto_sub -h 192.168.1.169 -t test/topic -v
mosquitto_pub -h 192.168.1.169 -t test/topic -m "hello from my mac"
```
The subscribing terminal immediately printed `test/topic hello from my mac` —
full proof the broker is reachable and working correctly over the network,
completely independent of the ESP32.

**Confirming the config survives a reboot**

```bash
reboot
```
After reconnecting via SSH, checked without starting anything manually:
```bash
ps | grep mosquitto
netstat -tuln | grep 1883
```
```
180 nobody   /usr/sbin/mosquitto -c /etc/mosquitto/mosquitto.conf
tcp        0      0 0.0.0.0:1883            0.0.0.0:*               LISTEN
```
Mosquitto came up automatically, already bound to `0.0.0.0:1883`, with no
manual intervention — confirms the config change is genuinely saved on the SD
card and correctly wired into the boot sequence, not just a runtime fix that
would be lost on the next power cycle.

**Result**

✅ Mosquitto broker running on the Pi, reachable from other devices on the
network, auto-starting correctly configured on every boot. Publish/subscribe
confirmed working end-to-end from the Mac.

**Next steps**

- Update the ESP32 firmware to connect to WiFi and publish real sensor readings via MQTT
- Write the gateway-side Python script that subscribes to the sensor topic and writes readings into SQLite

**Update ESP32 Firmware with WiFi Connection**
- Creation of `secrets.h` file in `include/`, which contains `WIFI_SSID` and `WIFI_PASSWORD`.
```cpp
#pragma once
#define WIFI_SSID "wlan_name"
#define WIFI_PASSWORD "wlan_passwort"
```

- This file is added to `.gitignore`.

```
.pio
.vscode/.browse.c_cpp.db*
.vscode/c_cpp_properties.json
.vscode/launch.json
.vscode/ipch
include/secrets.h
```

This way, the file exists on my machine and compiles fine locally. But, Git will never track or upload it.
- Adding MQTT library to `platformio.ini`

```ini
lib_deps =
    adafruit/DHT sensor library@^1.4.6
    adafruit/Adafruit Unified Sensor@^1.1.14
    knolleary/PubSubClient@^2.8
```

- Update `src/main.cpp`
```cpp
#include <Arduino.h>
#include <DHT.h>
#include <WiFi.h>
#include <PubSubClient.h> // MQTT client library publishes readings to Pi's Mosquitto broker
#include "secrets.h"

#define DHTPIN 2
#define DHTTYPE DHT22

const char* MQTT_BROKER = "192.168.1.169";   //  Pi's IP
const int   MQTT_PORT   = 1883;
const char* MQTT_TOPIC  = "sensor/room1/climate";  // matches architecture.md
const char* MQTT_CLIENT_ID = "esp32-sensor-node";

DHT dht(DHTPIN, DHTTYPE);
WiFiClient espClient;
PubSubClient mqttClient(espClient);

void connectWiFi() {
  Serial.print("Connecting to WiFi");
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();
  Serial.print("WiFi connected, IP address: ");
  Serial.println(WiFi.localIP());
}

void connectMQTT() {
  while (!mqttClient.connected()) {
    Serial.print("Connecting to MQTT broker...");
    if (mqttClient.connect(MQTT_CLIENT_ID)) {
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" - retrying in 5s");
      delay(5000);
    }
  }
}

void setup() {
  Serial.begin(115200);
  dht.begin();
  connectWiFi();
  mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) connectWiFi();
  if (!mqttClient.connected()) connectMQTT();
  mqttClient.loop();

  delay(2000);

  float humidity = dht.readHumidity();
  float tempC = dht.readTemperature();

  if (isnan(humidity) || isnan(tempC)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }

  char payload[64];
  snprintf(payload, sizeof(payload), "{\"temperature\": %.1f, \"humidity\": %.1f}", tempC, humidity);

  Serial.print("Publishing: ");
  Serial.println(payload);
  mqttClient.publish(MQTT_TOPIC, payload);
}
```
- Compiled new code in main.cpp and got this:
```
--Terminal on /dev/cu.usbmodem206EF13285D82 | 115200 8-N-1
--- Available filters and text transformations: debug, default, direct, esp32_exception_decoder, hexlify, log2file, nocontrol, printable, send_on_enter, time
--- More details at https://bit.ly/pio-monitor-filters
--- Quit: Ctrl+C | Menu: Ctrl+T | Help: Ctrl+T followed by Ctrl+H
.......................................................................
```
This confirms that the board connected, detected and running the new firmware. The dots pattern is connectWiFi() function's Serial.print(".") loop, which only appears in the new code. But, it stuck at the WiFi connection stage and never getting past it.

**Debugging**

- Checked the Wifi name and password -> correct. 
- Checking WiFi broadcasting Frequency because ESP32 support only 2.4 Ghz. I have first did a test with my iPhone hotspot. I have also added diagnostics instead of dots.

```
void connectWiFi() {
  Serial.print("Connecting to WiFi: ");
  Serial.println(WIFI_SSID);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print("status=");
    Serial.println(WiFi.status());
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.print("WiFi connected, IP address: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("WiFi connection FAILED after 20 attempts.");
  }
}
```
Then build, upload and watch the Serial Monitor. With the diagnostic code Status=N will be print out every hald second instead of dots forever. Meaning of the numbers:
```
Code	Meaning
0	Idle — hasn't really started trying yet
1	No SSID available — the ESP32 can't see a network with that exact name at all
4	Connect failed — network found, but authentication failed (wrong password, most likely)
6	Disconnected
```
- compiled, run and upload the new code: `status = 6`.
- Number 6 can have a few different causes. Update the code: Explicitly set station mode and clear any stale state before connecting
```cpp
void connectWiFi() {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect(true);
  delay(100);
  // ...rest unchanged from the version above...
}
```
- build, run and upload with the following output:
```
--- Terminal on /dev/cu.usbmodem206EF13285D82 | 115200 8-N-1
--- Available filters and text transformations: debug, default, direct, esp32_exception_decoder, hexlify, log2file, nocontrol, printable, send_on_enter, time
--- More details at https://bit.ly/pio-monitor-filters
--- Quit: Ctrl+C | Menu: Ctrl+T | Help: Ctrl+T followed by Ctrl+H
status=6
status=6
status=6
status=6
status=6
status=6
status=6
status=6
status=6
status=6
status=6
status=6
status=6
status=6
status=6
status=6
status=6
status=6
status=6
WiFi connection FAILED after 20 attempts.
Connecting to WiFiiPhone
status=6
status=6
```
- Added a length check to catch invisible characters to check the correctness of wifi_ssid and wifi_password.
```
Serial.print("SSID length: ");
Serial.println(strlen(WIFI_SSID));
Serial.print("Password length: ");
Serial.println(strlen(WIFI_PASSWORD));
```
- After build, run, upload, the board has been connected to iphone wifi: status=3 with IP address 172.20.10.5.
- After this temporarly test I have switched to home Wifi credentials. I only enabled 2.4 GHz and tested again with the result:
```
Executing task: platformio device monitor --- Terminal on /dev/cu.usbmodem206EF13285D82 | 115200 8-N-1 --- Available filters and text transformations: debug, default, direct, esp32_exception_decoder, hexlify, log2file, nocontrol, printable, send_on_enter, time --- More details at https://bit.ly/pio-monitor-filters --- Quit: Ctrl+C | Menu: Ctrl+T | Help: Ctrl+T followed by Ctrl+H status=6 status=6 status=6 status=1 status=1 status=1 status=1 status=1 status=1 status=1 status=1 status=1 status=1 status=1 status=1 status=1 status=1 status=1 status=1 WiFi connection FAILED after 20 attempts. Connecting to WiFiZyxel_CF21 SSID length: 10 Password length: 10 status=1 status=1 status=1 status=1 status=1 status=1 status=1 status=1 status=1 status=1 status=1 status=1 status=1 status=1 status=1 status=1 status=1 status=1 status=1 status=1 WiFi connection FAILED after 20 attempts. Connecting to MQTT broker...Failed to connect to MQTT broker, rc=-2 Retrying in 5 seconds... Connecting to MQTT broker...Failed to connect to MQTT broker, rc=-2 Retrying in 5 seconds... Connecting to MQTT broker...Failed to connect to MQTT broker, rc=-2 Retrying in 5 seconds... Connecting to MQTT broker...Failed to connect to MQTT broker, rc=-2 Retrying in 5 seconds...
```
Status=1 means that ESP32 cannot find my home wifi router. After this I have checked the router config and saw that 2.4 GHz and 5GHz were both enabled together. After that I have deactivated Mesh so that only 2.4 Ghz was active. Rebuild, upload with the same result status = 1. After this result I have checked the WiFi channel because ESP 32 scans wifi channels from 1-11. It was set to auto and I set it to 6. After rebuild the result was the same. To deepen the debugging I added to the code WiFi scan:

```
void scanNetworks() {
  Serial.println("Scanning for WiFi networks...");
  int n = WiFi.scanNetworks();
  if (n == 0) {
    Serial.println("No networks found at all.");
  } else {
    Serial.print(n);
    Serial.println(" networks found:");
    for (int i = 0; i < n; i++) {
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (RSSI: ");
      Serial.print(WiFi.RSSI(i));
      Serial.println(")");
    }
  }
}
```
then call it in setup() before connectWiFi().
```
void setup() {
  Serial.begin(115200);
  dht.begin();
  scanNetworks();
  connectWiFi();
  mqttClient.setServer(MQTT_BROKER, MQTT_PORT);
}
```
- rebuild and upload. The result was:

```
--- Terminal on /dev/cu.usbmodem206EF13285D82 | 115200 8-N-1
--- Available filters and text transformations: debug, default, direct, esp32_exception_decoder, hexlify, log2file, nocontrol, printable, send_on_enter, time
--- More details at https://bit.ly/pio-monitor-filters
--- Quit: Ctrl+C | Menu: Ctrl+T | Help: Ctrl+T followed by Ctrl+H
No networks found at all.
Connecting to WiFiZyxel_CF21
SSID length: 10
Password length: 10
status=6
status=6
status=6
status=6
status=6
status=6

.......
```
This shows that zero networks found at all. In a normal home environment a scan should find at least few networks. The most likely cause is that WiFi radio wasn't in the right mode yet when the scan ran. Update the scanNetworks function with station mode and add some delay. 

```
void scanNetworks() {
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(500);

  Serial.println("Scanning for WiFi networks...");
  int n = WiFi.scanNetworks();
  Serial.print("Scan returned: ");
  Serial.println(n);

  if (n <= 0) {
    Serial.println("No networks found at all.");
  } else {
    for (int i = 0; i < n; i++) {
      Serial.print(i + 1);
      Serial.print(": ");
      Serial.print(WiFi.SSID(i));
      Serial.print(" (RSSI: ");
      Serial.print(WiFi.RSSI(i));
      Serial.println(")");
    }
  }
}
```
Rebuild, upload with the same result that no networks found at all. Then I just run a only WiFi scan, nothing else to check whether in the rest of the code somehow interfering with this code by creating a new project wifi-scan-test. 

```
#include <Arduino.h>
#include <WiFi.h>

void setup() {
  Serial.begin(115200);
  delay(1000);
  WiFi.mode(WIFI_STA);
  delay(500);
  int n = WiFi.scanNetworks();
  Serial.print("Networks found: ");
  Serial.println(n);
  for (int i = 0; i < n; i++) {
    Serial.println(WiFi.SSID(i));
  }
}

void loop() {}
```
After running this code, only one network is found, my printer. This result shows that ESP32 Wifi radio and antenna are genuinely working. It scan and detect a network. The earlier zero networks result weren't hardware defect or library conflict after all. That also means my home router isnt visible frome ESP32. The likely explanation is the signal range, not a bug at all. After this reasoning I have just sit next to the router and scanned again the network with this result:

```
Executing task in folder wifi-scan-test: platformio device monitor 
--- Terminal on /dev/cu.usbmodem206EF13285D82 | 9600 8-N-1
--- Available filters and text transformations: debug, default, direct, esp32_exception_decoder, hexlify, log2file, nocontrol, printable, send_on_enter, time
--- More details at https://bit.ly/pio-monitor-filters
--- Quit: Ctrl+C | Menu: Ctrl+T | Help: Ctrl+T followed by Ctrl+H
Networks found: 2
DIRECT-94-HP M140 LaserJet
Zyxel_CF21
```
This confirmed cleanly the signal-range issue the whole time and not a bug, not a router misconfiguration and not a hardware defect. This means the sensor node needs to be reasonably close to the router. Then I have run my bosporus project and "voilà!". It connected to home WiFi and to MQTT brocker. I could not read the DHT Sensor readings because I did not correct wired up. After fixing that issue, the result:

```
Executing task: platformio device monitor --- Terminal on /dev/cu.usbmodem206EF13285D82 | 115200 8-N-1 --- Available filters and text transformations: debug, default, direct, esp32_exception_decoder, hexlify, log2file, nocontrol, printable, send_on_enter, time --- More details at https://bit.ly/pio-monitor-filters --- Quit: Ctrl+C | Menu: Ctrl+T | Help: Ctrl+T followed by Ctrl+H Scan returned: 1 1: Zyxel_CF21 (RSSI: -92) Connecting to WiFiZyxel_CF21 SSID length: 10 Password length: 10 status=6 status=3 WiFi connected, IP address: 192.168.1.195 Connecting to MQTT broker...Connected to MQTT broker Publishing: {"temperature": 27.4, "humidity": 36.5} Publishing: {"temperature": 27.4, "humidity": 36.4} Publishing: {"temperature": 27.4, "humidity": 36.4} Publishing: {"temperature": 27.4, "humidity": 36.3} Publishing: {"temperature": 27.4, "humidity": 36.3} Publishing: {"temperature": 27.4, "humidity": 36.3} Publishing: {"temperature": 27.4, "humidity": 36.3} Publishing: {"temperature": 27.4, "humidity": 36.4} Publishing: {"temperature": 27.4, "humidity": 36.4} Publishing: {"temperature": 27.4, "humidity": 36.5} Publishing: {"temperature": 27.4, "humidity": 36.5} Publishing: {"temperature": 27.4, "humidity": 36.5} Publishing: {"temperature": 27.4, "humidity": 36.6} Publishing: 
```
Adding that measured RSSI of -92 dBm at ~5m line-of-sight — weaker than typical for this distance, likely a combination of the board's small embedded antenna and local RF conditions. Confirmed functional despite the weak signal; would investigate further (antenna placement, dedicated access point) in a production deployment.

**Adding an MQTT library for Python via Buildroot**
Python's standard library doesn't include MQTT support. I have to add python-paho-mqtt package through menuconfig. The rebuild the image, copy to the SD card and flash onto the Pi. 

The following script is the application code for the gateway. 

```
import json
import sqlite3
import time
import paho.mqtt.client as mqtt

MQTT_BROKER = "localhost"  # script runs ON the Pi, same machine as the broker
MQTT_PORT = 1883
MQTT_TOPIC = "sensor/room1/climate"

DB_PATH = "/root/bosporus.db"

def init_db():
    conn = sqlite3.connect(DB_PATH)
    conn.execute("""
        CREATE TABLE IF NOT EXISTS readings (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            timestamp INTEGER NOT NULL,
            temperature REAL,
            humidity REAL
        )
    """)
    conn.commit()
    conn.close()

def on_connect(client, userdata, flags, rc):
    print(f"Connected to broker, rc={rc}")
    client.subscribe(MQTT_TOPIC)

def on_message(client, userdata, msg):
    try:
        payload = json.loads(msg.payload.decode())
        temperature = payload.get("temperature")
        humidity = payload.get("humidity")

        conn = sqlite3.connect(DB_PATH)
        conn.execute(
            "INSERT INTO readings (timestamp, temperature, humidity) VALUES (?, ?, ?)",
            (int(time.time()), temperature, humidity)
        )
        conn.commit()
        conn.close()

        print(f"Stored: temp={temperature}, humidity={humidity}")
    except Exception as e:
        print(f"Failed to process message: {e}")

def main():
    init_db()
    client = mqtt.Client()
    client.on_connect = on_connect
    client.on_message = on_message
    client.connect(MQTT_BROKER, MQTT_PORT)
    client.loop_forever()

if __name__ == "__main__":
    main()
```
This application, I named it as bosporus_subscriber.py, created the table for sensor data, pass the read temperature and humidty data to MQTT brocker by connecting to it, and listen continuously the sensor data which are available at the output of the ESP32 and parse the JSON file of the MQTT protocol. I created file directly in the terminal by the command **cat > ~/bosporus_subscriber.py << 'EOF'** at the beginning tof the script file. This previously named script file as **bosporus_subscriber.py** renamed later on as **gateway.py** because of the previously defined arhitecture as gateway. 

After the image with python package, python-paho-mqtt has been flashed on to the Pi. While I was flashing I faced the following issue: trying accessing PI with ssh with the know IP address 192.168.1.169 resulted with permission denied. The problem was solved by running the command **ssh -keygen -R 192.168.1.169**. The issue was that SSH serve has a unique host key - a cryptographic identity. This will be deleted after each reflash. At the first time of the accessing the ssh asks, security reasons, so do you trust this device? After saying yes, the access has been granted. But after reflash, the host key is not the same and SSH refuse the access. Thefore we need to tell it with the command above handle this device as a new one so that user can confirm again the trustnees.   

So far so good, after re-flashing the device, tested the availability of paho-mqtt with **python3 -c "import paho.mqtt.client; print('paho-mqtt is available')"** I have copied the script onto the Pi and run it manually. The reason why I have copied this file over the network is so that I can test it faster instead and flashing it with a new image which would then include this file. 

```
scp bosporus_subscriber.py root@192.168.1.169:/root/
python3 /root/bosporus_subscriber.py
```
after running the script, I have received the error message

```
# python3 /root/bosporus_subscriber.py
Traceback (most recent call last):
  File "/root/bosporus_subscriber.py", line 2, in <module>
    import sqlite3
ModuleNotFoundError: No module named 'sqlite3'
#
```
 Although I have selected the sqlite module using docker make menuconfig I received about this error message. The root cause is when the sub-options of a package are changed, the package is not automatically rebuilt. So thus I ran the command **make python3-reconfigure**. This re-runs Python3's build from its configure step. Then repeated whole flashing process. After running the script on the PI after reflush I tackled another error:

 ```
# python3 /root/bosporus_subscriber.py
python3: can't open file '/root/bosporus_subscriber.py': [Errno 2] No such file or directory
#
```
 The reason for the error was that the script hadn't made it onto the new image — it only ever existed on the SD card via the manual network copy, and that gets wiped on every reflash. So I copied it again over the network to the Pi. After running the script again, this time it worked out:

 ```
/root/bosporus_subscriber.py:49: DeprecationWarning: Callback API version 1 is deprecated, update to latest version
  client = mqtt.Client()
Connected to broker, rc=0
```
by tackling another issue: the Mosquitto config wasn't included in the new image either, wiped by the same reflash. I killed the previous Mosquitto process and restarted it:

```
ps | grep mosquitto
kill <that PID>
mosquitto -d -c /etc/mosquitto/mosquitto.conf
netstat -tuln | grep 1883
```
Then it worked. 

```
root/bosporus_subscriber.py:49: DeprecationWarning: Callback API version 1 is deprecated, update to latest version
  client = mqtt.Client()
Connected to broker, rc=0
Stored: temp=29.5, humidity=33.0
Stored: temp=29.5, humidity=33.1
Stored: temp=29.5, humidity=32.9
Stored: temp=29.5, humidity=33.0
Stored: temp=29.5, humidity=33.0
Stored: temp=29.5, humidity=33.0
Stored: temp=29.5, humidity=33.0
```
With a second SSH session I have printed the read sensor data into a table:

```
ssh root@192.168.1.169
sqlite3 /root/bosporus.db "SELECT * FROM readings ORDER BY id DESC LIMIT 5;
```
Voila!

```
╭────┬───────────┬─────────────┬──────────╮
│ id │ timestamp │ temperature │ humidity │
╞════╪═══════════╪═════════════╪══════════╡
│ 56 │       968 │        29.5 │     33.1 │
│ 55 │       966 │        29.5 │     33.0 │
│ 54 │       964 │        29.5 │     33.1 │
│ 53 │       962 │        29.5 │     33.0 │
│ 52 │       960 │        29.5 │     33.0 │
╰────┴───────────┴─────────────┴──────────╯
```

So far the script gateway.py and Mosquitto config added via network without including into the image. Let's include them permanent into the image via a Buildroot rootfs overlay. First of all make directories and then copy the gateway script itself:

```
mkdir -p ~/bosporus-overlay/opt/bosporus
mkdir -p ~/bosporus-overlay/etc/init.d
cp ~/gateway.py ~/bosporus-overlay/opt/bosporus/gateway.py
```
Then create the mosquitto config file in mosquitto directory, simply using terminal

```
mkdir -p ~/bosporus-overlay/etc/mosquitto
cat > ~/bosporus-overlay/etc/mosquitto/mosquitto.conf << 'EOF'
listener 1883 0.0.0.0
allow_anonymous true
EOF
```
An init script is necessary so that the gateway.py starts automatically at boot. 

```
cat > ~/bosporus-overlay/etc/init.d/S60bosporus-gateway << 'EOF'
#!/bin/sh
DAEMON=/usr/bin/python3
SCRIPT=/opt/bosporus/gateway.py
PIDFILE=/var/run/bosporus-gateway.pid

start() {
	printf "Starting bosporus-gateway: "
	start-stop-daemon -S -q -b -m -p $PIDFILE --exec $DAEMON -- $SCRIPT
	[ $? = 0 ] && echo "OK" || echo "FAIL"
}
stop() {
	printf "Stopping bosporus-gateway: "
	start-stop-daemon -K -q -p $PIDFILE
	[ $? = 0 ] && echo "OK" || echo "FAIL"
}
restart() {
	stop
	start
}
case "$1" in
  start) start ;;
  stop) stop ;;
  restart|reload) restart ;;
  *) echo "Usage: $0 {start|stop|restart}"; exit 1 ;;
esac
exit $?
EOF
chmod +x ~/bosporus-overlay/etc/init.d/S60bosporus-gateway
```
After I've setup the directories on Mac I need to copy them into the docker container.

```
docker cp ~/bosporus-overlay buildroot-builder:/home/builder/buildroot/board-bosporus-overlay
```
Run the following commands so that the buildroot integrate the overlay and configure it in menuconfig.

```
docker start -ai buildroot-builder
cd buildroot
make menuconfig
```
In menuconfig navigate to System configuration -> Root filesystem overlay directories then create the above directory also in the menuconfig: **board-bosporus-overlay**. Afterwards the rebuild process copying into the sd card and verifying:

```
make
docker cp buildroot-builder:/home/builder/buildroot/output/images/sdcard.img ~/bosporus-sdcard.img
ssh-keygen -R 192.168.1.169
ssh root@192.168.1.169
ps | grep -E 'mosquitto|gateway'
netstat -tuln | grep 1883
sqlite3 /opt/bosporus/bosporus.db "SELECT * FROM readings ORDER BY id DESC LIMIT 3;"
```
This resulted with:
```
╭────┬───────────┬─────────────┬──────────╮
│ id │ timestamp │ temperature │ humidity │
╞════╪═══════════╪═════════════╪══════════╡
│ 56 │       968 │        29.5 │     33.1 │
│ 55 │       966 │        29.5 │     33.0 │
│ 54 │       964 │        29.5 │     33.1 │
│ 53 │       962 │        29.5 │     33.0 │
│ 52 │       960 │        29.5 │     33.0 │
╰────┴───────────┴─────────────┴──────────╯
```
With this result Phase 3 is complete: sensor → WiFi → MQTT → auto-starting Python subscriber → SQLite.

---

## Phase 4: Fixing the clock, and a Flask dashboard

### Fixing the clock issue

Start Docker, enter the buildroot folder, then go into `menuconfig`:

```
Target packages -> Networking applications -> ntp
```
In the sub-menu, enable **sntp**.

### Flask dashboard

Flask is a web framework — a tool that lets a Python program act as a web server. It makes the Python script able to listen for HTTP requests and respond with temperature and humidity data.

Enabling Flask:
```
Target packages -> Interpreter languages and scripting -> python3 -> External python modules -> python-flask
make
```
Flask isn't part of Python's standard library. It's a separate package that needs to be present on the Pi's filesystem, hence adding **python-flask**.

I created the dashboard app on my Mac and saved it in the project folder there. Before adding `dashboard.py` to the overlay, I checked that the build had succeeded:

```
ls -lh output/images/sdcard.img
```
Then added `dashboard.py`:

```
cp ~/dashboard.py ~/bosporus-overlay/opt/bosporus/dashboard.py
```
Then got the updated overlay into the container. An overlay is a plain folder on the build machine. Buildroot copies its contents onto the target's root filesystem, at the same paths, as one of the final build steps. A file at `bosporus-overlay/opt/bosporus/gateway.py` ends up at `/opt/bosporus/gateway.py` on the Pi. This is how custom application files become a permanent part of the image — they survive reflashes, unlike files added manually over SSH/scp.

```
docker exec buildroot-builder rm -rf /home/builder/buildroot/board-bosporus-overlay
docker cp ~/bosporus-overlay buildroot-builder:/home/builder/buildroot/board-bosporus-overlay
```
Rebuilt — this picks up the overlay changes without redoing the whole toolchain:

```
docker start -ai buildroot-builder
cd buildroot
make
```
Got the image onto the Mac, reflashed, card into Pi, power cycle. After booting, checked whether Flask was running:

```
ssh root@192.168.1.169
python3 /opt/bosporus/dashboard.py
```
Result:

```
# python3 /opt/bosporus/dashboard.py
* Serving Flask app 'dashboard'
* Debug mode: off
WARNING: This is a development server. Do not use it in a production deployment. Use a production WSGI server instead.
* Running on all addresses (0.0.0.0)
* Running on http://127.0.0.1:5000
* Running on http://192.168.1.169:5000
```
That's a full success. Flask is running, listening on 0.0.0.0.

Checked the dashboard in the browser at:
```
http://192.168.1.169:5000
```
It did not work in Firefox — likely its HTTPS-Only Mode, which forces an upgrade to `https://` even when `http://` is typed, and Flask's built-in dev server only speaks plain HTTP. It worked immediately in Safari, no changes needed on the Pi side, which supports that explanation.

**Artifacts**

![Sensor Readings](images/dashboard-measurement.jpg)

**Result**

✅ Real timestamps confirmed via `sntp`. Dashboard reachable and rendering a live chart of temperature and humidity, served by Flask, running permanently via the overlay and starting automatically at boot — same as `gateway.py`. This completes the *"Live history of readings visible"* milestone from the original project plan.
