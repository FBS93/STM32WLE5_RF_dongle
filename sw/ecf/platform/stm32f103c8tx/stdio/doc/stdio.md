# Stdio overview

This library provides strong definitions for the EBF stdin/stdout functions using USART1 for STM32F103C8Tx series.

TX uses a 2048-byte interrupt-driven FIFO. Message admission is internally protected, non-blocking and all-or-nothing for messages up to the TX FIFO capacity. Physical USART transmission occurs only in the USART interrupt handler, outside the admission critical section. Accepted messages retain FIFO order and cannot interleave. Interrupt latency caused by admission is bounded by the accepted message length copied into the FIFO.

RX is interrupt-driven and forwards received bytes to a user-registered callback (only one listener is supported).

The USART1 interrupt is configured with the highest NVIC priority. A USART receive overrun activates EAF assert.

Hardware configuration:
- USART: USART1
- TX pin: PA9 (alternate function push-pull)
- RX pin: PA10 (floating input)
- Baud rate: 115200 bps
- Assumed APB2 clock: 8 MHz; this module does not configure the system clock

# Glossary

| Term | Definition |
|------|------------|
|   |   |

# Usage example

See usage example in [ebf.md](../../../../embedded_base_framework/doc/ebf.md).
