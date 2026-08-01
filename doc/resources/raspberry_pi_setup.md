# Raspberry Pi setup

## Overview

This document describes the minimal Raspberry Pi configuration required to use it as a hardware abstraction gateway.

## Setup

For automated workflows, it is recommended to use a dedicated, locally isolated Raspberry Pi configured for this purpose with passwordless SSH access, as described below.

@todo Simultaneous hardware access is not coordinated. A remote reservation or exclusion mechanism could be implemented if needed.

Connect to the Raspberry Pi via SSH.

```bash
ssh "${RPI_USER}@${RPI_HOST}"
```

Remove the user password.

```bash
sudo passwd -d <RPI_USER>
```

Edit the SSH configuration file.

```bash
sudo nano /etc/ssh/sshd_config
```

Ensure the following options are set:

```text
PasswordAuthentication yes
PermitEmptyPasswords yes
```

Restart the SSH service.

```bash
sudo systemctl restart ssh
```
