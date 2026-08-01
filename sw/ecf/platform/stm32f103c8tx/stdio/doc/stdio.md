# Stdio overview

This library provides strong definitions for the EBF stdin/stdout functions using USART1 for STM32F103C8Tx series.

TX is non-buffered, supports only single-byte writes, so may block when serializing consecutive bytes.

RX is interrupt-driven and forwards received bytes to a user-registered callback (only one listener is supported).

RX interrupts are enabled and configured with the highest NVIC priority to avoid data loss.

Hardware configuration:
- USART: USART1
- TX pin: PA9 (alternate function push-pull)
- RX pin: PA10 (floating input)
- Baud rate: 115200 bps
- Clock source: 8 MHz HSI (no PLL)

# Glossary

| Term | Definition |
|------|------------|
|   |   |

# Usage example

See usage example in [ebf.md](../../../../embedded_base_framework/doc/ebf.md).
