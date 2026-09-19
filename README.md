# SystemScope

**Windows System Monitoring & Diagnostics**

SystemScope is a C++ desktop application designed to simplify Windows performance monitoring and system troubleshooting. It combines real-time hardware monitoring with a centralized error dashboard, making it easier to identify crashes, hardware errors, and other significant system events without digging through Windows Event Viewer.

The goal is to provide a lightweight, organized, and accessible tool for understanding system performance and diagnosing instability.

---

## Version 1 — Core Features

### 1. System Performance Monitoring

Monitor essential system metrics in real time through a desktop dashboard.

* CPU utilization
* RAM usage and available memory
* Historical performance graphs
* Background monitoring without interrupting the interface

### 2. Unified Error Center

Collect and display important Windows events in one centralized interface.

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

SystemScope presents diagnostic evidence without automatically assuming that one event caused another.

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
* [ ] Create the Error Center interface.
* [ ] Add event filtering and detailed event views.
* [ ] Implement live event monitoring and SQLite storage.
* [ ] Build CPU and RAM monitoring.
* [ ] Create real-time performance graphs.
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

* CPU and GPU temperature monitoring.
* GPU utilization and VRAM usage.
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

SystemScope is intended to be both a practical Windows diagnostic utility and an exploration of systems-level software engineering.

Key development goals include:

* Gain experience with modern C++ and native Windows APIs.
* Implement responsive, multithreaded desktop software.
* Develop reliable event collection and processing.
* Design persistent local data storage.
* Apply modular architecture and automated testing.

The initial release will prioritize reliability, usability, and a focused feature set before introducing more advanced diagnostic capabilities.
