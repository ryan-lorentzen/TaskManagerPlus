# TaskManager++

**Windows System Monitoring & Diagnostics**

TaskManager++ is a C++ desktop application designed to simplify Windows performance monitoring and system troubleshooting. It combines real-time hardware monitoring with a centralized error dashboard, making it easier to identify crashes, hardware errors, and other significant system events without digging through Windows Event Viewer.

The goal is to provide a lightweight, organized, and accessible tool for understanding system performance and diagnosing instability.

**Current status:** Overview displays real CPU/GPU utilization, current physical RAM usage, and available CPU/GPU temperatures, sampled approximately once per second. Its CPU/GPU activity chart uses a rolling 60-second history with a fixed 0–100% scale. The Celsius/Fahrenheit switch converts temperatures for display only. CPU/GPU identity and installed RAM/type are still detected once at startup through Windows WMI. Performance, Events, Incident Timeline, and Reports remain sample-data previews; the Incident Timeline's sample resource lines can be toggled individually, and report exports remain disabled.

For multiple GPUs, Overview prefers the adapter whose WMI device ID matches the primary desktop display adapter; otherwise it displays the first name in sorted order. GPU counters and temperatures follow that selection, using device IDs and a unique name match only when necessary. Ambiguous or unavailable metrics display `—`; unknown or mixed installed RAM types are omitted. Event collection, filtering, SQLite storage, persistent performance history, and the other pages' live functionality are not connected.

### Overview metrics and temperature dependency

CPU usage uses `GetSystemTimes` deltas (kernel time includes idle); RAM uses
`GlobalMemoryStatusEx`. GPU usage uses Windows PDH GPU Engine counters matched to
the selected adapter's DXGI LUID: process contributions are summed within each
physical engine, and the busiest engine is displayed. Multi-processor-group
systems show CPU usage as unavailable rather than presenting one group's usage
as whole-system usage. CPU/GPU samples have monotonic timestamps, retain at most
60 readings/60 seconds, and leave gaps on failed readings; history is not saved.

Temperatures use **LibreHardwareMonitorLib 0.9.6** through the automatically
started `sensors/TaskManager.SensorBridge.exe`. This optional, framework-dependent
helper requires the **.NET 8 x64 runtime**; building it requires a .NET 8-or-newer
SDK and NuGet access. CMake builds it with the existing Qt workflow when an SDK is
available (`TASKMANAGER_BUILD_SENSOR_BRIDGE=OFF` skips that build). Publish/package
the entire generated `sensors/` directory, including its `licenses/` directory.
Dependency/source notices are in `SensorBridge/THIRD_PARTY_NOTICES.md`.

CPU temperature access additionally requires LibreHardwareMonitor's supported
**PawnIO driver**, installed separately, and permission to access it. TaskManager++
does not install drivers or automatically elevate. Missing runtime/helper,
inaccessible sensors, or a failed sensor process do not stop native utilization or
RAM sampling. On the development machine GPU core temperature was available;
PawnIO was installed and running, but the standard-user sensor process received
access denied, so CPU temperature displayed `—`. That does not mean the CPU lacks
a sensor or that Adrenalin cannot read it. CPU collection requires driver access;
restart the application after changing its permissions or driver availability.

Temperature replies include request IDs and separate CPU/GPU acquisition timings.
Readings older than one sampling interval (one second), malformed replies, and
replies for another request are rejected; GPU readings must also match the selected
adapter. Driver-access and sensor-update
failures are reported once in Qt Creator's Application Output, not in normal UI
labels. The helper accepts `--diagnostics --once` with one JSON request on stdin
(`requestId`, `gpuName`, `gpuDeviceId`) to report selected sensor names/identifiers,
raw Celsius values, and acquisition timing, then exit. Celsius values are only
rounded for display; no percentage calibration is applied to match another tool.

CPU sensor priority is CPU Package, Core/CPU (Tctl/Tdie), Core (Tdie), Core (Tctl),
Core Average, CPU Core, then CCDs Average (Tdie); identifier order breaks ties.
Multi-socket systems use the hottest selected package/die reading. GPU priority is
GPU Core, GPU Edge, then GPU Temperature. Hotspot, VRAM, per-core, and motherboard
thermal zones are not substituted. Windows driver/counter support and sensor
coverage vary; the initial live verification used AMD hardware, not every vendor.

---

## Version 1 — Core Features

### 1. System Performance Monitoring

Monitor essential system metrics in real time through a desktop dashboard.

* CPU utilization
* RAM usage and available memory
* Historical performance graphs
* Background monitoring without interrupting the interface

### 2. Unified Events Screen

Collect and display important Windows events and their history in one centralized interface.

* Retrieve historical events from Windows Event Logs.
* Monitor new events as they occur.
* Collect hardware, system, and application errors.
* Filter events by severity, category, and date.
* Display readable summaries alongside original technical information.

Each event includes its timestamp, severity, source, event ID, and available diagnostic details.

### 3. Event History & Storage

Maintain a persistent history of collected system events using a local SQLite database.

* Preserve event records across application restarts.
* Prevent duplicate entries.
* Search and review previous errors.
* Organize events chronologically.

### 4. Incident Timeline

Help users investigate system failures by displaying events that occurred around the same time.

* View a chronological timeline of nearby errors.
* Identify potentially related events.
* Compare event timestamps with recorded performance data.

TaskManager++ presents diagnostic evidence without automatically assuming that one event caused another.

### 5. Diagnostic Reports

Allow users to export collected information for troubleshooting.

* Export event history and technical details.
* Export recorded performance data.
* Generate reports for individual diagnostic sessions.

---

## Technology Stack

| Technology  | Purpose                                  |
| ----------- | ---------------------------------------- |
| C++         | Core application and monitoring logic    |
| Qt          | Desktop interface and performance graphs |
| Windows API | System metrics and event log collection  |
| SQLite      | Local event and performance storage      |
| CMake       | Build configuration                      |
| GoogleTest  | Automated testing                        |

**Target platform:** Windows 11

---

## Development Roadmap

* [ ] Implement Windows Event Log collection.
* [ ] Connect the unified Events interface to collected events.
* [ ] Add event filtering and detailed event views.
* [ ] Implement live event monitoring and SQLite storage.
* [x] Add live CPU and physical RAM usage to Overview.
* [x] Add rolling live CPU/GPU utilization graphs to Overview.
* [ ] Implement basic incident timelines.
* [ ] Add diagnostic report exports.
* [ ] Write automated tests and package the application.

---

## Future Features

The following features are planned for potential releases beyond Version 1.

### Advanced Crash Diagnostics

* Integrate Windows Error Reporting to collect additional crash information.
* Support LiveKernelEvent reports and bug-check details.
* Detect recurring application and driver failures.
* Associate relevant crash reports with existing incidents.

### Hardware Monitoring

* Broader CPU/GPU temperature hardware coverage (Overview reads supported sensors).
* VRAM usage (GPU utilization is already connected on Overview).
* Disk utilization and drive health information.
* Configurable performance thresholds and alerts.

### Performance Recording

* Record system performance during gaming or demanding applications.
* Automatically preserve recent performance history when a significant event is detected, where possible.
* Generate session summaries highlighting resource usage and performance spikes.

### Intelligent Event Analysis

* Group related events into individual incidents.
* Identify repeated error patterns.
* Provide explanations for common Windows errors.
* Suggest relevant troubleshooting steps based on documented event information.

### Additional Functionality

* System tray integration and desktop notifications.
* Advanced event searching and customizable filters.
* Configurable data retention.
* Privacy controls and redaction options for exported reports.

---

## Project Goals

TaskManager++ is intended to be both a practical Windows diagnostic utility and an exploration of systems-level software engineering.

Key development goals include:

* Gain experience with modern C++ and native Windows APIs.
* Implement responsive, multithreaded desktop software.
* Develop reliable event collection and processing.
* Design persistent local data storage.
* Apply modular architecture and automated testing.

The initial release will prioritize reliability, usability, and a focused feature set before introducing more advanced diagnostic capabilities.
