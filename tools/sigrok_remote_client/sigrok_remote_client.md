# Sigrok remote client overview

`sigrok_remote_client.py` provides a minimal Python wrapper that runs `sigrok-cli` on the Raspberry Pi and stores fetched capture artifacts under the configured local logic analyzer artifacts directory.

It works as follows:
- Loading the Raspberry Pi SSH settings and logic analyzer configuration from the environment variables.
- Running `sigrok-cli` on the Raspberry Pi with the configured `LOGIC_ANALYZER_DEVICE` selector.
- Fetching one or more remote capture artifacts into `LOGIC_ANALYZER_ARTIFACTS_DIR/<capture_name>/`.

# Glossary

| Term | Definition |
|---|---|
| `.sr` capture | Sigrok session file that stores one logic analyzer capture. |
| `sigrok-cli` | Command-line tool used to control capture and export operations of the logic analyzer. |
| PulseView | Sigrok graphical viewer used to inspect `.sr` captures interactively. |

# Usage example

Configuration:
- `RPI_USER`: Raspberry Pi SSH username.
- `RPI_HOST`: Raspberry Pi IP address.
- `LOGIC_ANALYZER_DEVICE`: Sigrok device selector assigned to the logic analyzer workflow.
- `LOGIC_ANALYZER_ARTIFACTS_DIR`: Directory where logic analyzer artifacts are stored grouped by logical capture name.

Run one remote capture and fetch the resulting `.sr` file into the configured local artifacts directory:

```python
from tools.sigrok_remote_client import SigrokRemoteClient

client = SigrokRemoteClient()
remote_capture = "/tmp/example_test.sr"

client.run(
  "--config", "<config>",
  "--output-file", remote_capture,
)

local_capture = client.fetch_artifact(
  "example_test",
  remote_capture,
)

print(local_capture)
```

Start one remote capture asynchronously and wait for it to finish later:

```python
from tools.sigrok_remote_client import SigrokRemoteClient

client = SigrokRemoteClient()

capture_proc = client.run_async(
  "--config", "<config>",
  "--output-file", "<remote_capture>",
)

capture_proc.wait(timeout=<timeout_seconds>)
```

Run one offline Sigrok conversion on the Raspberry Pi without injecting the configured device selector. Note that the client still requires the standard remote Sigrok environment configuration at construction time, including `LOGIC_ANALYZER_DEVICE`, even though `run_offline()` itself does not forward that selector to `sigrok-cli`:

```python
from tools.sigrok_remote_client import SigrokRemoteClient

client = SigrokRemoteClient()

client.run_offline(
  "-i", "<remote_input_capture>",
  "-O", "<output_format>",
  "-o", "<remote_output_artifact>",
)
```
