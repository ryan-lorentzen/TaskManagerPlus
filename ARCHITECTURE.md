
The diagram below describes the planned architecture.

Current hardware-identity path: `main.cpp` exposes `HardwareInfoProvider` to
`Main.qml`. The provider runs one WMI query batch on a worker thread and delivers
CPU/GPU names and installed RAM to the existing Overview cards on the GUI thread.
Windows collection and memory normalization live in `HardwareInfo.cpp`; no live
monitoring, event collection, or storage is implemented by this component.

Overview live metrics: `main.cpp` exposes `SystemMetricsProvider` to `Main.qml`.
One worker samples native CPU times, physical RAM, and the selected GPU's PDH
engine counters approximately once per second. A single automatically launched
.NET/LibreHardwareMonitor helper answers temperature requests over local pipes;
it has no independent polling timer. GUI deliveries are coalesced to the latest
snapshot, while timestamped CPU/GPU histories stay bounded to 60 readings/60
seconds. The existing sample series continue to serve the other pages. No metric
history is persisted. The native metrics component owns and closes its PDH query,
worker, and helper process; unavailable temperature sensors fail independently.
Temperature replies carry request IDs and per-sensor acquisition offsets; each
reading expires after one sampling interval, independently of the other sensor.

TaskManager++
│
├── Presentation Layer
│   └── Qt UI
│
├── Application Layer
│   ├── Performance Monitor
│   ├── Event Collector
│   └── Incident Processor
│
├── Data Layer
│   └── SQLite Repository
│
└── Platform Layer
    └── Windows APIs
