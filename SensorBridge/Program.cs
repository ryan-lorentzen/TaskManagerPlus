using System.Text.Json;
using System.ComponentModel;
using System.Diagnostics;
using System.Runtime.InteropServices;
using Microsoft.Win32.SafeHandles;
using LibreHardwareMonitor.Hardware;
using LibreHardwareMonitor.Hardware.Gpu;
using LibreHardwareMonitor.PawnIo;

internal static class Program
{
    private record Request(string GpuName, string GpuDeviceId, long RequestId = 0);
    private record Temperature(string Name, string Identifier, double? Celsius);
    private record SampledTemperature(Temperature Sensor, string HardwareName, long OffsetMs);
    private record CpuAccess(bool Available, string Description);
    private static readonly HashSet<string> ReportedErrors = [];
    private static readonly JsonSerializerOptions JsonOptions = new()
    {
        PropertyNamingPolicy = JsonNamingPolicy.CamelCase,
        PropertyNameCaseInsensitive = true
    };

    private static int Main(string[] args)
    {
        if (args.Contains("--self-test"))
            return SelfTest();

        var computer = new Computer(); // All unrelated hardware groups remain disabled.
        bool diagnostics = args.Contains("--diagnostics");
        bool once = args.Contains("--once");
        var cpuAccess = CheckCpuAccess();
        if (!cpuAccess.Available || diagnostics)
            Console.Error.WriteLine("CPU sensor access: " + cpuAccess.Description);
        try
        {
            computer.Open();
            try { computer.IsCpuEnabled = cpuAccess.Available; }
            catch (Exception e) { Console.Error.WriteLine("CPU sensors unavailable: " + e.Message); }
            try { computer.IsGpuEnabled = true; }
            catch (Exception e) { Console.Error.WriteLine("GPU sensors unavailable: " + e.Message); }

            // No timer: the C++ sampling cycle requests one response over stdin/stdout.
            string? line;
            while ((line = Console.ReadLine()) != null)
            {
                Request? request = null;
                SampledTemperature? cpuReading = null;
                SampledTemperature? gpuReading = null;
                var samplingClock = Stopwatch.StartNew();
                try
                {
                    if (line.Length > 4096) throw new InvalidDataException("Request too large");
                    request = JsonSerializer.Deserialize<Request>(line, JsonOptions)
                                  ?? new Request("", "");
                    var hardware = computer.Hardware;
                    var cpus = hardware.Where(h => h.HardwareType == HardwareType.Cpu)
                                       .OrderBy(h => h.Identifier.ToString(), StringComparer.Ordinal);
                    var cpuValues = new List<SampledTemperature>();
                    foreach (var cpu in cpus)
                    {
                        var value = ReadTemperature(cpu, true, samplingClock);
                        if (value != null) cpuValues.Add(value);
                    }
                    // On multi-socket systems report the hottest selected package sensor.
                    cpuReading = cpuValues.OrderByDescending(r => r.Sensor.Celsius)
                                          .ThenBy(r => r.Sensor.Identifier, StringComparer.Ordinal)
                                          .FirstOrDefault();

                    var gpus = hardware.OfType<GenericGpu>().ToArray();
                    var matches = gpus.Where(g => NormalizeDeviceId(g.DeviceId)
                                      == NormalizeDeviceId(request.GpuDeviceId)
                                      && !string.IsNullOrEmpty(request.GpuDeviceId)).ToArray();
                    if (matches.Length == 0)
                        matches = gpus.Where(g => g.Name.Trim().Equals(request.GpuName.Trim(),
                                                               StringComparison.OrdinalIgnoreCase)).ToArray();
                    // Duplicate names are not sufficient to identify a physical adapter.
                    if (matches.Length == 1 && (matches[0].HardwareType != HardwareType.GpuIntel || cpuAccess.Available))
                        gpuReading = ReadTemperature(matches[0], false, samplingClock);
                }
                catch (Exception e)
                {
                    ReportOnce("Sensor request unavailable: " + e.Message);
                }
                long collectionDurationMs = samplingClock.ElapsedMilliseconds;
                var response = new {
                    requestId = request?.RequestId ?? 0,
                    cpuTemperatureC = cpuReading?.Sensor.Celsius,
                    gpuTemperatureC = gpuReading?.Sensor.Celsius,
                    cpuSampleOffsetMs = cpuReading?.OffsetMs,
                    gpuSampleOffsetMs = gpuReading?.OffsetMs,
                    collectionDurationMs,
                    cpuSensor = cpuReading?.Sensor.Name,
                    gpuSensor = gpuReading?.Sensor.Name,
                    cpuSensorId = cpuReading?.Sensor.Identifier,
                    gpuSensorId = gpuReading?.Sensor.Identifier
                };
                if (diagnostics)
                    Console.Error.WriteLine(JsonSerializer.Serialize(new {
                        sampledUtc = DateTimeOffset.UtcNow,
                        cpuHardware = cpuReading?.HardwareName,
                        gpuHardware = gpuReading?.HardwareName,
                        requestedGpuDeviceId = request?.GpuDeviceId,
                        readings = response
                    }));
                Console.WriteLine(JsonSerializer.Serialize(response));
                Console.Out.Flush();
                if (once) break;
            }
        }
        catch (IOException) { } // Parent closed its pipes.
        catch (Exception e) { Console.Error.WriteLine("Sensor backend unavailable: " + e.Message); }
        finally
        {
            try { computer.Close(); }
            catch (Exception e) { Console.Error.WriteLine("Sensor cleanup: " + e.Message); }
        }
        return 0;
    }

    [DllImport("kernel32.dll", CharSet = CharSet.Unicode, ExactSpelling = true, SetLastError = true)]
    private static extern SafeFileHandle CreateFileW(string path, uint access, uint share,
        IntPtr security, uint creation, uint flags, IntPtr template);

    private static CpuAccess CheckCpuAccess()
    {
        // 0.9.6 uses the separately installed PawnIO driver. Do not ask the library
        // to read CPU registers when that backend is absent/inaccessible: zero-filled
        // low-level reads could otherwise resemble a plausible temperature.
        try
        {
            if (!PawnIo.IsInstalled) return new(false, "PawnIO installation was not detected.");
            // Match LibreHardwareMonitor's device-handle access exactly. This is
            // only an availability probe; register/sensor reads remain in the library.
            using var handle = CreateFileW(@"\\?\GLOBALROOT\Device\PawnIO", 3, 3,
                                          IntPtr.Zero, 3, 0, IntPtr.Zero);
            if (!handle.IsInvalid) return new(true, "PawnIO driver is accessible.");
            int error = Marshal.GetLastWin32Error();
            return new(false, error == 5
                ? "PawnIO is installed but access is denied to this sensor process (Windows error 5)."
                : $"PawnIO could not be opened: {new Win32Exception(error).Message} (Windows error {error}).");
        }
        catch (Exception e) { return new(false, "PawnIO access check failed: " + e.Message); }
    }

    private static SampledTemperature? ReadTemperature(IHardware hardware, bool cpu, Stopwatch samplingClock)
    {
        try
        {
            // Disable the library's own sensor history; the only chart history is in C++.
            foreach (var sensor in hardware.Sensors) sensor.ValuesTimeWindow = TimeSpan.Zero;
            hardware.Update();
            foreach (var sensor in hardware.Sensors) sensor.ValuesTimeWindow = TimeSpan.Zero;
            var selectedSensor = SelectSensor(hardware.Sensors.Where(s => s.SensorType == SensorType.Temperature)
                .Select(s => new Temperature(s.Name, s.Identifier.ToString(), s.Value)), cpu);
            return selectedSensor == null ? null : new(selectedSensor, hardware.Name, samplingClock.ElapsedMilliseconds);
        }
        catch (Exception e)
        {
            ReportOnce($"Temperature sensor update failed for {hardware.Name}: {e.Message}");
            return null; // A failing GPU must not discard the CPU reading (or vice versa).
        }
    }

    private static double? SelectTemperature(IEnumerable<Temperature> sensors, bool cpu)
        => SelectSensor(sensors, cpu)?.Celsius;

    private static Temperature? SelectSensor(IEnumerable<Temperature> sensors, bool cpu)
    {
        string[] preferred = cpu
            ? ["CPU Package", "Core (Tctl/Tdie)", "CPU (Tctl/Tdie)", "Core (Tdie)",
               "Core (Tctl)", "Core Average", "CPU Core", "CCDs Average (Tdie)"]
            : ["GPU Core", "GPU Edge", "GPU Temperature"];
        foreach (string name in preferred)
        {
            var match = sensors.Where(s => s.Name.Equals(name, StringComparison.OrdinalIgnoreCase)
                                        && s.Celsius.HasValue && double.IsFinite(s.Celsius.Value)
                                        && s.Celsius.Value > 0 && s.Celsius.Value < 150)
                               .OrderBy(s => s.Identifier, StringComparer.Ordinal).FirstOrDefault();
            if (match != null) return match;
        }
        // No per-core, hotspot, VRAM, or motherboard-zone substitution.
        return null;
    }

    private static void ReportOnce(string message)
    {
        if (ReportedErrors.Count < 32 && ReportedErrors.Add(message))
            Console.Error.WriteLine(message);
    }

    private static string NormalizeDeviceId(string id)
    {
        string result = (id ?? "").Trim();
        if (result.StartsWith(@"\\?\")) result = result[4..];
        int guid = result.IndexOf("#{", StringComparison.Ordinal);
        if (guid >= 0) result = result[..guid];
        return result.Replace('#', '\\').ToUpperInvariant();
    }

    private static int SelfTest()
    {
        int failures = 0;
        void Check(bool result, string name)
        {
            if (!result) { Console.Error.WriteLine("FAIL: " + name); ++failures; }
        }
        Check(SelectTemperature([new("CPU Package", "pkg", 55), new("Core Average", "avg", 45)], true) == 55,
              "CPU package preference");
        Check(SelectTemperature([new("Core (Tctl/Tdie)", "amd", 64)], true) == 64, "AMD die selection");
        Check(SelectTemperature([new("CPU Package", "pkg", null), new("Core Average", "avg", 45)], true) == 45,
              "Unavailable primary sensor fallback");
        Check(SelectTemperature([new("GPU Hot Spot", "hot", 85), new("GPU Core", "core", 50)], false) == 50,
              "GPU core instead of hotspot");
        Check(SelectTemperature([new("GPU Hot Spot", "hot", 85)], false) == null, "Hotspot-only unavailable");
        Check(SelectTemperature([new("CPU Package", "pkg", 0)], true) == null, "Zero unavailable");
        Check(SelectTemperature([new("CPU Package", "pkg", double.NaN)], true) == null, "Invalid unavailable");
        Check(SelectTemperature([new("Core #1", "core", 55)], true) == null, "No arbitrary per-core fallback");
        Check(SelectTemperature([new("GPU Core", "z", 60), new("GPU Core", "a", 50)], false) == 50,
              "Deterministic sensor identifier tie break");
        Check(NormalizeDeviceId(@"\\?\PCI#VEN_1002#ABC#{GUID}") == @"PCI\VEN_1002\ABC",
              "Device interface normalization");
        var selected = SelectSensor([new("GPU Core", "edge-id", 74.125), new("GPU Hot Spot", "hot-id", 90)], false);
        Check(selected?.Name == "GPU Core" && selected.Identifier == "edge-id" && selected.Celsius == 74.125,
              "Diagnostics retain exact sensor and raw Celsius without calibration");
        Console.WriteLine(failures == 0 ? "All 11 temperature selection checks passed" : "Temperature checks failed");
        return failures == 0 ? 0 : 1;
    }
}
