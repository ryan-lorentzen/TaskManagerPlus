
The diagram below describes the planned architecture.

Current hardware-identity path: `main.cpp` exposes `HardwareInfoProvider` to
`Main.qml`. The provider runs one WMI query batch on a worker thread and delivers
CPU/GPU names and installed RAM to the existing Overview cards on the GUI thread.
Windows collection and memory normalization live in `HardwareInfo.cpp`; no live
monitoring, event collection, or storage is implemented by this component.

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
