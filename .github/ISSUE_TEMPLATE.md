---
name: Bug report or feature request
about: Report a problem or suggest an enhancement
title: ''
labels: ''
assignees: ''
---

## Issue Type

- [ ] Bug report
- [ ] Feature request
- [ ] Documentation issue
- [ ] Question

## Description

<!-- A clear and concise description of the issue or feature request -->

## Hardware Information

- **Device**: LilyGo T5 4.7" E-Paper Display (ESP32-S3)
- **Firmware Version**: <!-- e.g., v1.1.0 - shown on device during boot -->
- **FastEPD Version**: <!-- Patched or unpatched? -->

## Expected Behavior

<!-- What you expected to happen -->

## Actual Behavior

<!-- What actually happened -->

## Steps to Reproduce

<!-- If reporting a bug, provide steps to reproduce the issue -->

1.
2.
3.

## Serial Output

<!-- If possible, paste relevant serial output (115200 baud) -->

```
[Paste serial output here]
```

## Additional Context

<!-- Any other information that might be helpful -->

- WiFi signal strength (RSSI):
- Battery level:
- Environment (temperature, humidity, etc.):
- Other relevant details:

## Known Issue Checklist

Before submitting, please check if your issue is already documented:

- [ ] I've checked `KNOWN_ISSUES.md`
- [ ] I've checked `FASTEPD_KNOWN_BUG.md` (if display-related)
- [ ] I've checked existing GitHub issues
- [ ] I've checked the troubleshooting section in `README.md`

## For Display Issues

If reporting a display issue, please indicate:

- [ ] Issue occurs with patched FastEPD version (this repo)
- [ ] Issue occurs with unpatched FastEPD version
- [ ] Issue is the known right-edge artifact (see FASTEPD_KNOWN_BUG.md)
- [ ] Issue is something different

<!-- Note: The known right-edge rendering artifact is documented in FASTEPD_KNOWN_BUG.md and is caused by an upstream FastEPD library bug -->
