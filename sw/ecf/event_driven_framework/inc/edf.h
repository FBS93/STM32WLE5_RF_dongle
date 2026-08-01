/*******************************************************************************
 * @brief Event Driven Framework (EDF).
 *
 * @copyright
 * Copyright (c) 2026 FBS93.
 * See the LICENSE file of this project for license details.
 * This notice shall be retained in all copies or substantial portions
 * of the software.
 *
 * @note
 * This file is a derivative work based on:
 * QP/C (c) Quantum Leaps, LLC.
 *
 * @warning
 * This software is provided "as is", without any express or implied warranty.
 * The user assumes all responsibility for its use and any consequences.
 ******************************************************************************/

#ifndef EDF_H
#define EDF_H

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/

/* -----------------------------------------------------------------------------
 * System library headers
 * -------------------------------------------------------------------------- */

/* -----------------------------------------------------------------------------
 * External library headers
 * -------------------------------------------------------------------------- */

/* -----------------------------------------------------------------------------
 * Project-specific headers
 * -------------------------------------------------------------------------- */
#include "edf_config.h"
#include "edf_active_object.h"
#include "edf_event_queue.h"
#include "edf_event.h"
#include "edf_hsm.h"
#include "edf_pool.h"
#include "edf_time_event.h"
#include "ebf_config.h"

/*******************************************************************************
 * PUBLIC MACROS
 ******************************************************************************/

/*******************************************************************************
 * PUBLIC TYPEDEFS
 ******************************************************************************/

/*******************************************************************************
 * PUBLIC VARIABLES
 ******************************************************************************/

/*******************************************************************************
 * PUBLIC FUNCTIONS
 ******************************************************************************/

/**
 * @brief Initializes the EDF framework.
 *
 * Shall be called once before any other function of the framework.
 */
void EDF_init(void);

/**
 * @brief Stops the EDF framework.
 */
void EDF_stop(void);

/**
 * @brief Takes control and runs the EDF main loop.
 *
 * @return 0 on normal exit, non-zero on error.
 */
int EDF_run(void);

/**
 * @brief User-specific startup callback.
 *
 * Called just before the EDF framework takes control with the main loop.
 * The framework provides a dummy weak implementation, can be overridden by the
 * application.
 */
void EDF_onStartup(void);

/**
 * @brief User-specific shutdown callback.
 *
 * Called just before the EDF framework stops.
 * The framework provides a dummy weak implementation, can be overridden by the
 * application.
 */
void EDF_onShutdown(void);

#if (EBF_CORE == EBF_CORE_BAREMETAL)
/**
 * @brief Framework ISR exit handler.
 *
 * Shall be called at the end of every ISR that posts an EDF event.
 * Evaluates pending activations of active objects and triggers a
 * context switch if required.
 */
void EDF_ISRExit(void);
#endif  //(EBF_CORE == EBF_CORE_BAREMETAL)

#if ((EBF_CORE == EBF_CORE_BAREMETAL) ||            \
     (EBF_PORT == EBF_PORT_POSIX_NON_PREEMPTIVE) || \
     (EBF_PORT == EBF_PORT_POSIX_NON_PREEMPTIVE_FUZZ))
/**
 * @brief User-specific context switch callback.
 *
 * Called whenever EDF switches execution between two active objects.
 * The framework provides a dummy weak implementation, can be overridden
 * by the application (e.g., to trace or profile scheduling, apply memory
 * protection techniques, etc.).
 *
 * @param[in] prev Pointer to the previously running active object (can be
 * NULL).
 * @param[in] next Pointer to the newly scheduled active object.
 */
void EDF_onContextSwitch(EDF_activeObject_t* prev, EDF_activeObject_t* next);

/**
 * @brief User-specific idle callback.
 *
 * Called whenever EDF have no active objects are ready to run.
 * The framework provides a dummy weak implementation, can be overridden by the
 * application.
 *
 * @note In non-preemptive ports, the idle task can delay the execution of the
 * next ready active object.
 */
void EDF_onIdle(void);
#endif  // ((EBF_CORE == EBF_CORE_BAREMETAL) ||
        //  (EBF_PORT == EBF_PORT_POSIX_NON_PREEMPTIVE) ||
        //  (EBF_PORT == EBF_PORT_POSIX_NON_PREEMPTIVE_FUZZ))

/**
 * @todo Implement a logging mechanism called edf_spy module to collect
 * and expose internal metrics (e.g. watermarks of buffers, event queues,
 * pools, etc.). The mechanism should be flexible enough to extend with
 * additional logs in the future. This must be integrated into the
 * framework-wide log module to ensure access to internal variables.
 *
 * Example reference of a function that previously was in edf_active_object
 * module, but should be moved into edf_spy:
 *
 * @code
 * uint_fast16_t EDF_activeObject_getQueueMin(uint8_t prio)
 * {
 *     EBF_CRITICAL_SECTION_ENTRY();
 *     EAF_ASSERT_IN_CRITICAL_SECTION(prio <= EDF_MAX_ACTIVE_OBJECT);
 *     EAF_ASSERT_IN_CRITICAL_SECTION(EDF_framework.ao_registry[prio] != NULL);
 *     uint_fast16_t min =
 *        (uint_fast16_t)EDF_framework.ao_registry[prio]->e_queue.n_min;
 *     EBF_CRITICAL_SECTION_EXIT();
 *     return min;
 * }
 * @endcode
 */

/**
 * @todo Review and clean up all @c #include directives across the framework.
 * Many modules currently include @c edf_core.h, but now @c ebf.h already
 * provides critical section handling. It should be checked whether
 * @c edf_core.h (and potentially other headers) are still needed in each file.
 *
 * This review must be applied consistently to all framework modules.
 */

/**
 * @todo Review and enforce consistent coding style across the framework.
 * This includes comment formatting, naming conventions (functions, variables,
 * types, macros) and implementation patterns. Ensure all modules follow
 * the same style guide uniformly.
 */

/**
 * @todo Investigate how the framework could provide mechanisms for applying
 * memory protection between kernel and user code, as well as between individual
 * Active Objects (AOs). Define potential integration points within the EDF
 * scheduling and context-switching model.
 */

/**
 * @todo Add coding guidelines defining mandatory suffixes (U, UL, ULL)
 * for numeric literals to ensure type safety and consistency.
 */

/**
 * @todo Add freertos port.
 */

/**
 * @todo Add RISCV port.
 */

#endif /* EDF_H */
