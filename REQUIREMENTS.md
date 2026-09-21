
# TaskManager++ — Version 1 Requirements

## Objective

Develop a Windows 11 desktop application that
consolidates performance monitoring and significant
system events into a single interface.

## Required Functionality

### Performance Monitoring
- Display current CPU utilization.
- Display RAM utilization and available memory.
- Display historical performance graphs.

### Error Center
- Retrieve historical Windows events.
- Monitor newly recorded events.
- Display severity, source, timestamp, event ID,
  and diagnostic information.
- Filter events by severity, category, and date.

### Event Storage
- Store collected events locally using SQLite.
- Preserve records across application restarts.
- Prevent duplicate event records.

### Incident Timeline
- Display potentially related events chronologically.
- Allow inspection of nearby performance data.
- Avoid unsupported conclusions about root causes.

### Reports
- Export event information.
- Export recorded performance data.

## Out of Scope

The following are not required for Version 1:

- Automatic crash diagnosis
- GPU and CPU temperature monitoring
- Windows Error Reporting integration
- Automatic process termination
- Remote monitoring
- Cloud storage

These features may be considered for later releases.

## Completion Criteria

Version 1 is complete when all required functionality
is implemented, verified, and documented.