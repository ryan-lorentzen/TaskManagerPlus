# Temperature helper dependencies

TaskManager.SensorBridge dynamically uses the unmodified
**LibreHardwareMonitorLib 0.9.6**, copyright LibreHardwareMonitor and Contributors
(and Michael Möller/Open Hardware Monitor contributors where indicated).
It is licensed under **Mozilla Public License 2.0**.

- Corresponding source: https://github.com/LibreHardwareMonitor/LibreHardwareMonitor/tree/v0.9.6
- MPL: https://github.com/LibreHardwareMonitor/LibreHardwareMonitor/blob/v0.9.6/LICENSE
- Upstream third-party notices (including LGPL-2.1 PawnIO modules):
  https://github.com/LibreHardwareMonitor/LibreHardwareMonitor/blob/v0.9.6/THIRD-PARTY-NOTICES.txt
- PawnIO module source: https://github.com/namazso/PawnIO.Modules

CMake downloads and distributes the upstream license and third-party notice texts
alongside the helper under `sensors/licenses/`. Do not remove those files when
packaging. The LibreHardwareMonitor DLL stays separate and replaceable; no changes
have been made to its source. This repository's helper source is available under
`SensorBridge/`.

The framework-dependent helper also redistributes these unmodified dependencies:

| Dependency | Copyright | License / corresponding source |
| --- | --- | --- |
| Microsoft .NET libraries | Microsoft / .NET Foundation and contributors | MIT; https://github.com/dotnet/runtime |
| Mono.Posix.NETStandard 1.0.0 | Microsoft / Mono contributors | MIT and upstream notices; https://github.com/mono/mono |
| HidSharp 2.6.4 | James F. Bellinger, 2010–2025 | Apache-2.0; https://software.seekye.com/hidsharp |
| DiskInfoToolkit 1.1.2 | Florian K. | MPL-2.0; https://github.com/Blacktempel/DiskInfoToolkit/tree/25319eae5781e75bcf141e844ceab2afe94d40ea |
| RAMSPDToolkit-NDD 1.4.2 | Florian K. | MPL-2.0; https://github.com/Blacktempel/RAMSPDToolkit/tree/3b47b960e0830fef344624ad5e389675d5f0a1ce |
| BlackSharp.Core 1.0.7 | Florian K. | MPL-2.0; https://github.com/Blacktempel/BlackSharp/tree/c70b735c6cec123ee8a046ac4a0bc6c606f52cf0 |

The upstream MPL, LGPL/module notices, .NET/Mono notices, and HidSharp Apache license
are distributed under `sensors/licenses/`. NuGet metadata is in the generated
`project.assets.json`. Storage/RAM-SPD hardware groups remain disabled; those DLLs
are transitive library dependencies, not implemented TaskManager++ features.
The .NET runtime is installed separately, not bundled. The PawnIO driver is not installed or
redistributed by TaskManager++.
