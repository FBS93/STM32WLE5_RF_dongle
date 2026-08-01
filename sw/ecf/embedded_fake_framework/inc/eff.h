/*******************************************************************************
 * @brief Defines macros and mechanisms to create fake functions.
 *
 * @copyright
 * Copyright (c) 2026 FBS93.
 * See the LICENSE file of this project for license details.
 * This notice shall be retained in all copies or substantial portions
 * of the software.
 *
 * @note
 * This file is a derivative work based on:
 * fff (c) Meekrosoft.
 *
 * @warning
 * This software is provided "as is", without any express or implied warranty.
 * The user assumes all responsibility for its use and any consequences.
 ******************************************************************************/

#ifndef EFF_H
#define EFF_H

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/

/* -----------------------------------------------------------------------------
 * System library headers
 * -------------------------------------------------------------------------- */
#include <string.h>

/* -----------------------------------------------------------------------------
 * External library headers
 * -------------------------------------------------------------------------- */

/* -----------------------------------------------------------------------------
 * Project-specific headers
 * -------------------------------------------------------------------------- */

/*******************************************************************************
 * PUBLIC MACROS
 ******************************************************************************/

// clang-format off
// clang-format disabled: macro layout kept for readability.

/* -----------------------------------------------------------------------------
 * Macros for external use (used in tests and user code)
 * -------------------------------------------------------------------------- */

/**
 * @brief Sets the sequence of return values for a fake function.
 *
 * Assigns an array of return values and its length to the specified fake
 * function, enabling it to return different values on consecutive calls.
 *
 * @param[in] FUNCNAME Name of the fake function.
 * @param[in] ARRAY_POINTER Pointer to the array of return values.
 * @param[in] ARRAY_LEN Length of the return values array.
 */
#define SET_RETURN_SEQ(FUNCNAME, ARRAY_POINTER, ARRAY_LEN) \
  FUNCNAME##_fake.return_val_seq = ARRAY_POINTER;          \
  FUNCNAME##_fake.return_val_seq_len = ARRAY_LEN;

/**
 * @brief Sets the sequence of custom fake function pointers.
 *
 * Assigns an array of custom fake function pointers and its length to the
 * specified fake function, allowing dynamic behavior changes on consecutive
 * calls.
 *
 * @param[in] FUNCNAME Name of the fake function.
 * @param[in] ARRAY_POINTER Pointer to the array of custom fake function
 * pointers.
 * @param[in] ARRAY_LEN Length of the custom fake function pointer array.
 */
#define SET_CUSTOM_FAKE_SEQ(FUNCNAME, ARRAY_POINTER, ARRAY_LEN) \
  FUNCNAME##_fake.custom_fake_seq = ARRAY_POINTER;              \
  FUNCNAME##_fake.custom_fake_seq_len = ARRAY_LEN;

/**
 * @brief Resets the internal state of a fake function.
 *
 * Calls the reset function associated with the specified fake function,
 * clearing call counts, argument histories and return value sequences.
 *
 * @param[in] FUNCNAME Name of the fake function to reset.
 */
#define RESET_FAKE(FUNCNAME) \
  {                          \
    FUNCNAME##_reset();      \
  }

/**
 * @brief Resets the global call history.
 *
 * Clears the recorded history of fake function calls by resetting the
 * call index and zeroing the call history array.
 */
#define EFF_RESET_HISTORY() \
  eff.call_history_idx = 0; \
  /* NOLINTNEXTLINE(clang-analyzer-security.insecureAPI.DeprecatedOrUnsafeBufferHandling) */ \
  memset((void *)eff.call_history, 0, sizeof(eff.call_history));

/* -----------------------------------------------------------------------------
 * Macros for fake function definition and configuration
 * -------------------------------------------------------------------------- */

/**
 * @brief Defines the global call history instance.
 *
 * Shall be used to define EFF global variables.
 */
#define DEFINE_EFF_GLOBALS eff_globals_t eff;

/**
 * @brief Maximum number of arguments supported by a fake function.
 */
#define EFF_MAX_ARGS (10U)

/**
 * @brief Maximum number of arguments stored per fake function call.
 *
 * Defines how many argument values are stored in history for each individual
 * fake function argument.
 *
 * @note Can be overridden before including this header.
 */
#ifndef EFF_ARG_HISTORY_LEN
#define EFF_ARG_HISTORY_LEN (50U)
#endif

/**
 * @brief Maximum number of fake function calls stored globally.
 *
 * Defines how many function calls can be recorded in the global call history,
 * allowing verification of the order in which fakes were invoked.
 *
 * @note Can be overridden before including this header.
 */
#ifndef EFF_CALL_HISTORY_LEN
#define EFF_CALL_HISTORY_LEN (50U)
#endif

/**
 * @brief Function attributes for fake function declarations.
 *
 * Allows users to define compiler-specific attributes  to be applied to all
 * generated fake function definitions.
 *
 * @note Can be overridden before including this header.
 */
#ifndef EFF_GCC_FUNCTION_ATTRIBUTES
#define EFF_GCC_FUNCTION_ATTRIBUTES
#endif

/**
 * @brief Custom function pointer template for fake function handlers.
 *
 * Allows overriding the way function pointers are declared for `custom_fake`
 * and `custom_fake_seq`. Useful for adapting to different coding standards
 * or function signature requirements.
 *
 * @note Can be overridden before including this header.
 *
 * @param[in] RETURN Return type of the function pointer.
 * @param[in] FUNCNAME Name identifier for the function pointer.
 * @param[in] ... Parameter types of the function pointer.
 * @return Function pointer type matching the specified signature.
 */
#ifndef CUSTOM_EFF_FUNCTION_TEMPLATE
#define CUSTOM_EFF_FUNCTION_TEMPLATE(RETURN, FUNCNAME, ...) \
  RETURN (*FUNCNAME)(__VA_ARGS__)
#endif

/**
 * @defgroup FakeFunction fake function macros
 * @brief Macros to declare and define fake functions (with or without return value) using variadic arguments.
 *
 * These macros automatically dispatch to the correct internal fake function macro
 * based on the number of arguments (0–10).
 * @{
 */
#define FAKE_VALUE_FUNC(...) EXPAND(FUNC_VALUE_(PP_NARG_MINUS2(__VA_ARGS__), __VA_ARGS__))
#define FAKE_VOID_FUNC(...) EXPAND(FUNC_VOID_(PP_NARG_MINUS1(__VA_ARGS__), __VA_ARGS__))
/** @} */ // End of FakeFunction

/**
 * @defgroup FakeFunctionDeclarations fake function declaration macros
 * @brief Macros to declare fake functions (void or non-void) with 0–10 arguments.
 *
 * These macros resolve to the appropriate `DECLARE_FAKE_*` variant based on argument count.
 * @{
 */
#define DECLARE_FAKE_VALUE_FUNC(...) EXPAND(DECLARE_FUNC_VALUE_(PP_NARG_MINUS2(__VA_ARGS__), __VA_ARGS__))
#define DECLARE_FAKE_VOID_FUNC(...) EXPAND(DECLARE_FUNC_VOID_(PP_NARG_MINUS1(__VA_ARGS__), __VA_ARGS__))
/** @} */ // End of FakeFunctionDeclarations

/**
 * @defgroup FakeFunctionDefinitions fake function definition macros
 * @brief Macros to define fake functions (void or non-void) with 0–10 arguments.
 *
 * Place these in your source (.c) files to define the implementation of a fake function.
 * @{
 */
#define DEFINE_FAKE_VALUE_FUNC(...) EXPAND(DEFINE_FUNC_VALUE_(PP_NARG_MINUS2(__VA_ARGS__), __VA_ARGS__))
#define DEFINE_FAKE_VOID_FUNC(...) EXPAND(DEFINE_FUNC_VOID_(PP_NARG_MINUS1(__VA_ARGS__), __VA_ARGS__))
/** @} */ // End of FakeFunctionDefinitions

/* -----------------------------------------------------------------------------
 * Macros for internal use only
 * -------------------------------------------------------------------------- */

/**
 * @brief Declares storage for a single argument value and its history.
 *
 * @param[in] type Data type of the argument.
 * @param[in] n Argument index number.
 * @param[in] FUNCNAME Name identifier for the fake function.
 */
#define DECLARE_ARG(type, n, FUNCNAME) \
  type arg##n##_val;                   \
  type arg##n##_history[EFF_ARG_HISTORY_LEN];

/**
 * @brief Declares common variables for a fake function.
 */
#define DECLARE_ALL_FUNC_COMMON \
  unsigned int call_count;      \
  unsigned int arg_history_len; \
  unsigned int arg_histories_dropped;

/**
 * @brief Declares storage for the history of return values.
 *
 * @param[in] RETURN_TYPE Data type of the return value.
 */
#define DECLARE_RETURN_VALUE_HISTORY(RETURN_TYPE) \
  RETURN_TYPE return_val_history[EFF_ARG_HISTORY_LEN];

/**
 * @brief Saves the current value of an argument to the fake function's storage.
 *
 * @param[in] FUNCNAME Name identifier for the fake function.
 * @param[in] n Argument index number.
 */
#define SAVE_ARG(FUNCNAME, n) \
  /* NOLINTNEXTLINE(clang-analyzer-security.insecureAPI.DeprecatedOrUnsafeBufferHandling) */ \
  memcpy((void *)&FUNCNAME##_fake.arg##n##_val, (const void *)&arg##n, sizeof(arg##n));

/**
 * @brief Checks if there is space to store more call history for a fake function.
 *
 * @param[in] FUNCNAME Name identifier for the fake function.
 * @return True if more call history can be stored, false otherwise.
 */
#define ROOM_FOR_MORE_HISTORY(FUNCNAME) \
  FUNCNAME##_fake.call_count < EFF_ARG_HISTORY_LEN

/**
 * @brief Saves a return value to the history of a fake function.
 *
 * Stores the return value of the most recent call in the return value history
 * if there is room.
 *
 * @param[in] FUNCNAME Name identifier for the fake function.
 * @param[in] RETVAL Return value to save.
 */
#define SAVE_RET_HISTORY(FUNCNAME, RETVAL)                    \
  if ((FUNCNAME##_fake.call_count - 1) < EFF_ARG_HISTORY_LEN) \
    /* NOLINTNEXTLINE(clang-analyzer-security.insecureAPI.DeprecatedOrUnsafeBufferHandling) */ \
    memcpy((void *)&FUNCNAME##_fake.return_val_history[FUNCNAME##_fake.call_count - 1], (const void *)&(RETVAL), sizeof(RETVAL));

/**
 * @brief Saves an argument value to the argument history of a fake function.
 *
 * Copies the current argument value into the history buffer at the current call index.
 *
 * @param[in] FUNCNAME Name identifier for the fake function.
 * @param[in] ARGN Argument index number.
 */
#define SAVE_ARG_HISTORY(FUNCNAME, ARGN) \
  /* NOLINTNEXTLINE(clang-analyzer-security.insecureAPI.DeprecatedOrUnsafeBufferHandling) */ \
  memcpy((void *)&FUNCNAME##_fake.arg##ARGN##_history[FUNCNAME##_fake.call_count], (const void *)&arg##ARGN, sizeof(arg##ARGN));

/**
 * @brief Increments the count of dropped argument histories for a fake function.
 *
 * Tracks how many argument values have been dropped due to exceeding history capacity.
 *
 * @param[in] FUNCNAME Name identifier for the fake function.
 */
#define HISTORY_DROPPED(FUNCNAME) \
  FUNCNAME##_fake.arg_histories_dropped++

/**
 * @brief Declares variables for managing return values in a fake function.
 *
 * Includes the current return value, a sequence of return values with length,
 * and the current index in the return sequence.
 *
 * @param[in] RETURN_TYPE The return type of the fake function.
 */
#define DECLARE_VALUE_FUNCTION_VARIABLES(RETURN_TYPE) \
  RETURN_TYPE return_val;                             \
  int return_val_seq_len;                             \
  int return_val_seq_idx;                             \
  RETURN_TYPE (*return_val_seq);

/**
 * @brief Declares variables for managing a sequence of custom fake functions.
 *
 * Tracks the length of the custom fake function sequence and the current index.
 */
#define DECLARE_CUSTOM_FAKE_SEQ_VARIABLES \
  int custom_fake_seq_len;                \
  int custom_fake_seq_idx;

/**
 * @brief Increments the call count of a fake function.
 *
 * @param[in] FUNCNAME Name identifier for the fake function.
 */
#define INCREMENT_CALL_COUNT(FUNCNAME) \
  FUNCNAME##_fake.call_count++

/**
 * @brief Registers a fake function call in the global call history.
 *
 * Records the provided function pointer in the global call history array
 * if there is available space.
 *
 * @param[in] function Function pointer to register.
 */
#define REGISTER_CALL(function)                    \
  if (eff.call_history_idx < EFF_CALL_HISTORY_LEN) \
    eff.call_history[eff.call_history_idx++] = (eff_function_t)(function);

/**
 * @brief Returns the appropriate fake function result, handling sequences.
 *
 * If a return value sequence is set, returns the next value in the sequence,
 * saving each returned value in the history. Once the sequence ends, continues
 * returning the last value. Otherwise, returns the single stored return value.
 *
 * @param[in] FUNCNAME Name identifier for the fake function.
 */
#define RETURN_FAKE_RESULT(FUNCNAME)                                                                         \
  if (FUNCNAME##_fake.return_val_seq_len)                                                                    \
  { /* then its a sequence */                                                                                \
    if (FUNCNAME##_fake.return_val_seq_idx < FUNCNAME##_fake.return_val_seq_len)                             \
    {                                                                                                        \
      SAVE_RET_HISTORY(FUNCNAME, FUNCNAME##_fake.return_val_seq[FUNCNAME##_fake.return_val_seq_idx])         \
      return FUNCNAME##_fake.return_val_seq[FUNCNAME##_fake.return_val_seq_idx++];                           \
    }                                                                                                        \
    SAVE_RET_HISTORY(FUNCNAME, FUNCNAME##_fake.return_val_seq[FUNCNAME##_fake.return_val_seq_len - 1])       \
    return FUNCNAME##_fake.return_val_seq[FUNCNAME##_fake.return_val_seq_len - 1]; /* return last element */ \
  }                                                                                                          \
  SAVE_RET_HISTORY(FUNCNAME, FUNCNAME##_fake.return_val)                                                     \
  return FUNCNAME##_fake.return_val;

/**
 * @brief Defines a reset function for a fake function.
 *
 * @param[in] FUNCNAME Name identifier of the fake function.
 */
#define DEFINE_RESET_FUNCTION(FUNCNAME)                                                                                                           \
  void FUNCNAME##_reset(void)                                                                                                                     \
  {                                                                                                                                               \
    /* NOLINTNEXTLINE(clang-analyzer-security.insecureAPI.DeprecatedOrUnsafeBufferHandling) */                                                    \
    memset((void *)&FUNCNAME##_fake, 0, sizeof(FUNCNAME##_fake) - sizeof(FUNCNAME##_fake.custom_fake) - sizeof(FUNCNAME##_fake.custom_fake_seq)); \
    FUNCNAME##_fake.custom_fake = NULL;                                                                                                           \
    FUNCNAME##_fake.custom_fake_seq = NULL;                                                                                                       \
    FUNCNAME##_fake.arg_history_len = EFF_ARG_HISTORY_LEN;                                                                                        \
  }

/**
 * @defgroup FakeFunctionMacros Fake function declaration and definition macros
 * @brief Macros to create fake functions with 0–10 arguments and void or non-void return values.
 *
 * @{
 */
#define DECLARE_FAKE_VOID_FUNC0(FUNCNAME)                       \
  typedef struct FUNCNAME##_Fake                                \
  {                                                             \
    DECLARE_ALL_FUNC_COMMON                                     \
    DECLARE_CUSTOM_FAKE_SEQ_VARIABLES                           \
    CUSTOM_EFF_FUNCTION_TEMPLATE(void, custom_fake, void);      \
    CUSTOM_EFF_FUNCTION_TEMPLATE(void, *custom_fake_seq, void); \
  } FUNCNAME##_Fake;                                            \
  extern FUNCNAME##_Fake FUNCNAME##_fake;                       \
  void FUNCNAME##_reset(void);                                  \
  void EFF_GCC_FUNCTION_ATTRIBUTES FUNCNAME(void);

#define DEFINE_FAKE_VOID_FUNC0(FUNCNAME)                                             \
  FUNCNAME##_Fake FUNCNAME##_fake;                                                   \
  void EFF_GCC_FUNCTION_ATTRIBUTES FUNCNAME(void)                                    \
  {                                                                                  \
    if (ROOM_FOR_MORE_HISTORY(FUNCNAME))                                             \
    {                                                                                \
    }                                                                                \
    else                                                                             \
    {                                                                                \
      HISTORY_DROPPED(FUNCNAME);                                                     \
    }                                                                                \
    INCREMENT_CALL_COUNT(FUNCNAME);                                                  \
    REGISTER_CALL(FUNCNAME);                                                         \
    if (FUNCNAME##_fake.custom_fake_seq_len)                                         \
    {                                                                                \
      if (FUNCNAME##_fake.custom_fake_seq_idx < FUNCNAME##_fake.custom_fake_seq_len) \
      {                                                                              \
        FUNCNAME##_fake.custom_fake_seq[FUNCNAME##_fake.custom_fake_seq_idx++]();    \
      }                                                                              \
      else                                                                           \
      {                                                                              \
        FUNCNAME##_fake.custom_fake_seq[FUNCNAME##_fake.custom_fake_seq_len - 1]();  \
      }                                                                              \
    }                                                                                \
    if (FUNCNAME##_fake.custom_fake != NULL)                                         \
    {                                                                                \
      FUNCNAME##_fake.custom_fake();                                                 \
    }                                                                                \
  }                                                                                  \
  DEFINE_RESET_FUNCTION(FUNCNAME)

#define FAKE_VOID_FUNC0(FUNCNAME)   \
  DECLARE_FAKE_VOID_FUNC0(FUNCNAME) \
  DEFINE_FAKE_VOID_FUNC0(FUNCNAME)

#define DECLARE_FAKE_VOID_FUNC1(FUNCNAME, ARG0_TYPE)                 \
  typedef struct FUNCNAME##_Fake                                     \
  {                                                                  \
    DECLARE_ARG(ARG0_TYPE, 0, FUNCNAME)                              \
    DECLARE_ALL_FUNC_COMMON                                          \
    DECLARE_CUSTOM_FAKE_SEQ_VARIABLES                                \
    CUSTOM_EFF_FUNCTION_TEMPLATE(void, custom_fake, ARG0_TYPE);      \
    CUSTOM_EFF_FUNCTION_TEMPLATE(void, *custom_fake_seq, ARG0_TYPE); \
  } FUNCNAME##_Fake;                                                 \
  extern FUNCNAME##_Fake FUNCNAME##_fake;                            \
  void FUNCNAME##_reset(void);                                       \
  void EFF_GCC_FUNCTION_ATTRIBUTES FUNCNAME(ARG0_TYPE arg0);

#define DEFINE_FAKE_VOID_FUNC1(FUNCNAME, ARG0_TYPE)                                     \
  FUNCNAME##_Fake FUNCNAME##_fake;                                                      \
  void EFF_GCC_FUNCTION_ATTRIBUTES FUNCNAME(ARG0_TYPE arg0)                             \
  {                                                                                     \
    SAVE_ARG(FUNCNAME, 0);                                                              \
    if (ROOM_FOR_MORE_HISTORY(FUNCNAME))                                                \
    {                                                                                   \
      SAVE_ARG_HISTORY(FUNCNAME, 0);                                                    \
    }                                                                                   \
    else                                                                                \
    {                                                                                   \
      HISTORY_DROPPED(FUNCNAME);                                                        \
    }                                                                                   \
    INCREMENT_CALL_COUNT(FUNCNAME);                                                     \
    REGISTER_CALL(FUNCNAME);                                                            \
    if (FUNCNAME##_fake.custom_fake_seq_len)                                            \
    {                                                                                   \
      if (FUNCNAME##_fake.custom_fake_seq_idx < FUNCNAME##_fake.custom_fake_seq_len)    \
      {                                                                                 \
        FUNCNAME##_fake.custom_fake_seq[FUNCNAME##_fake.custom_fake_seq_idx++](arg0);   \
      }                                                                                 \
      else                                                                              \
      {                                                                                 \
        FUNCNAME##_fake.custom_fake_seq[FUNCNAME##_fake.custom_fake_seq_len - 1](arg0); \
      }                                                                                 \
    }                                                                                   \
    if (FUNCNAME##_fake.custom_fake != NULL)                                            \
    {                                                                                   \
      FUNCNAME##_fake.custom_fake(arg0);                                                \
    }                                                                                   \
  }                                                                                     \
  DEFINE_RESET_FUNCTION(FUNCNAME)

#define FAKE_VOID_FUNC1(FUNCNAME, ARG0_TYPE)   \
  DECLARE_FAKE_VOID_FUNC1(FUNCNAME, ARG0_TYPE) \
  DEFINE_FAKE_VOID_FUNC1(FUNCNAME, ARG0_TYPE)

#define DECLARE_FAKE_VOID_FUNC2(FUNCNAME, ARG0_TYPE, ARG1_TYPE)                 \
  typedef struct FUNCNAME##_Fake                                                \
  {                                                                             \
    DECLARE_ARG(ARG0_TYPE, 0, FUNCNAME)                                         \
    DECLARE_ARG(ARG1_TYPE, 1, FUNCNAME)                                         \
    DECLARE_ALL_FUNC_COMMON                                                     \
    DECLARE_CUSTOM_FAKE_SEQ_VARIABLES                                           \
    CUSTOM_EFF_FUNCTION_TEMPLATE(void, custom_fake, ARG0_TYPE, ARG1_TYPE);      \
    CUSTOM_EFF_FUNCTION_TEMPLATE(void, *custom_fake_seq, ARG0_TYPE, ARG1_TYPE); \
  } FUNCNAME##_Fake;                                                            \
  extern FUNCNAME##_Fake FUNCNAME##_fake;                                       \
  void FUNCNAME##_reset(void);                                                  \
  void EFF_GCC_FUNCTION_ATTRIBUTES FUNCNAME(ARG0_TYPE arg0, ARG1_TYPE arg1);

#define DEFINE_FAKE_VOID_FUNC2(FUNCNAME, ARG0_TYPE, ARG1_TYPE)                                \
  FUNCNAME##_Fake FUNCNAME##_fake;                                                            \
  void EFF_GCC_FUNCTION_ATTRIBUTES FUNCNAME(ARG0_TYPE arg0, ARG1_TYPE arg1)                   \
  {                                                                                           \
    SAVE_ARG(FUNCNAME, 0);                                                                    \
    SAVE_ARG(FUNCNAME, 1);                                                                    \
    if (ROOM_FOR_MORE_HISTORY(FUNCNAME))                                                      \
    {                                                                                         \
      SAVE_ARG_HISTORY(FUNCNAME, 0);                                                          \
      SAVE_ARG_HISTORY(FUNCNAME, 1);                                                          \
    }                                                                                         \
    else                                                                                      \
    {                                                                                         \
      HISTORY_DROPPED(FUNCNAME);                                                              \
    }                                                                                         \
    INCREMENT_CALL_COUNT(FUNCNAME);                                                           \
    REGISTER_CALL(FUNCNAME);                                                                  \
    if (FUNCNAME##_fake.custom_fake_seq_len)                                                  \
    {                                                                                         \
      if (FUNCNAME##_fake.custom_fake_seq_idx < FUNCNAME##_fake.custom_fake_seq_len)          \
      {                                                                                       \
        FUNCNAME##_fake.custom_fake_seq[FUNCNAME##_fake.custom_fake_seq_idx++](arg0, arg1);   \
      }                                                                                       \
      else                                                                                    \
      {                                                                                       \
        FUNCNAME##_fake.custom_fake_seq[FUNCNAME##_fake.custom_fake_seq_len - 1](arg0, arg1); \
      }                                                                                       \
    }                                                                                         \
    if (FUNCNAME##_fake.custom_fake != NULL)                                                  \
    {                                                                                         \
      FUNCNAME##_fake.custom_fake(arg0, arg1);                                                \
    }                                                                                         \
  }                                                                                           \
  DEFINE_RESET_FUNCTION(FUNCNAME)

#define FAKE_VOID_FUNC2(FUNCNAME, ARG0_TYPE, ARG1_TYPE)   \
  DECLARE_FAKE_VOID_FUNC2(FUNCNAME, ARG0_TYPE, ARG1_TYPE) \
  DEFINE_FAKE_VOID_FUNC2(FUNCNAME, ARG0_TYPE, ARG1_TYPE)

#define DECLARE_FAKE_VOID_FUNC3(FUNCNAME, ARG0_TYPE, ARG1_TYPE, ARG2_TYPE)                 \
  typedef struct FUNCNAME##_Fake                                                           \
  {                                                                                        \
    DECLARE_ARG(ARG0_TYPE, 0, FUNCNAME)                                                    \
    DECLARE_ARG(ARG1_TYPE, 1, FUNCNAME)                                                    \
    DECLARE_ARG(ARG2_TYPE, 2, FUNCNAME)                                                    \
    DECLARE_ALL_FUNC_COMMON                                                                \
    DECLARE_CUSTOM_FAKE_SEQ_VARIABLES                                                      \
    CUSTOM_EFF_FUNCTION_TEMPLATE(void, custom_fake, ARG0_TYPE, ARG1_TYPE, ARG2_TYPE);      \
    CUSTOM_EFF_FUNCTION_TEMPLATE(void, *custom_fake_seq, ARG0_TYPE, ARG1_TYPE, ARG2_TYPE); \
  } FUNCNAME##_Fake;                                                                       \
  extern FUNCNAME##_Fake FUNCNAME##_fake;                                                  \
  void FUNCNAME##_reset(void);                                                             \
  void EFF_GCC_FUNCTION_ATTRIBUTES FUNCNAME(ARG0_TYPE arg0, ARG1_TYPE arg1, ARG2_TYPE arg2);

#define DEFINE_FAKE_VOID_FUNC3(FUNCNAME, ARG0_TYPE, ARG1_TYPE, ARG2_TYPE)                           \
  FUNCNAME##_Fake FUNCNAME##_fake;                                                                  \
  void EFF_GCC_FUNCTION_ATTRIBUTES FUNCNAME(ARG0_TYPE arg0, ARG1_TYPE arg1, ARG2_TYPE arg2)         \
  {                                                                                                 \
    SAVE_ARG(FUNCNAME, 0);                                                                          \
    SAVE_ARG(FUNCNAME, 1);                                                                          \
    SAVE_ARG(FUNCNAME, 2);                                                                          \
    if (ROOM_FOR_MORE_HISTORY(FUNCNAME))                                                            \
    {                                                                                               \
      SAVE_ARG_HISTORY(FUNCNAME, 0);                                                                \
      SAVE_ARG_HISTORY(FUNCNAME, 1);                                                                \
      SAVE_ARG_HISTORY(FUNCNAME, 2);                                                                \
    }                                                                                               \
    else                                                                                            \
    {                                                                                               \
      HISTORY_DROPPED(FUNCNAME);                                                                    \
    }                                                                                               \
    INCREMENT_CALL_COUNT(FUNCNAME);                                                                 \
    REGISTER_CALL(FUNCNAME);                                                                        \
    if (FUNCNAME##_fake.custom_fake_seq_len)                                                        \
    {                                                                                               \
      if (FUNCNAME##_fake.custom_fake_seq_idx < FUNCNAME##_fake.custom_fake_seq_len)                \
      {                                                                                             \
        FUNCNAME##_fake.custom_fake_seq[FUNCNAME##_fake.custom_fake_seq_idx++](arg0, arg1, arg2);   \
      }                                                                                             \
      else                                                                                          \
      {                                                                                             \
        FUNCNAME##_fake.custom_fake_seq[FUNCNAME##_fake.custom_fake_seq_len - 1](arg0, arg1, arg2); \
      }                                                                                             \
    }                                                                                               \
    if (FUNCNAME##_fake.custom_fake != NULL)                                                        \
    {                                                                                               \
      FUNCNAME##_fake.custom_fake(arg0, arg1, arg2);                                                \
    }                                                                                               \
  }                                                                                                 \
  DEFINE_RESET_FUNCTION(FUNCNAME)

#define FAKE_VOID_FUNC3(FUNCNAME, ARG0_TYPE, ARG1_TYPE, ARG2_TYPE)   \
  DECLARE_FAKE_VOID_FUNC3(FUNCNAME, ARG0_TYPE, ARG1_TYPE, ARG2_TYPE) \
  DEFINE_FAKE_VOID_FUNC3(FUNCNAME, ARG0_TYPE, ARG1_TYPE, ARG2_TYPE)

#define DECLARE_FAKE_VOID_FUNC4(FUNCNAME, ARG0_TYPE, ARG1_TYPE, ARG2_TYPE, ARG3_TYPE)                 \
  typedef struct FUNCNAME##_Fake                                                                      \
  {                                                                                                   \
    DECLARE_ARG(ARG0_TYPE, 0, FUNCNAME)                                                               \
    DECLARE_ARG(ARG1_TYPE, 1, FUNCNAME)                                                               \
    DECLARE_ARG(ARG2_TYPE, 2, FUNCNAME)                                                               \
    DECLARE_ARG(ARG3_TYPE, 3, FUNCNAME)                                                               \
    DECLARE_ALL_FUNC_COMMON                                                                           \
    DECLARE_CUSTOM_FAKE_SEQ_VARIABLES                                                                 \
    CUSTOM_EFF_FUNCTION_TEMPLATE(void, custom_fake, ARG0_TYPE, ARG1_TYPE, ARG2_TYPE, ARG3_TYPE);      \
    CUSTOM_EFF_FUNCTION_TEMPLATE(void, *custom_fake_seq, ARG0_TYPE, ARG1_TYPE, ARG2_TYPE, ARG3_TYPE); \
  } FUNCNAME##_Fake;                                                                                  \
  extern FUNCNAME##_Fake FUNCNAME##_fake;                                                             \
  void FUNCNAME##_reset(void);                                                                        \
  void EFF_GCC_FUNCTION_ATTRIBUTES FUNCNAME(ARG0_TYPE arg0, ARG1_TYPE arg1, ARG2_TYPE arg2, ARG3_TYPE arg3);

#define DEFINE_FAKE_VOID_FUNC4(FUNCNAME, ARG0_TYPE, ARG1_TYPE, ARG2_TYPE, ARG3_TYPE)                        \
  FUNCNAME##_Fake FUNCNAME##_fake;                                                                          \
  void EFF_GCC_FUNCTION_ATTRIBUTES FUNCNAME(ARG0_TYPE arg0, ARG1_TYPE arg1, ARG2_TYPE arg2, ARG3_TYPE arg3) \
  {                                                                                                         \
    SAVE_ARG(FUNCNAME, 0);                                                                                  \
    SAVE_ARG(FUNCNAME, 1);                                                                                  \
    SAVE_ARG(FUNCNAME, 2);                                                                                  \
    SAVE_ARG(FUNCNAME, 3);                                                                                  \
    if (ROOM_FOR_MORE_HISTORY(FUNCNAME))                                                                    \
    {                                                                                                       \
      SAVE_ARG_HISTORY(FUNCNAME, 0);                                                                        \
      SAVE_ARG_HISTORY(FUNCNAME, 1);                                                                        \
      SAVE_ARG_HISTORY(FUNCNAME, 2);                                                                        \
      SAVE_ARG_HISTORY(FUNCNAME, 3);                                                                        \
    }                                                                                                       \
    else                                                                                                    \
    {                                                                                                       \
      HISTORY_DROPPED(FUNCNAME);                                                                            \
    }                                                                                                       \
    INCREMENT_CALL_COUNT(FUNCNAME);                                                                         \
    REGISTER_CALL(FUNCNAME);                                                                                \
    if (FUNCNAME##_fake.custom_fake_seq_len)                                                                \
    {                                                                                                       \
      if (FUNCNAME##_fake.custom_fake_seq_idx < FUNCNAME##_fake.custom_fake_seq_len)                        \
      {                                                                                                     \
        FUNCNAME##_fake.custom_fake_seq[FUNCNAME##_fake.custom_fake_seq_idx++](arg0, arg1, arg2, arg3);     \
      }                                                                                                     \
      else                                                                                                  \
      {                                                                                                     \
        FUNCNAME##_fake.custom_fake_seq[FUNCNAME##_fake.custom_fake_seq_len - 1](arg0, arg1, arg2, arg3);   \
      }                                                                                                     \
    }                                                                                                       \
    if (FUNCNAME##_fake.custom_fake != NULL)                                                                \
    {                                                                                                       \
      FUNCNAME##_fake.custom_fake(arg0, arg1, arg2, arg3);                                                  \
    }                                                                                                       \
  }                                                                                                         \
  DEFINE_RESET_FUNCTION(FUNCNAME)

#define FAKE_VOID_FUNC4(FUNCNAME, ARG0_TYPE, ARG1_TYPE, ARG2_TYPE, ARG3_TYPE)   \
  DECLARE_FAKE_VOID_FUNC4(FUNCNAME, ARG0_TYPE, ARG1_TYPE, ARG2_TYPE, ARG3_TYPE) \
  DEFINE_FAKE_VOID_FUNC4(FUNCNAME, ARG0_TYPE, ARG1_TYPE, ARG2_TYPE, ARG3_TYPE)

#define DECLARE_FAKE_VOID_FUNC5(FUNCNAME, ARG0_TYPE, ARG1_TYPE, ARG2_TYPE, ARG3_TYPE, ARG4_TYPE)                 \
  typedef struct FUNCNAME##_Fake                                                                                 \
  {                                                                                                              \
    DECLARE_ARG(ARG0_TYPE, 0, FUNCNAME)                                                                          \
    DECLARE_ARG(ARG1_TYPE, 1, FUNCNAME)                                                                          \
    DECLARE_ARG(ARG2_TYPE, 2, FUNCNAME)                                                                          \
    DECLARE_ARG(ARG3_TYPE, 3, FUNCNAME)                                                                          \
    DECLARE_ARG(ARG4_TYPE, 4, FUNCNAME)                                                                          \
    DECLARE_ALL_FUNC_COMMON                                                                                      \
    DECLARE_CUSTOM_FAKE_SEQ_VARIABLES                                                                            \
    CUSTOM_EFF_FUNCTION_TEMPLATE(void, custom_fake, ARG0_TYPE, ARG1_TYPE, ARG2_TYPE, ARG3_TYPE, ARG4_TYPE);      \
    CUSTOM_EFF_FUNCTION_TEMPLATE(void, *custom_fake_seq, ARG0_TYPE, ARG1_TYPE, ARG2_TYPE, ARG3_TYPE, ARG4_TYPE); \
  } FUNCNAME##_Fake;                                                                                             \
  extern FUNCNAME##_Fake FUNCNAME##_fake;                                                                        \
  void FUNCNAME##_reset(void);                                                                                   \
  void EFF_GCC_FUNCTION_ATTRIBUTES FUNCNAME(ARG0_TYPE arg0, ARG1_TYPE arg1, ARG2_TYPE arg2, ARG3_TYPE arg3, ARG4_TYPE arg4);

#define DEFINE_FAKE_VOID_FUNC5(FUNCNAME, ARG0_TYPE, ARG1_TYPE, ARG2_TYPE, ARG3_TYPE, ARG4_TYPE)                             \
  FUNCNAME##_Fake FUNCNAME##_fake;                                                                                          \
  void EFF_GCC_FUNCTION_ATTRIBUTES FUNCNAME(ARG0_TYPE arg0, ARG1_TYPE arg1, ARG2_TYPE arg2, ARG3_TYPE arg3, ARG4_TYPE arg4) \
  {                                                                                                                         \
    SAVE_ARG(FUNCNAME, 0);                                                                                                  \
    SAVE_ARG(FUNCNAME, 1);                                                                                                  \
    SAVE_ARG(FUNCNAME, 2);                                                                                                  \
    SAVE_ARG(FUNCNAME, 3);                                                                                                  \
    SAVE_ARG(FUNCNAME, 4);                                                                                                  \
    if (ROOM_FOR_MORE_HISTORY(FUNCNAME))                                                                                    \
    {                                                                                                                       \
      SAVE_ARG_HISTORY(FUNCNAME, 0);                                                                                        \
      SAVE_ARG_HISTORY(FUNCNAME, 1);                                                                                        \
      SAVE_ARG_HISTORY(FUNCNAME, 2);                                                                                        \
      SAVE_ARG_HISTORY(FUNCNAME, 3);                                                                                        \
      SAVE_ARG_HISTORY(FUNCNAME, 4);                                                                                        \
    }                                                                                                                       \
    else                                                                                                                    \
    {                                                                                                                       \
      HISTORY_DROPPED(FUNCNAME);                                                                                            \
    }                                                                                                                       \
    INCREMENT_CALL_COUNT(FUNCNAME);                                                                                         \
    REGISTER_CALL(FUNCNAME);                                                                                                \
    if (FUNCNAME##_fake.custom_fake_seq_len)                                                                                \
    {                                                                                                                       \
      if (FUNCNAME##_fake.custom_fake_seq_idx < FUNCNAME##_fake.custom_fake_seq_len)                                        \
      {                                                                                                                     \
        FUNCNAME##_fake.custom_fake_seq[FUNCNAME##_fake.custom_fake_seq_idx++](arg0, arg1, arg2, arg3, arg4);               \
      }                                                                                                                     \
      else                                                                                                                  \
      {                                                                                                                     \
        FUNCNAME##_fake.custom_fake_seq[FUNCNAME##_fake.custom_fake_seq_len - 1](arg0, arg1, arg2, arg3, arg4);             \
      }                                                                                                                     \
    }                                                                                                                       \
    if (FUNCNAME##_fake.custom_fake != NULL)                                                                                \
    {                                                                                                                       \
      FUNCNAME##_fake.custom_fake(arg0, arg1, arg2, arg3, arg4);                                                            \
    }                                                                                                                       \
  }                                                                                                                         \
  DEFINE_RESET_FUNCTION(FUNCNAME)

#define FAKE_VOID_FUNC5(FUNCNAME, ARG0_TYPE, ARG1_TYPE, ARG2_TYPE, ARG3_TYPE, ARG4_TYPE)   \
  DECLARE_FAKE_VOID_FUNC5(FUNCNAME, ARG0_TYPE, ARG1_TYPE, ARG2_TYPE, ARG3_TYPE, ARG4_TYPE) \
  DEFINE_FAKE_VOID_FUNC5(FUNCNAME, ARG0_TYPE, ARG1_TYPE, ARG2_TYPE, ARG3_TYPE, ARG4_TYPE)

#define DECLARE_FAKE_VOID_FUNC6(FUNCNAME, ARG0_TYPE, ARG1_TYPE, ARG2_TYPE, ARG3_TYPE, ARG4_TYPE, ARG5_TYPE)                 \
  typedef struct FUNCNAME##_Fake                                                                                            \
  {                                                                                                                         \
    DECLARE_ARG(ARG0_TYPE, 0, FUNCNAME)                                                                                     \
    DECLARE_ARG(ARG1_TYPE, 1, FUNCNAME)                                                                                     \
    DECLARE_ARG(ARG2_TYPE, 2, FUNCNAME)                                                                                     \
    DECLARE_ARG(ARG3_TYPE, 3, FUNCNAME)                                                                                     \
    DECLARE_ARG(ARG4_TYPE, 4, FUNCNAME)                                                                                     \
    DECLARE_ARG(ARG5_TYPE, 5, FUNCNAME)                                                                                     \
    DECLARE_ALL_FUNC_COMMON                                                                                                 \
    DECLARE_CUSTOM_FAKE_SEQ_VARIABLES                                                                                       \
    CUSTOM_EFF_FUNCTION_TEMPLATE(void, custom_fake, ARG0_TYPE, ARG1_TYPE, ARG2_TYPE, ARG3_TYPE, ARG4_TYPE, ARG5_TYPE);      \
    CUSTOM_EFF_FUNCTION_TEMPLATE(void, *custom_fake_seq, ARG0_TYPE, ARG1_TYPE, ARG2_TYPE, ARG3_TYPE, ARG4_TYPE, ARG5_TYPE); \
  } FUNCNAME##_Fake;                                                                                                        \
  extern FUNCNAME##_Fake FUNCNAME##_fake;                                                                                   \
  void FUNCNAME##_reset(void);                                                                                              \
  void EFF_GCC_FUNCTION_ATTRIBUTES FUNCNAME(ARG0_TYPE arg0, ARG1_TYPE arg1, ARG2_TYPE arg2, ARG3_TYPE arg3, ARG4_TYPE arg4, ARG5_TYPE arg5);

#define DEFINE_FAKE_VOID_FUNC6(FUNCNAME, ARG0_TYPE, ARG1_TYPE, ARG2_TYPE, ARG3_TYPE, ARG4_TYPE, ARG5_TYPE)                                  \
  FUNCNAME##_Fake FUNCNAME##_fake;                                                                                                          \
  void EFF_GCC_FUNCTION_ATTRIBUTES FUNCNAME(ARG0_TYPE arg0, ARG1_TYPE arg1, ARG2_TYPE arg2, ARG3_TYPE arg3, ARG4_TYPE arg4, ARG5_TYPE arg5) \
  {                                                                                                                                         \
    SAVE_ARG(FUNCNAME, 0);                                                                                                                  \
    SAVE_ARG(FUNCNAME, 1);                                                                                                                  \
    SAVE_ARG(FUNCNAME, 2);                                                                                                                  \
    SAVE_ARG(FUNCNAME, 3);                                                                                                                  \
    SAVE_ARG(FUNCNAME, 4);                                                                                                                  \
    SAVE_ARG(FUNCNAME, 5);                                                                                                                  \
    if (ROOM_FOR_MORE_HISTORY(FUNCNAME))                                                                                                    \
    {                                                                                                                                       \
      SAVE_ARG_HISTORY(FUNCNAME, 0);                                                                                                        \
      SAVE_ARG_HISTORY(FUNCNAME, 1);                                                                                                        \
      SAVE_ARG_HISTORY(FUNCNAME, 2);                                                                                                        \
      SAVE_ARG_HISTORY(FUNCNAME, 3);                                                                                                        \
      SAVE_ARG_HISTORY(FUNCNAME, 4);                                                                                                        \
      SAVE_ARG_HISTORY(FUNCNAME, 5);                                                                                                        \
    }                                                                                                                                       \
    else                                                                                                                                    \
    {                                                                                                                                       \
      HISTORY_DROPPED(FUNCNAME);                                                                                                            \
    }                                                                                                                                       \
    INCREMENT_CALL_COUNT(FUNCNAME);                                                                                                         \
    REGISTER_CALL(FUNCNAME);                                                                                                                \
    if (FUNCNAME##_fake.custom_fake_seq_len)                                                                                                \
    {                                                                                                                                       \
      if (FUNCNAME##_fake.custom_fake_seq_idx < FUNCNAME##_fake.custom_fake_seq_len)                                                        \
      {                                                                                                                                     \
        FUNCNAME##_fake.custom_fake_seq[FUNCNAME##_fake.custom_fake_seq_idx++](arg0, arg1, arg2, arg3, arg4, arg5);                         \
      }                                                                                                                                     \
      else                                                                                                                                  \
      {                                                                                                                                     \
        FUNCNAME##_fake.custom_fake_seq[FUNCNAME##_fake.custom_fake_seq_len - 1](arg0, arg1, arg2, arg3, arg4, arg5);                       \
      }                                                                                                                                     \
    }                                                                                                                                       \
    if (FUNCNAME##_fake.custom_fake != NULL)                                                                                                \
    {                                                                                                                                       \
      FUNCNAME##_fake.custom_fake(arg0, arg1, arg2, arg3, arg4, arg5);                                                                      \
    }                                                                                                                                       \
  }                                                                                                                                         \
  DEFINE_RESET_FUNCTION(FUNCNAME)

#define FAKE_VOID_FUNC6(FUNCNAME, ARG0_TYPE, ARG1_TYPE, ARG2_TYPE, ARG3_TYPE, ARG4_TYPE, ARG5_TYPE)   \
  DECLARE_FAKE_VOID_FUNC6(FUNCNAME, ARG0_TYPE, ARG1_TYPE, ARG2_TYPE, ARG3_TYPE, ARG4_TYPE, ARG5_TYPE) \
  DEFINE_FAKE_VOID_FUNC6(FUNCNAME, ARG0_TYPE, ARG1_TYPE, ARG2_TYPE, ARG3_TYPE, ARG4_TYPE, ARG5_TYPE)

#define DECLARE_FAKE_VOID_FUNC7(FUNCNAME, ARG0_TYPE, ARG1_TYPE, ARG2_TYPE, ARG3_TYPE, ARG4_TYPE, ARG5_TYPE, ARG6_TYPE)                 \
  typedef struct FUNCNAME##_Fake                                                                                                       \
  {                                                                                                                                    \
    DECLARE_ARG(ARG0_TYPE, 0, FUNCNAME)                                                                                                \
    DECLARE_ARG(ARG1_TYPE, 1, FUNCNAME)                                                                                                \
    DECLARE_ARG(ARG2_TYPE, 2, FUNCNAME)                                                                                                \
    DECLARE_ARG(ARG3_TYPE, 3, FUNCNAME)                                                                                                \
    DECLARE_ARG(ARG4_TYPE, 4, FUNCNAME)                                                                                                \
    DECLARE_ARG(ARG5_TYPE, 5, FUNCNAME)                                                                                                \
    DECLARE_ARG(ARG6_TYPE, 6, FUNCNAME)                                                                                                \
    DECLARE_ALL_FUNC_COMMON                                                                                                            \
    DECLARE_CUSTOM_FAKE_SEQ_VARIABLES                                                                                                  \
    CUSTOM_EFF_FUNCTION_TEMPLATE(void, custom_fake, ARG0_TYPE, ARG1_TYPE, ARG2_TYPE, ARG3_TYPE, ARG4_TYPE, ARG5_TYPE, ARG6_TYPE);      \
    CUSTOM_EFF_FUNCTION_TEMPLATE(void, *custom_fake_seq, ARG0_TYPE, ARG1_TYPE, ARG2_TYPE, ARG3_TYPE, ARG4_TYPE, ARG5_TYPE, ARG6_TYPE); \
  } FUNCNAME##_Fake;                                                                                                                   \
  extern FUNCNAME##_Fake FUNCNAME##_fake;                                                                                              \
  void FUNCNAME##_reset(void);                                                                                                         \
  void EFF_GCC_FUNCTION_ATTRIBUTES FUNCNAME(ARG0_TYPE arg0, ARG1_TYPE arg1, ARG2_TYPE arg2, ARG3_TYPE arg3, ARG4_TYPE arg4, ARG5_TYPE arg5, ARG6_TYPE arg6);

#define DEFINE_FAKE_VOID_FUNC7(FUNCNAME, ARG0_TYPE, ARG1_TYPE, ARG2_TYPE, ARG3_TYPE, ARG4_TYPE, ARG5_TYPE, ARG6_TYPE)                                       \
  FUNCNAME##_Fake FUNCNAME##_fake;                                                                                                                          \
  void EFF_GCC_FUNCTION_ATTRIBUTES FUNCNAME(ARG0_TYPE arg0, ARG1_TYPE arg1, ARG2_TYPE arg2, ARG3_TYPE arg3, ARG4_TYPE arg4, ARG5_TYPE arg5, ARG6_TYPE arg6) \
  {                                                                                                                                                         \
    SAVE_ARG(FUNCNAME, 0);                                                                                                                                  \
    SAVE_ARG(FUNCNAME, 1);                                                                                                                                  \
    SAVE_ARG(FUNCNAME, 2);                                                                                                                                  \
    SAVE_ARG(FUNCNAME, 3);                                                                                                                                  \
    SAVE_ARG(FUNCNAME, 4);                                                                                                                                  \
    SAVE_ARG(FUNCNAME, 5);                                                                                                                                  \
    SAVE_ARG(FUNCNAME, 6);                                                                                                                                  \
    if (ROOM_FOR_MORE_HISTORY(FUNCNAME))                                                                                                                    \
    {                                                                                                                                                       \
      SAVE_ARG_HISTORY(FUNCNAME, 0);                                                                                                                        \
      SAVE_ARG_HISTORY(FUNCNAME, 1);                                                                                                                        \
      SAVE_ARG_HISTORY(FUNCNAME, 2);                                                                                                                        \
      SAVE_ARG_HISTORY(FUNCNAME, 3);                                                                                                                        \
      SAVE_ARG_HISTORY(FUNCNAME, 4);                                                                                                                        \
      SAVE_ARG_HISTORY(FUNCNAME, 5);                                                                                                                        \
      SAVE_ARG_HISTORY(FUNCNAME, 6);                                                                                                                        \
    }                                                                                                                                                       \
    else                                                                                                                                                    \
    {                                                                                                                                                       \
      HISTORY_DROPPED(FUNCNAME);                                                                                                                            \
    }                                                                                                                                                       \
    INCREMENT_CALL_COUNT(FUNCNAME);                                                                                                                         \
    REGISTER_CALL(FUNCNAME);                                                                                                                                \
    if (FUNCNAME##_fake.custom_fake_seq_len)                                                                                                                \
    {                                                                                                                                                       \
      if (FUNCNAME##_fake.custom_fake_seq_idx < FUNCNAME##_fake.custom_fake_seq_len)                                                                        \
      {                                                                                                                                                     \
        FUNCNAME##_fake.custom_fake_seq[FUNCNAME##_fake.custom_fake_seq_idx++](arg0, arg1, arg2, arg3, arg4, arg5, arg6);                                   \
      }                                                                                                                                                     \
      else                                                                                                                                                  \
      {                                                                                                                                                     \
        FUNCNAME##_fake.custom_fake_seq[FUNCNAME##_fake.custom_fake_seq_len - 1](arg0, arg1, arg2, arg3, arg4, arg5, arg6);                                 \
      }                                                                                                                                                     \
    }                                                                                                                                                       \
    if (FUNCNAME##_fake.custom_fake != NULL)                                                                                                                \
    {                                                                                                                                                       \
      FUNCNAME##_fake.custom_fake(arg0, arg1, arg2, arg3, arg4, arg5, arg6);                                                                                \
    }                                                                                                                                                       \
  }                                                                                                                                                         \
  DEFINE_RESET_FUNCTION(FUNCNAME)

#define FAKE_VOID_FUNC7(FUNCNAME, ARG0_TYPE, ARG1_TYPE, ARG2_TYPE, ARG3_TYPE, ARG4_TYPE, ARG5_TYPE, ARG6_TYPE)   \
  DECLARE_FAKE_VOID_FUNC7(FUNCNAME, ARG0_TYPE, ARG1_TYPE, ARG2_TYPE, ARG3_TYPE, ARG4_TYPE, ARG5_TYPE, ARG6_TYPE) \
  DEFINE_FAKE_VOID_FUNC7(FUNCNAME, ARG0_TYPE, ARG1_TYPE, ARG2_TYPE, ARG3_TYPE, ARG4_TYPE, ARG5_TYPE, ARG6_TYPE)

#define DECLARE_FAKE_VOID_FUNC8(FUNCNAME, ARG0_TYPE, ARG1_TYPE, ARG2_TYPE, ARG3_TYPE, ARG4_TYPE, ARG5_TYPE, ARG6_TYPE, ARG7_TYPE)                 \
  typedef struct FUNCNAME##_Fake                                                                                                                  \
  {                                                                                                                                               \
    DECLARE_ARG(ARG0_TYPE, 0, FUNCNAME)                                                                                                           \
    DECLARE_ARG(ARG1_TYPE, 1, FUNCNAME)                                                                                                           \
    DECLARE_ARG(ARG2_TYPE, 2, FUNCNAME)                                                                                                           \
    DECLARE_ARG(ARG3_TYPE, 3, FUNCNAME)                                                                                                           \
    DECLARE_ARG(ARG4_TYPE, 4, FUNCNAME)                                                                                                           \
    DECLARE_ARG(ARG5_TYPE, 5, FUNCNAME)                                                                                                           \
    DECLARE_ARG(ARG6_TYPE, 6, FUNCNAME)                                                                                                           \
    DECLARE_ARG(ARG7_TYPE, 7, FUNCNAME)                                                                                                           \
    DECLARE_ALL_FUNC_COMMON                                                                                                                       \
    DECLARE_CUSTOM_FAKE_SEQ_VARIABLES                                                                                                             \
    CUSTOM_EFF_FUNCTION_TEMPLATE(void, custom_fake, ARG0_TYPE, ARG1_TYPE, ARG2_TYPE, ARG3_TYPE, ARG4_TYPE, ARG5_TYPE, ARG6_TYPE, ARG7_TYPE);      \
    CUSTOM_EFF_FUNCTION_TEMPLATE(void, *custom_fake_seq, ARG0_TYPE, ARG1_TYPE, ARG2_TYPE, ARG3_TYPE, ARG4_TYPE, ARG5_TYPE, ARG6_TYPE, ARG7_TYPE); \
  } FUNCNAME##_Fake;                                                                                                                              \
  extern FUNCNAME##_Fake FUNCNAME##_fake;                                                                                                         \
  void FUNCNAME##_reset(void);                                                                                                                    \
  void EFF_GCC_FUNCTION_ATTRIBUTES FUNCNAME(ARG0_TYPE arg0, ARG1_TYPE arg1, ARG2_TYPE arg2, ARG3_TYPE arg3, ARG4_TYPE arg4, ARG5_TYPE arg5, ARG6_TYPE arg6, ARG7_TYPE arg7);

#define DEFINE_FAKE_VOID_FUNC8(FUNCNAME, ARG0_TYPE, ARG1_TYPE, ARG2_TYPE, ARG3_TYPE, ARG4_TYPE, ARG5_TYPE, ARG6_TYPE, ARG7_TYPE)                                            \
  FUNCNAME##_Fake FUNCNAME##_fake;                                                                                                                                          \
  void EFF_GCC_FUNCTION_ATTRIBUTES FUNCNAME(ARG0_TYPE arg0, ARG1_TYPE arg1, ARG2_TYPE arg2, ARG3_TYPE arg3, ARG4_TYPE arg4, ARG5_TYPE arg5, ARG6_TYPE arg6, ARG7_TYPE arg7) \
  {                                                                                                                                                                         \
    SAVE_ARG(FUNCNAME, 0);                                                                                                                                                  \
    SAVE_ARG(FUNCNAME, 1);                                                                                                                                                  \
    SAVE_ARG(FUNCNAME, 2);                                                                                                                                                  \
    SAVE_ARG(FUNCNAME, 3);                                                                                                                                                  \
    SAVE_ARG(FUNCNAME, 4);                                                                                                                                                  \
    SAVE_ARG(FUNCNAME, 5);                                                                                                                                                  \
    SAVE_ARG(FUNCNAME, 6);                                                                                                                                                  \
    SAVE_ARG(FUNCNAME, 7);                                                                                                                                                  \
    if (ROOM_FOR_MORE_HISTORY(FUNCNAME))                                                                                                                                    \
    {                                                                                                                                                                       \
      SAVE_ARG_HISTORY(FUNCNAME, 0);                                                                                                                                        \
      SAVE_ARG_HISTORY(FUNCNAME, 1);                                                                                                                                        \
      SAVE_ARG_HISTORY(FUNCNAME, 2);                                                                                                                                        \
      SAVE_ARG_HISTORY(FUNCNAME, 3);                                                                                                                                        \
      SAVE_ARG_HISTORY(FUNCNAME, 4);                                                                                                                                        \
      SAVE_ARG_HISTORY(FUNCNAME, 5);                                                                                                                                        \
      SAVE_ARG_HISTORY(FUNCNAME, 6);                                                                                                                                        \
      SAVE_ARG_HISTORY(FUNCNAME, 7);                                                                                                                                        \
    }                                                                                                                                                                       \
    else                                                                                                                                                                    \
    {                                                                                                                                                                       \
      HISTORY_DROPPED(FUNCNAME);                                                                                                                                            \
    }                                                                                                                                                                       \
    INCREMENT_CALL_COUNT(FUNCNAME);                                                                                                                                         \
    REGISTER_CALL(FUNCNAME);                                                                                                                                                \
    if (FUNCNAME##_fake.custom_fake_seq_len)                                                                                                                                \
    {                                                                                                                                                                       \
      if (FUNCNAME##_fake.custom_fake_seq_idx < FUNCNAME##_fake.custom_fake_seq_len)                                                                                        \
      {                                                                                                                                                                     \
        FUNCNAME##_fake.custom_fake_seq[FUNCNAME##_fake.custom_fake_seq_idx++](arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7);                                             \
      }                                                                                                                                                                     \
      else                                                                                                                                                                  \
      {                                                                                                                                                                     \
        FUNCNAME##_fake.custom_fake_seq[FUNCNAME##_fake.custom_fake_seq_len - 1](arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7);                                           \
      }                                                                                                                                                                     \
    }                                                                                                                                                                       \
    if (FUNCNAME##_fake.custom_fake != NULL)                                                                                                                                \
    {                                                                                                                                                                       \
      FUNCNAME##_fake.custom_fake(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7);                                                                                          \
    }                                                                                                                                                                       \
  }                                                                                                                                                                         \
  DEFINE_RESET_FUNCTION(FUNCNAME)

#define FAKE_VOID_FUNC8(FUNCNAME, ARG0_TYPE, ARG1_TYPE, ARG2_TYPE, ARG3_TYPE, ARG4_TYPE, ARG5_TYPE, ARG6_TYPE, ARG7_TYPE)   \
  DECLARE_FAKE_VOID_FUNC8(FUNCNAME, ARG0_TYPE, ARG1_TYPE, ARG2_TYPE, ARG3_TYPE, ARG4_TYPE, ARG5_TYPE, ARG6_TYPE, ARG7_TYPE) \
  DEFINE_FAKE_VOID_FUNC8(FUNCNAME, ARG0_TYPE, ARG1_TYPE, ARG2_TYPE, ARG3_TYPE, ARG4_TYPE, ARG5_TYPE, ARG6_TYPE, ARG7_TYPE)

#define DECLARE_FAKE_VOID_FUNC9(FUNCNAME, ARG0_TYPE, ARG1_TYPE, ARG2_TYPE, ARG3_TYPE, ARG4_TYPE, ARG5_TYPE, ARG6_TYPE, ARG7_TYPE, ARG8_TYPE)                 \
  typedef struct FUNCNAME##_Fake                                                                                                                             \
  {                                                                                                                                                          \
    DECLARE_ARG(ARG0_TYPE, 0, FUNCNAME)                                                                                                                      \
    DECLARE_ARG(ARG1_TYPE, 1, FUNCNAME)                                                                                                                      \
    DECLARE_ARG(ARG2_TYPE, 2, FUNCNAME)                                                                                                                      \
    DECLARE_ARG(ARG3_TYPE, 3, FUNCNAME)                                                                                                                      \
    DECLARE_ARG(ARG4_TYPE, 4, FUNCNAME)                                                                                                                      \
    DECLARE_ARG(ARG5_TYPE, 5, FUNCNAME)                                                                                                                      \
    DECLARE_ARG(ARG6_TYPE, 6, FUNCNAME)                                                                                                                      \
    DECLARE_ARG(ARG7_TYPE, 7, FUNCNAME)                                                                                                                      \
    DECLARE_ARG(ARG8_TYPE, 8, FUNCNAME)                                                                                                                      \
    DECLARE_ALL_FUNC_COMMON                                                                                                                                  \
    DECLARE_CUSTOM_FAKE_SEQ_VARIABLES                                                                                                                        \
    CUSTOM_EFF_FUNCTION_TEMPLATE(void, custom_fake, ARG0_TYPE, ARG1_TYPE, ARG2_TYPE, ARG3_TYPE, ARG4_TYPE, ARG5_TYPE, ARG6_TYPE, ARG7_TYPE, ARG8_TYPE);      \
    CUSTOM_EFF_FUNCTION_TEMPLATE(void, *custom_fake_seq, ARG0_TYPE, ARG1_TYPE, ARG2_TYPE, ARG3_TYPE, ARG4_TYPE, ARG5_TYPE, ARG6_TYPE, ARG7_TYPE, ARG8_TYPE); \
  } FUNCNAME##_Fake;                                                                                                                                         \
  extern FUNCNAME##_Fake FUNCNAME##_fake;                                                                                                                    \
  void FUNCNAME##_reset(void);                                                                                                                               \
  void EFF_GCC_FUNCTION_ATTRIBUTES FUNCNAME(ARG0_TYPE arg0, ARG1_TYPE arg1, ARG2_TYPE arg2, ARG3_TYPE arg3, ARG4_TYPE arg4, ARG5_TYPE arg5, ARG6_TYPE arg6, ARG7_TYPE arg7, ARG8_TYPE arg8);

#define DEFINE_FAKE_VOID_FUNC9(FUNCNAME, ARG0_TYPE, ARG1_TYPE, ARG2_TYPE, ARG3_TYPE, ARG4_TYPE, ARG5_TYPE, ARG6_TYPE, ARG7_TYPE, ARG8_TYPE)                                                 \
  FUNCNAME##_Fake FUNCNAME##_fake;                                                                                                                                                          \
  void EFF_GCC_FUNCTION_ATTRIBUTES FUNCNAME(ARG0_TYPE arg0, ARG1_TYPE arg1, ARG2_TYPE arg2, ARG3_TYPE arg3, ARG4_TYPE arg4, ARG5_TYPE arg5, ARG6_TYPE arg6, ARG7_TYPE arg7, ARG8_TYPE arg8) \
  {                                                                                                                                                                                         \
    SAVE_ARG(FUNCNAME, 0);                                                                                                                                                                  \
    SAVE_ARG(FUNCNAME, 1);                                                                                                                                                                  \
    SAVE_ARG(FUNCNAME, 2);                                                                                                                                                                  \
    SAVE_ARG(FUNCNAME, 3);                                                                                                                                                                  \
    SAVE_ARG(FUNCNAME, 4);                                                                                                                                                                  \
    SAVE_ARG(FUNCNAME, 5);                                                                                                                                                                  \
    SAVE_ARG(FUNCNAME, 6);                                                                                                                                                                  \
    SAVE_ARG(FUNCNAME, 7);                                                                                                                                                                  \
    SAVE_ARG(FUNCNAME, 8);                                                                                                                                                                  \
    if (ROOM_FOR_MORE_HISTORY(FUNCNAME))                                                                                                                                                    \
    {                                                                                                                                                                                       \
      SAVE_ARG_HISTORY(FUNCNAME, 0);                                                                                                                                                        \
      SAVE_ARG_HISTORY(FUNCNAME, 1);                                                                                                                                                        \
      SAVE_ARG_HISTORY(FUNCNAME, 2);                                                                                                                                                        \
      SAVE_ARG_HISTORY(FUNCNAME, 3);                                                                                                                                                        \
      SAVE_ARG_HISTORY(FUNCNAME, 4);                                                                                                                                                        \
      SAVE_ARG_HISTORY(FUNCNAME, 5);                                                                                                                                                        \
      SAVE_ARG_HISTORY(FUNCNAME, 6);                                                                                                                                                        \
      SAVE_ARG_HISTORY(FUNCNAME, 7);                                                                                                                                                        \
      SAVE_ARG_HISTORY(FUNCNAME, 8);                                                                                                                                                        \
    }                                                                                                                                                                                       \
    else                                                                                                                                                                                    \
    {                                                                                                                                                                                       \
      HISTORY_DROPPED(FUNCNAME);                                                                                                                                                            \
    }                                                                                                                                                                                       \
    INCREMENT_CALL_COUNT(FUNCNAME);                                                                                                                                                         \
    REGISTER_CALL(FUNCNAME);                                                                                                                                                                \
    if (FUNCNAME##_fake.custom_fake_seq_len)                                                                                                                                                \
    {                                                                                                                                                                                       \
      if (FUNCNAME##_fake.custom_fake_seq_idx < FUNCNAME##_fake.custom_fake_seq_len)                                                                                                        \
      {                                                                                                                                                                                     \
        FUNCNAME##_fake.custom_fake_seq[FUNCNAME##_fake.custom_fake_seq_idx++](arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);                                                       \
      }                                                                                                                                                                                     \
      else                                                                                                                                                                                  \
      {                                                                                                                                                                                     \
        FUNCNAME##_fake.custom_fake_seq[FUNCNAME##_fake.custom_fake_seq_len - 1](arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);                                                     \
      }                                                                                                                                                                                     \
    }                                                                                                                                                                                       \
    if (FUNCNAME##_fake.custom_fake != NULL)                                                                                                                                                \
    {                                                                                                                                                                                       \
      FUNCNAME##_fake.custom_fake(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);                                                                                                    \
    }                                                                                                                                                                                       \
  }                                                                                                                                                                                         \
  DEFINE_RESET_FUNCTION(FUNCNAME)

#define FAKE_VOID_FUNC9(FUNCNAME, ARG0_TYPE, ARG1_TYPE, ARG2_TYPE, ARG3_TYPE, ARG4_TYPE, ARG5_TYPE, ARG6_TYPE, ARG7_TYPE, ARG8_TYPE)   \
  DECLARE_FAKE_VOID_FUNC9(FUNCNAME, ARG0_TYPE, ARG1_TYPE, ARG2_TYPE, ARG3_TYPE, ARG4_TYPE, ARG5_TYPE, ARG6_TYPE, ARG7_TYPE, ARG8_TYPE) \
  DEFINE_FAKE_VOID_FUNC9(FUNCNAME, ARG0_TYPE, ARG1_TYPE, ARG2_TYPE, ARG3_TYPE, ARG4_TYPE, ARG5_TYPE, ARG6_TYPE, ARG7_TYPE, ARG8_TYPE)

#define DECLARE_FAKE_VOID_FUNC10(FUNCNAME, ARG0_TYPE, ARG1_TYPE, ARG2_TYPE, ARG3_TYPE, ARG4_TYPE, ARG5_TYPE, ARG6_TYPE, ARG7_TYPE, ARG8_TYPE, ARG9_TYPE)                \
  typedef struct FUNCNAME##_Fake                                                                                                                                        \
  {                                                                                                                                                                     \
    DECLARE_ARG(ARG0_TYPE, 0, FUNCNAME)                                                                                                                                 \
    DECLARE_ARG(ARG1_TYPE, 1, FUNCNAME)                                                                                                                                 \
    DECLARE_ARG(ARG2_TYPE, 2, FUNCNAME)                                                                                                                                 \
    DECLARE_ARG(ARG3_TYPE, 3, FUNCNAME)                                                                                                                                 \
    DECLARE_ARG(ARG4_TYPE, 4, FUNCNAME)                                                                                                                                 \
    DECLARE_ARG(ARG5_TYPE, 5, FUNCNAME)                                                                                                                                 \
    DECLARE_ARG(ARG6_TYPE, 6, FUNCNAME)                                                                                                                                 \
    DECLARE_ARG(ARG7_TYPE, 7, FUNCNAME)                                                                                                                                 \
    DECLARE_ARG(ARG8_TYPE, 8, FUNCNAME)                                                                                                                                 \
    DECLARE_ARG(ARG9_TYPE, 9, FUNCNAME)                                                                                                                                 \
    DECLARE_ALL_FUNC_COMMON                                                                                                                                             \
    DECLARE_CUSTOM_FAKE_SEQ_VARIABLES                                                                                                                                   \
    CUSTOM_EFF_FUNCTION_TEMPLATE(void, custom_fake, ARG0_TYPE, ARG1_TYPE, ARG2_TYPE, ARG3_TYPE, ARG4_TYPE, ARG5_TYPE, ARG6_TYPE, ARG7_TYPE, ARG8_TYPE, ARG9_TYPE);      \
    CUSTOM_EFF_FUNCTION_TEMPLATE(void, *custom_fake_seq, ARG0_TYPE, ARG1_TYPE, ARG2_TYPE, ARG3_TYPE, ARG4_TYPE, ARG5_TYPE, ARG6_TYPE, ARG7_TYPE, ARG8_TYPE, ARG9_TYPE); \
  } FUNCNAME##_Fake;                                                                                                                                                    \
  extern FUNCNAME##_Fake FUNCNAME##_fake;                                                                                                                               \
  void FUNCNAME##_reset(void);                                                                                                                                          \
  void EFF_GCC_FUNCTION_ATTRIBUTES FUNCNAME(ARG0_TYPE arg0, ARG1_TYPE arg1, ARG2_TYPE arg2, ARG3_TYPE arg3, ARG4_TYPE arg4, ARG5_TYPE arg5, ARG6_TYPE arg6, ARG7_TYPE arg7, ARG8_TYPE arg8, ARG9_TYPE arg9);

#define DEFINE_FAKE_VOID_FUNC10(FUNCNAME, ARG0_TYPE, ARG1_TYPE, ARG2_TYPE, ARG3_TYPE, ARG4_TYPE, ARG5_TYPE, ARG6_TYPE, ARG7_TYPE, ARG8_TYPE, ARG9_TYPE)                                                     \
  FUNCNAME##_Fake FUNCNAME##_fake;                                                                                                                                                                          \
  void EFF_GCC_FUNCTION_ATTRIBUTES FUNCNAME(ARG0_TYPE arg0, ARG1_TYPE arg1, ARG2_TYPE arg2, ARG3_TYPE arg3, ARG4_TYPE arg4, ARG5_TYPE arg5, ARG6_TYPE arg6, ARG7_TYPE arg7, ARG8_TYPE arg8, ARG9_TYPE arg9) \
  {                                                                                                                                                                                                         \
    SAVE_ARG(FUNCNAME, 0);                                                                                                                                                                                  \
    SAVE_ARG(FUNCNAME, 1);                                                                                                                                                                                  \
    SAVE_ARG(FUNCNAME, 2);                                                                                                                                                                                  \
    SAVE_ARG(FUNCNAME, 3);                                                                                                                                                                                  \
    SAVE_ARG(FUNCNAME, 4);                                                                                                                                                                                  \
    SAVE_ARG(FUNCNAME, 5);                                                                                                                                                                                  \
    SAVE_ARG(FUNCNAME, 6);                                                                                                                                                                                  \
    SAVE_ARG(FUNCNAME, 7);                                                                                                                                                                                  \
    SAVE_ARG(FUNCNAME, 8);                                                                                                                                                                                  \
    SAVE_ARG(FUNCNAME, 9);                                                                                                                                                                                  \
    if (ROOM_FOR_MORE_HISTORY(FUNCNAME))                                                                                                                                                                    \
    {                                                                                                                                                                                                       \
      SAVE_ARG_HISTORY(FUNCNAME, 0);                                                                                                                                                                        \
      SAVE_ARG_HISTORY(FUNCNAME, 1);                                                                                                                                                                        \
      SAVE_ARG_HISTORY(FUNCNAME, 2);                                                                                                                                                                        \
      SAVE_ARG_HISTORY(FUNCNAME, 3);                                                                                                                                                                        \
      SAVE_ARG_HISTORY(FUNCNAME, 4);                                                                                                                                                                        \
      SAVE_ARG_HISTORY(FUNCNAME, 5);                                                                                                                                                                        \
      SAVE_ARG_HISTORY(FUNCNAME, 6);                                                                                                                                                                        \
      SAVE_ARG_HISTORY(FUNCNAME, 7);                                                                                                                                                                        \
      SAVE_ARG_HISTORY(FUNCNAME, 8);                                                                                                                                                                        \
      SAVE_ARG_HISTORY(FUNCNAME, 9);                                                                                                                                                                        \
    }                                                                                                                                                                                                       \
    else                                                                                                                                                                                                    \
    {                                                                                                                                                                                                       \
      HISTORY_DROPPED(FUNCNAME);                                                                                                                                                                            \
    }                                                                                                                                                                                                       \
    INCREMENT_CALL_COUNT(FUNCNAME);                                                                                                                                                                         \
    REGISTER_CALL(FUNCNAME);                                                                                                                                                                                \
    if (FUNCNAME##_fake.custom_fake_seq_len)                                                                                                                                                                \
    {                                                                                                                                                                                                       \
      if (FUNCNAME##_fake.custom_fake_seq_idx < FUNCNAME##_fake.custom_fake_seq_len)                                                                                                                        \
      {                                                                                                                                                                                                     \
        FUNCNAME##_fake.custom_fake_seq[FUNCNAME##_fake.custom_fake_seq_idx++](arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9);                                                                 \
      }                                                                                                                                                                                                     \
      else                                                                                                                                                                                                  \
      {                                                                                                                                                                                                     \
        FUNCNAME##_fake.custom_fake_seq[FUNCNAME##_fake.custom_fake_seq_len - 1](arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9);                                                               \
      }                                                                                                                                                                                                     \
    }                                                                                                                                                                                                       \
    if (FUNCNAME##_fake.custom_fake != NULL)                                                                                                                                                                \
    {                                                                                                                                                                                                       \
      FUNCNAME##_fake.custom_fake(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9);                                                                                                              \
    }                                                                                                                                                                                                       \
  }                                                                                                                                                                                                         \
  DEFINE_RESET_FUNCTION(FUNCNAME)

#define FAKE_VOID_FUNC10(FUNCNAME, ARG0_TYPE, ARG1_TYPE, ARG2_TYPE, ARG3_TYPE, ARG4_TYPE, ARG5_TYPE, ARG6_TYPE, ARG7_TYPE, ARG8_TYPE, ARG9_TYPE)   \
  DECLARE_FAKE_VOID_FUNC10(FUNCNAME, ARG0_TYPE, ARG1_TYPE, ARG2_TYPE, ARG3_TYPE, ARG4_TYPE, ARG5_TYPE, ARG6_TYPE, ARG7_TYPE, ARG8_TYPE, ARG9_TYPE) \
  DEFINE_FAKE_VOID_FUNC10(FUNCNAME, ARG0_TYPE, ARG1_TYPE, ARG2_TYPE, ARG3_TYPE, ARG4_TYPE, ARG5_TYPE, ARG6_TYPE, ARG7_TYPE, ARG8_TYPE, ARG9_TYPE)

#define DECLARE_FAKE_VALUE_FUNC0(RETURN_TYPE, FUNCNAME)                \
  typedef struct FUNCNAME##_Fake                                       \
  {                                                                    \
    DECLARE_ALL_FUNC_COMMON                                            \
    DECLARE_VALUE_FUNCTION_VARIABLES(RETURN_TYPE)                      \
    DECLARE_RETURN_VALUE_HISTORY(RETURN_TYPE)                          \
    DECLARE_CUSTOM_FAKE_SEQ_VARIABLES                                  \
    CUSTOM_EFF_FUNCTION_TEMPLATE(RETURN_TYPE, custom_fake, void);      \
    CUSTOM_EFF_FUNCTION_TEMPLATE(RETURN_TYPE, *custom_fake_seq, void); \
  } FUNCNAME##_Fake;                                                   \
  extern FUNCNAME##_Fake FUNCNAME##_fake;                              \
  void FUNCNAME##_reset(void);                                         \
  RETURN_TYPE EFF_GCC_FUNCTION_ATTRIBUTES FUNCNAME(void);

#define DEFINE_FAKE_VALUE_FUNC0(RETURN_TYPE, FUNCNAME)                                                \
  FUNCNAME##_Fake FUNCNAME##_fake;                                                                    \
  RETURN_TYPE EFF_GCC_FUNCTION_ATTRIBUTES FUNCNAME(void)                                              \
  {                                                                                                   \
    if (ROOM_FOR_MORE_HISTORY(FUNCNAME))                                                              \
    {                                                                                                 \
    }                                                                                                 \
    else                                                                                              \
    {                                                                                                 \
      HISTORY_DROPPED(FUNCNAME);                                                                      \
    }                                                                                                 \
    INCREMENT_CALL_COUNT(FUNCNAME);                                                                   \
    REGISTER_CALL(FUNCNAME);                                                                          \
    if (FUNCNAME##_fake.custom_fake_seq_len)                                                          \
    {                                                                                                 \
      if (FUNCNAME##_fake.custom_fake_seq_idx < FUNCNAME##_fake.custom_fake_seq_len)                  \
      {                                                                                               \
        RETURN_TYPE ret = FUNCNAME##_fake.custom_fake_seq[FUNCNAME##_fake.custom_fake_seq_idx++]();   \
        SAVE_RET_HISTORY(FUNCNAME, ret);                                                              \
        return ret;                                                                                   \
      }                                                                                               \
      else                                                                                            \
      {                                                                                               \
        RETURN_TYPE ret = FUNCNAME##_fake.custom_fake_seq[FUNCNAME##_fake.custom_fake_seq_len - 1](); \
        SAVE_RET_HISTORY(FUNCNAME, ret);                                                              \
        return ret;                                                                                   \
      }                                                                                               \
    }                                                                                                 \
    if (FUNCNAME##_fake.custom_fake != NULL)                                                          \
    {                                                                                                 \
      RETURN_TYPE ret = FUNCNAME##_fake.custom_fake();                                                \
      SAVE_RET_HISTORY(FUNCNAME, ret);                                                                \
      return ret;                                                                                     \
    }                                                                                                 \
    RETURN_FAKE_RESULT(FUNCNAME)                                                                      \
  }                                                                                                   \
  DEFINE_RESET_FUNCTION(FUNCNAME)

#define FAKE_VALUE_FUNC0(RETURN_TYPE, FUNCNAME)   \
  DECLARE_FAKE_VALUE_FUNC0(RETURN_TYPE, FUNCNAME) \
  DEFINE_FAKE_VALUE_FUNC0(RETURN_TYPE, FUNCNAME)

#define DECLARE_FAKE_VALUE_FUNC1(RETURN_TYPE, FUNCNAME, ARG0_TYPE)          \
  typedef struct FUNCNAME##_Fake                                            \
  {                                                                         \
    DECLARE_ARG(ARG0_TYPE, 0, FUNCNAME)                                     \
    DECLARE_ALL_FUNC_COMMON                                                 \
    DECLARE_VALUE_FUNCTION_VARIABLES(RETURN_TYPE)                           \
    DECLARE_RETURN_VALUE_HISTORY(RETURN_TYPE)                               \
    DECLARE_CUSTOM_FAKE_SEQ_VARIABLES                                       \
    CUSTOM_EFF_FUNCTION_TEMPLATE(RETURN_TYPE, custom_fake, ARG0_TYPE);      \
    CUSTOM_EFF_FUNCTION_TEMPLATE(RETURN_TYPE, *custom_fake_seq, ARG0_TYPE); \
  } FUNCNAME##_Fake;                                                        \
  extern FUNCNAME##_Fake FUNCNAME##_fake;                                   \
  void FUNCNAME##_reset(void);                                              \
  RETURN_TYPE EFF_GCC_FUNCTION_ATTRIBUTES FUNCNAME(ARG0_TYPE arg0);

#define DEFINE_FAKE_VALUE_FUNC1(RETURN_TYPE, FUNCNAME, ARG0_TYPE)                                         \
  FUNCNAME##_Fake FUNCNAME##_fake;                                                                        \
  RETURN_TYPE EFF_GCC_FUNCTION_ATTRIBUTES FUNCNAME(ARG0_TYPE arg0)                                        \
  {                                                                                                       \
    SAVE_ARG(FUNCNAME, 0);                                                                                \
    if (ROOM_FOR_MORE_HISTORY(FUNCNAME))                                                                  \
    {                                                                                                     \
      SAVE_ARG_HISTORY(FUNCNAME, 0);                                                                      \
    }                                                                                                     \
    else                                                                                                  \
    {                                                                                                     \
      HISTORY_DROPPED(FUNCNAME);                                                                          \
    }                                                                                                     \
    INCREMENT_CALL_COUNT(FUNCNAME);                                                                       \
    REGISTER_CALL(FUNCNAME);                                                                              \
    if (FUNCNAME##_fake.custom_fake_seq_len)                                                              \
    {                                                                                                     \
      if (FUNCNAME##_fake.custom_fake_seq_idx < FUNCNAME##_fake.custom_fake_seq_len)                      \
      {                                                                                                   \
        RETURN_TYPE ret = FUNCNAME##_fake.custom_fake_seq[FUNCNAME##_fake.custom_fake_seq_idx++](arg0);   \
        SAVE_RET_HISTORY(FUNCNAME, ret);                                                                  \
        return ret;                                                                                       \
      }                                                                                                   \
      else                                                                                                \
      {                                                                                                   \
        RETURN_TYPE ret = FUNCNAME##_fake.custom_fake_seq[FUNCNAME##_fake.custom_fake_seq_len - 1](arg0); \
        SAVE_RET_HISTORY(FUNCNAME, ret);                                                                  \
        return ret;                                                                                       \
      }                                                                                                   \
    }                                                                                                     \
    if (FUNCNAME##_fake.custom_fake != NULL)                                                              \
    {                                                                                                     \
      RETURN_TYPE ret = FUNCNAME##_fake.custom_fake(arg0);                                                \
      SAVE_RET_HISTORY(FUNCNAME, ret);                                                                    \
      return ret;                                                                                         \
    }                                                                                                     \
    RETURN_FAKE_RESULT(FUNCNAME)                                                                          \
  }                                                                                                       \
  DEFINE_RESET_FUNCTION(FUNCNAME)

#define FAKE_VALUE_FUNC1(RETURN_TYPE, FUNCNAME, ARG0_TYPE)   \
  DECLARE_FAKE_VALUE_FUNC1(RETURN_TYPE, FUNCNAME, ARG0_TYPE) \
  DEFINE_FAKE_VALUE_FUNC1(RETURN_TYPE, FUNCNAME, ARG0_TYPE)

#define DECLARE_FAKE_VALUE_FUNC2(RETURN_TYPE, FUNCNAME, ARG0_TYPE, ARG1_TYPE)          \
  typedef struct FUNCNAME##_Fake                                                       \
  {                                                                                    \
    DECLARE_ARG(ARG0_TYPE, 0, FUNCNAME)                                                \
    DECLARE_ARG(ARG1_TYPE, 1, FUNCNAME)                                                \
    DECLARE_ALL_FUNC_COMMON                                                            \
    DECLARE_VALUE_FUNCTION_VARIABLES(RETURN_TYPE)                                      \
    DECLARE_RETURN_VALUE_HISTORY(RETURN_TYPE)                                          \
    DECLARE_CUSTOM_FAKE_SEQ_VARIABLES                                                  \
    CUSTOM_EFF_FUNCTION_TEMPLATE(RETURN_TYPE, custom_fake, ARG0_TYPE, ARG1_TYPE);      \
    CUSTOM_EFF_FUNCTION_TEMPLATE(RETURN_TYPE, *custom_fake_seq, ARG0_TYPE, ARG1_TYPE); \
  } FUNCNAME##_Fake;                                                                   \
  extern FUNCNAME##_Fake FUNCNAME##_fake;                                              \
  void FUNCNAME##_reset(void);                                                         \
  RETURN_TYPE EFF_GCC_FUNCTION_ATTRIBUTES FUNCNAME(ARG0_TYPE arg0, ARG1_TYPE arg1);

#define DEFINE_FAKE_VALUE_FUNC2(RETURN_TYPE, FUNCNAME, ARG0_TYPE, ARG1_TYPE)                                    \
  FUNCNAME##_Fake FUNCNAME##_fake;                                                                              \
  RETURN_TYPE EFF_GCC_FUNCTION_ATTRIBUTES FUNCNAME(ARG0_TYPE arg0, ARG1_TYPE arg1)                              \
  {                                                                                                             \
    SAVE_ARG(FUNCNAME, 0);                                                                                      \
    SAVE_ARG(FUNCNAME, 1);                                                                                      \
    if (ROOM_FOR_MORE_HISTORY(FUNCNAME))                                                                        \
    {                                                                                                           \
      SAVE_ARG_HISTORY(FUNCNAME, 0);                                                                            \
      SAVE_ARG_HISTORY(FUNCNAME, 1);                                                                            \
    }                                                                                                           \
    else                                                                                                        \
    {                                                                                                           \
      HISTORY_DROPPED(FUNCNAME);                                                                                \
    }                                                                                                           \
    INCREMENT_CALL_COUNT(FUNCNAME);                                                                             \
    REGISTER_CALL(FUNCNAME);                                                                                    \
    if (FUNCNAME##_fake.custom_fake_seq_len)                                                                    \
    {                                                                                                           \
      if (FUNCNAME##_fake.custom_fake_seq_idx < FUNCNAME##_fake.custom_fake_seq_len)                            \
      {                                                                                                         \
        RETURN_TYPE ret = FUNCNAME##_fake.custom_fake_seq[FUNCNAME##_fake.custom_fake_seq_idx++](arg0, arg1);   \
        SAVE_RET_HISTORY(FUNCNAME, ret);                                                                        \
        return ret;                                                                                             \
      }                                                                                                         \
      else                                                                                                      \
      {                                                                                                         \
        RETURN_TYPE ret = FUNCNAME##_fake.custom_fake_seq[FUNCNAME##_fake.custom_fake_seq_len - 1](arg0, arg1); \
        SAVE_RET_HISTORY(FUNCNAME, ret);                                                                        \
        return ret;                                                                                             \
      }                                                                                                         \
    }                                                                                                           \
    if (FUNCNAME##_fake.custom_fake != NULL)                                                                    \
    {                                                                                                           \
      RETURN_TYPE ret = FUNCNAME##_fake.custom_fake(arg0, arg1);                                                \
      SAVE_RET_HISTORY(FUNCNAME, ret);                                                                          \
      return ret;                                                                                               \
    }                                                                                                           \
    RETURN_FAKE_RESULT(FUNCNAME)                                                                                \
  }                                                                                                             \
  DEFINE_RESET_FUNCTION(FUNCNAME)

#define FAKE_VALUE_FUNC2(RETURN_TYPE, FUNCNAME, ARG0_TYPE, ARG1_TYPE)   \
  DECLARE_FAKE_VALUE_FUNC2(RETURN_TYPE, FUNCNAME, ARG0_TYPE, ARG1_TYPE) \
  DEFINE_FAKE_VALUE_FUNC2(RETURN_TYPE, FUNCNAME, ARG0_TYPE, ARG1_TYPE)

#define DECLARE_FAKE_VALUE_FUNC3(RETURN_TYPE, FUNCNAME, ARG0_TYPE, ARG1_TYPE, ARG2_TYPE)          \
  typedef struct FUNCNAME##_Fake                                                                  \
  {                                                                                               \
    DECLARE_ARG(ARG0_TYPE, 0, FUNCNAME)                                                           \
    DECLARE_ARG(ARG1_TYPE, 1, FUNCNAME)                                                           \
    DECLARE_ARG(ARG2_TYPE, 2, FUNCNAME)                                                           \
    DECLARE_ALL_FUNC_COMMON                                                                       \
    DECLARE_VALUE_FUNCTION_VARIABLES(RETURN_TYPE)                                                 \
    DECLARE_RETURN_VALUE_HISTORY(RETURN_TYPE)                                                     \
    DECLARE_CUSTOM_FAKE_SEQ_VARIABLES                                                             \
    CUSTOM_EFF_FUNCTION_TEMPLATE(RETURN_TYPE, custom_fake, ARG0_TYPE, ARG1_TYPE, ARG2_TYPE);      \
    CUSTOM_EFF_FUNCTION_TEMPLATE(RETURN_TYPE, *custom_fake_seq, ARG0_TYPE, ARG1_TYPE, ARG2_TYPE); \
  } FUNCNAME##_Fake;                                                                              \
  extern FUNCNAME##_Fake FUNCNAME##_fake;                                                         \
  void FUNCNAME##_reset(void);                                                                    \
  RETURN_TYPE EFF_GCC_FUNCTION_ATTRIBUTES FUNCNAME(ARG0_TYPE arg0, ARG1_TYPE arg1, ARG2_TYPE arg2);

#define DEFINE_FAKE_VALUE_FUNC3(RETURN_TYPE, FUNCNAME, ARG0_TYPE, ARG1_TYPE, ARG2_TYPE)                               \
  FUNCNAME##_Fake FUNCNAME##_fake;                                                                                    \
  RETURN_TYPE EFF_GCC_FUNCTION_ATTRIBUTES FUNCNAME(ARG0_TYPE arg0, ARG1_TYPE arg1, ARG2_TYPE arg2)                    \
  {                                                                                                                   \
    SAVE_ARG(FUNCNAME, 0);                                                                                            \
    SAVE_ARG(FUNCNAME, 1);                                                                                            \
    SAVE_ARG(FUNCNAME, 2);                                                                                            \
    if (ROOM_FOR_MORE_HISTORY(FUNCNAME))                                                                              \
    {                                                                                                                 \
      SAVE_ARG_HISTORY(FUNCNAME, 0);                                                                                  \
      SAVE_ARG_HISTORY(FUNCNAME, 1);                                                                                  \
      SAVE_ARG_HISTORY(FUNCNAME, 2);                                                                                  \
    }                                                                                                                 \
    else                                                                                                              \
    {                                                                                                                 \
      HISTORY_DROPPED(FUNCNAME);                                                                                      \
    }                                                                                                                 \
    INCREMENT_CALL_COUNT(FUNCNAME);                                                                                   \
    REGISTER_CALL(FUNCNAME);                                                                                          \
    if (FUNCNAME##_fake.custom_fake_seq_len)                                                                          \
    {                                                                                                                 \
      if (FUNCNAME##_fake.custom_fake_seq_idx < FUNCNAME##_fake.custom_fake_seq_len)                                  \
      {                                                                                                               \
        RETURN_TYPE ret = FUNCNAME##_fake.custom_fake_seq[FUNCNAME##_fake.custom_fake_seq_idx++](arg0, arg1, arg2);   \
        SAVE_RET_HISTORY(FUNCNAME, ret);                                                                              \
        return ret;                                                                                                   \
      }                                                                                                               \
      else                                                                                                            \
      {                                                                                                               \
        RETURN_TYPE ret = FUNCNAME##_fake.custom_fake_seq[FUNCNAME##_fake.custom_fake_seq_len - 1](arg0, arg1, arg2); \
        SAVE_RET_HISTORY(FUNCNAME, ret);                                                                              \
        return ret;                                                                                                   \
      }                                                                                                               \
    }                                                                                                                 \
    if (FUNCNAME##_fake.custom_fake != NULL)                                                                          \
    {                                                                                                                 \
      RETURN_TYPE ret = FUNCNAME##_fake.custom_fake(arg0, arg1, arg2);                                                \
      SAVE_RET_HISTORY(FUNCNAME, ret);                                                                                \
      return ret;                                                                                                     \
    }                                                                                                                 \
    RETURN_FAKE_RESULT(FUNCNAME)                                                                                      \
  }                                                                                                                   \
  DEFINE_RESET_FUNCTION(FUNCNAME)

#define FAKE_VALUE_FUNC3(RETURN_TYPE, FUNCNAME, ARG0_TYPE, ARG1_TYPE, ARG2_TYPE)   \
  DECLARE_FAKE_VALUE_FUNC3(RETURN_TYPE, FUNCNAME, ARG0_TYPE, ARG1_TYPE, ARG2_TYPE) \
  DEFINE_FAKE_VALUE_FUNC3(RETURN_TYPE, FUNCNAME, ARG0_TYPE, ARG1_TYPE, ARG2_TYPE)

#define DECLARE_FAKE_VALUE_FUNC4(RETURN_TYPE, FUNCNAME, ARG0_TYPE, ARG1_TYPE, ARG2_TYPE, ARG3_TYPE)          \
  typedef struct FUNCNAME##_Fake                                                                             \
  {                                                                                                          \
    DECLARE_ARG(ARG0_TYPE, 0, FUNCNAME)                                                                      \
    DECLARE_ARG(ARG1_TYPE, 1, FUNCNAME)                                                                      \
    DECLARE_ARG(ARG2_TYPE, 2, FUNCNAME)                                                                      \
    DECLARE_ARG(ARG3_TYPE, 3, FUNCNAME)                                                                      \
    DECLARE_ALL_FUNC_COMMON                                                                                  \
    DECLARE_VALUE_FUNCTION_VARIABLES(RETURN_TYPE)                                                            \
    DECLARE_RETURN_VALUE_HISTORY(RETURN_TYPE)                                                                \
    DECLARE_CUSTOM_FAKE_SEQ_VARIABLES                                                                        \
    CUSTOM_EFF_FUNCTION_TEMPLATE(RETURN_TYPE, custom_fake, ARG0_TYPE, ARG1_TYPE, ARG2_TYPE, ARG3_TYPE);      \
    CUSTOM_EFF_FUNCTION_TEMPLATE(RETURN_TYPE, *custom_fake_seq, ARG0_TYPE, ARG1_TYPE, ARG2_TYPE, ARG3_TYPE); \
  } FUNCNAME##_Fake;                                                                                         \
  extern FUNCNAME##_Fake FUNCNAME##_fake;                                                                    \
  void FUNCNAME##_reset(void);                                                                               \
  RETURN_TYPE EFF_GCC_FUNCTION_ATTRIBUTES FUNCNAME(ARG0_TYPE arg0, ARG1_TYPE arg1, ARG2_TYPE arg2, ARG3_TYPE arg3);

#define DEFINE_FAKE_VALUE_FUNC4(RETURN_TYPE, FUNCNAME, ARG0_TYPE, ARG1_TYPE, ARG2_TYPE, ARG3_TYPE)                          \
  FUNCNAME##_Fake FUNCNAME##_fake;                                                                                          \
  RETURN_TYPE EFF_GCC_FUNCTION_ATTRIBUTES FUNCNAME(ARG0_TYPE arg0, ARG1_TYPE arg1, ARG2_TYPE arg2, ARG3_TYPE arg3)          \
  {                                                                                                                         \
    SAVE_ARG(FUNCNAME, 0);                                                                                                  \
    SAVE_ARG(FUNCNAME, 1);                                                                                                  \
    SAVE_ARG(FUNCNAME, 2);                                                                                                  \
    SAVE_ARG(FUNCNAME, 3);                                                                                                  \
    if (ROOM_FOR_MORE_HISTORY(FUNCNAME))                                                                                    \
    {                                                                                                                       \
      SAVE_ARG_HISTORY(FUNCNAME, 0);                                                                                        \
      SAVE_ARG_HISTORY(FUNCNAME, 1);                                                                                        \
      SAVE_ARG_HISTORY(FUNCNAME, 2);                                                                                        \
      SAVE_ARG_HISTORY(FUNCNAME, 3);                                                                                        \
    }                                                                                                                       \
    else                                                                                                                    \
    {                                                                                                                       \
      HISTORY_DROPPED(FUNCNAME);                                                                                            \
    }                                                                                                                       \
    INCREMENT_CALL_COUNT(FUNCNAME);                                                                                         \
    REGISTER_CALL(FUNCNAME);                                                                                                \
    if (FUNCNAME##_fake.custom_fake_seq_len)                                                                                \
    {                                                                                                                       \
      if (FUNCNAME##_fake.custom_fake_seq_idx < FUNCNAME##_fake.custom_fake_seq_len)                                        \
      {                                                                                                                     \
        RETURN_TYPE ret = FUNCNAME##_fake.custom_fake_seq[FUNCNAME##_fake.custom_fake_seq_idx++](arg0, arg1, arg2, arg3);   \
        SAVE_RET_HISTORY(FUNCNAME, ret);                                                                                    \
        return ret;                                                                                                         \
      }                                                                                                                     \
      else                                                                                                                  \
      {                                                                                                                     \
        RETURN_TYPE ret = FUNCNAME##_fake.custom_fake_seq[FUNCNAME##_fake.custom_fake_seq_len - 1](arg0, arg1, arg2, arg3); \
        SAVE_RET_HISTORY(FUNCNAME, ret);                                                                                    \
        return ret;                                                                                                         \
      }                                                                                                                     \
    }                                                                                                                       \
    if (FUNCNAME##_fake.custom_fake != NULL)                                                                                \
    {                                                                                                                       \
      RETURN_TYPE ret = FUNCNAME##_fake.custom_fake(arg0, arg1, arg2, arg3);                                                \
      SAVE_RET_HISTORY(FUNCNAME, ret);                                                                                      \
      return ret;                                                                                                           \
    }                                                                                                                       \
    RETURN_FAKE_RESULT(FUNCNAME)                                                                                            \
  }                                                                                                                         \
  DEFINE_RESET_FUNCTION(FUNCNAME)

#define FAKE_VALUE_FUNC4(RETURN_TYPE, FUNCNAME, ARG0_TYPE, ARG1_TYPE, ARG2_TYPE, ARG3_TYPE)   \
  DECLARE_FAKE_VALUE_FUNC4(RETURN_TYPE, FUNCNAME, ARG0_TYPE, ARG1_TYPE, ARG2_TYPE, ARG3_TYPE) \
  DEFINE_FAKE_VALUE_FUNC4(RETURN_TYPE, FUNCNAME, ARG0_TYPE, ARG1_TYPE, ARG2_TYPE, ARG3_TYPE)

#define DECLARE_FAKE_VALUE_FUNC5(RETURN_TYPE, FUNCNAME, ARG0_TYPE, ARG1_TYPE, ARG2_TYPE, ARG3_TYPE, ARG4_TYPE)          \
  typedef struct FUNCNAME##_Fake                                                                                        \
  {                                                                                                                     \
    DECLARE_ARG(ARG0_TYPE, 0, FUNCNAME)                                                                                 \
    DECLARE_ARG(ARG1_TYPE, 1, FUNCNAME)                                                                                 \
    DECLARE_ARG(ARG2_TYPE, 2, FUNCNAME)                                                                                 \
    DECLARE_ARG(ARG3_TYPE, 3, FUNCNAME)                                                                                 \
    DECLARE_ARG(ARG4_TYPE, 4, FUNCNAME)                                                                                 \
    DECLARE_ALL_FUNC_COMMON                                                                                             \
    DECLARE_VALUE_FUNCTION_VARIABLES(RETURN_TYPE)                                                                       \
    DECLARE_RETURN_VALUE_HISTORY(RETURN_TYPE)                                                                           \
    DECLARE_CUSTOM_FAKE_SEQ_VARIABLES                                                                                   \
    CUSTOM_EFF_FUNCTION_TEMPLATE(RETURN_TYPE, custom_fake, ARG0_TYPE, ARG1_TYPE, ARG2_TYPE, ARG3_TYPE, ARG4_TYPE);      \
    CUSTOM_EFF_FUNCTION_TEMPLATE(RETURN_TYPE, *custom_fake_seq, ARG0_TYPE, ARG1_TYPE, ARG2_TYPE, ARG3_TYPE, ARG4_TYPE); \
  } FUNCNAME##_Fake;                                                                                                    \
  extern FUNCNAME##_Fake FUNCNAME##_fake;                                                                               \
  void FUNCNAME##_reset(void);                                                                                          \
  RETURN_TYPE EFF_GCC_FUNCTION_ATTRIBUTES FUNCNAME(ARG0_TYPE arg0, ARG1_TYPE arg1, ARG2_TYPE arg2, ARG3_TYPE arg3, ARG4_TYPE arg4);

#define DEFINE_FAKE_VALUE_FUNC5(RETURN_TYPE, FUNCNAME, ARG0_TYPE, ARG1_TYPE, ARG2_TYPE, ARG3_TYPE, ARG4_TYPE)                      \
  FUNCNAME##_Fake FUNCNAME##_fake;                                                                                                 \
  RETURN_TYPE EFF_GCC_FUNCTION_ATTRIBUTES FUNCNAME(ARG0_TYPE arg0, ARG1_TYPE arg1, ARG2_TYPE arg2, ARG3_TYPE arg3, ARG4_TYPE arg4) \
  {                                                                                                                                \
    SAVE_ARG(FUNCNAME, 0);                                                                                                         \
    SAVE_ARG(FUNCNAME, 1);                                                                                                         \
    SAVE_ARG(FUNCNAME, 2);                                                                                                         \
    SAVE_ARG(FUNCNAME, 3);                                                                                                         \
    SAVE_ARG(FUNCNAME, 4);                                                                                                         \
    if (ROOM_FOR_MORE_HISTORY(FUNCNAME))                                                                                           \
    {                                                                                                                              \
      SAVE_ARG_HISTORY(FUNCNAME, 0);                                                                                               \
      SAVE_ARG_HISTORY(FUNCNAME, 1);                                                                                               \
      SAVE_ARG_HISTORY(FUNCNAME, 2);                                                                                               \
      SAVE_ARG_HISTORY(FUNCNAME, 3);                                                                                               \
      SAVE_ARG_HISTORY(FUNCNAME, 4);                                                                                               \
    }                                                                                                                              \
    else                                                                                                                           \
    {                                                                                                                              \
      HISTORY_DROPPED(FUNCNAME);                                                                                                   \
    }                                                                                                                              \
    INCREMENT_CALL_COUNT(FUNCNAME);                                                                                                \
    REGISTER_CALL(FUNCNAME);                                                                                                       \
    if (FUNCNAME##_fake.custom_fake_seq_len)                                                                                       \
    {                                                                                                                              \
      if (FUNCNAME##_fake.custom_fake_seq_idx < FUNCNAME##_fake.custom_fake_seq_len)                                               \
      {                                                                                                                            \
        RETURN_TYPE ret = FUNCNAME##_fake.custom_fake_seq[FUNCNAME##_fake.custom_fake_seq_idx++](arg0, arg1, arg2, arg3, arg4);    \
        SAVE_RET_HISTORY(FUNCNAME, ret);                                                                                           \
        return ret;                                                                                                                \
      }                                                                                                                            \
      else                                                                                                                         \
      {                                                                                                                            \
        RETURN_TYPE ret = FUNCNAME##_fake.custom_fake_seq[FUNCNAME##_fake.custom_fake_seq_len - 1](arg0, arg1, arg2, arg3, arg4);  \
        SAVE_RET_HISTORY(FUNCNAME, ret);                                                                                           \
        return ret;                                                                                                                \
      }                                                                                                                            \
    }                                                                                                                              \
    if (FUNCNAME##_fake.custom_fake != NULL)                                                                                       \
    {                                                                                                                              \
      RETURN_TYPE ret = FUNCNAME##_fake.custom_fake(arg0, arg1, arg2, arg3, arg4);                                                 \
      SAVE_RET_HISTORY(FUNCNAME, ret);                                                                                             \
      return ret;                                                                                                                  \
    }                                                                                                                              \
    RETURN_FAKE_RESULT(FUNCNAME)                                                                                                   \
  }                                                                                                                                \
  DEFINE_RESET_FUNCTION(FUNCNAME)

#define FAKE_VALUE_FUNC5(RETURN_TYPE, FUNCNAME, ARG0_TYPE, ARG1_TYPE, ARG2_TYPE, ARG3_TYPE, ARG4_TYPE)   \
  DECLARE_FAKE_VALUE_FUNC5(RETURN_TYPE, FUNCNAME, ARG0_TYPE, ARG1_TYPE, ARG2_TYPE, ARG3_TYPE, ARG4_TYPE) \
  DEFINE_FAKE_VALUE_FUNC5(RETURN_TYPE, FUNCNAME, ARG0_TYPE, ARG1_TYPE, ARG2_TYPE, ARG3_TYPE, ARG4_TYPE)

#define DECLARE_FAKE_VALUE_FUNC6(RETURN_TYPE, FUNCNAME, ARG0_TYPE, ARG1_TYPE, ARG2_TYPE, ARG3_TYPE, ARG4_TYPE, ARG5_TYPE)          \
  typedef struct FUNCNAME##_Fake                                                                                                   \
  {                                                                                                                                \
    DECLARE_ARG(ARG0_TYPE, 0, FUNCNAME)                                                                                            \
    DECLARE_ARG(ARG1_TYPE, 1, FUNCNAME)                                                                                            \
    DECLARE_ARG(ARG2_TYPE, 2, FUNCNAME)                                                                                            \
    DECLARE_ARG(ARG3_TYPE, 3, FUNCNAME)                                                                                            \
    DECLARE_ARG(ARG4_TYPE, 4, FUNCNAME)                                                                                            \
    DECLARE_ARG(ARG5_TYPE, 5, FUNCNAME)                                                                                            \
    DECLARE_ALL_FUNC_COMMON                                                                                                        \
    DECLARE_VALUE_FUNCTION_VARIABLES(RETURN_TYPE)                                                                                  \
    DECLARE_RETURN_VALUE_HISTORY(RETURN_TYPE)                                                                                      \
    DECLARE_CUSTOM_FAKE_SEQ_VARIABLES                                                                                              \
    CUSTOM_EFF_FUNCTION_TEMPLATE(RETURN_TYPE, custom_fake, ARG0_TYPE, ARG1_TYPE, ARG2_TYPE, ARG3_TYPE, ARG4_TYPE, ARG5_TYPE);      \
    CUSTOM_EFF_FUNCTION_TEMPLATE(RETURN_TYPE, *custom_fake_seq, ARG0_TYPE, ARG1_TYPE, ARG2_TYPE, ARG3_TYPE, ARG4_TYPE, ARG5_TYPE); \
  } FUNCNAME##_Fake;                                                                                                               \
  extern FUNCNAME##_Fake FUNCNAME##_fake;                                                                                          \
  void FUNCNAME##_reset(void);                                                                                                     \
  RETURN_TYPE EFF_GCC_FUNCTION_ATTRIBUTES FUNCNAME(ARG0_TYPE arg0, ARG1_TYPE arg1, ARG2_TYPE arg2, ARG3_TYPE arg3, ARG4_TYPE arg4, ARG5_TYPE arg5);

#define DEFINE_FAKE_VALUE_FUNC6(RETURN_TYPE, FUNCNAME, ARG0_TYPE, ARG1_TYPE, ARG2_TYPE, ARG3_TYPE, ARG4_TYPE, ARG5_TYPE)                           \
  FUNCNAME##_Fake FUNCNAME##_fake;                                                                                                                 \
  RETURN_TYPE EFF_GCC_FUNCTION_ATTRIBUTES FUNCNAME(ARG0_TYPE arg0, ARG1_TYPE arg1, ARG2_TYPE arg2, ARG3_TYPE arg3, ARG4_TYPE arg4, ARG5_TYPE arg5) \
  {                                                                                                                                                \
    SAVE_ARG(FUNCNAME, 0);                                                                                                                         \
    SAVE_ARG(FUNCNAME, 1);                                                                                                                         \
    SAVE_ARG(FUNCNAME, 2);                                                                                                                         \
    SAVE_ARG(FUNCNAME, 3);                                                                                                                         \
    SAVE_ARG(FUNCNAME, 4);                                                                                                                         \
    SAVE_ARG(FUNCNAME, 5);                                                                                                                         \
    if (ROOM_FOR_MORE_HISTORY(FUNCNAME))                                                                                                           \
    {                                                                                                                                              \
      SAVE_ARG_HISTORY(FUNCNAME, 0);                                                                                                               \
      SAVE_ARG_HISTORY(FUNCNAME, 1);                                                                                                               \
      SAVE_ARG_HISTORY(FUNCNAME, 2);                                                                                                               \
      SAVE_ARG_HISTORY(FUNCNAME, 3);                                                                                                               \
      SAVE_ARG_HISTORY(FUNCNAME, 4);                                                                                                               \
      SAVE_ARG_HISTORY(FUNCNAME, 5);                                                                                                               \
    }                                                                                                                                              \
    else                                                                                                                                           \
    {                                                                                                                                              \
      HISTORY_DROPPED(FUNCNAME);                                                                                                                   \
    }                                                                                                                                              \
    INCREMENT_CALL_COUNT(FUNCNAME);                                                                                                                \
    REGISTER_CALL(FUNCNAME);                                                                                                                       \
    if (FUNCNAME##_fake.custom_fake_seq_len)                                                                                                       \
    {                                                                                                                                              \
      if (FUNCNAME##_fake.custom_fake_seq_idx < FUNCNAME##_fake.custom_fake_seq_len)                                                               \
      {                                                                                                                                            \
        RETURN_TYPE ret = FUNCNAME##_fake.custom_fake_seq[FUNCNAME##_fake.custom_fake_seq_idx++](arg0, arg1, arg2, arg3, arg4, arg5);              \
        SAVE_RET_HISTORY(FUNCNAME, ret);                                                                                                           \
        return ret;                                                                                                                                \
      }                                                                                                                                            \
      else                                                                                                                                         \
      {                                                                                                                                            \
        RETURN_TYPE ret = FUNCNAME##_fake.custom_fake_seq[FUNCNAME##_fake.custom_fake_seq_len - 1](arg0, arg1, arg2, arg3, arg4, arg5);            \
        SAVE_RET_HISTORY(FUNCNAME, ret);                                                                                                           \
        return ret;                                                                                                                                \
      }                                                                                                                                            \
    }                                                                                                                                              \
    if (FUNCNAME##_fake.custom_fake != NULL)                                                                                                       \
    {                                                                                                                                              \
      RETURN_TYPE ret = FUNCNAME##_fake.custom_fake(arg0, arg1, arg2, arg3, arg4, arg5);                                                           \
      SAVE_RET_HISTORY(FUNCNAME, ret);                                                                                                             \
      return ret;                                                                                                                                  \
    }                                                                                                                                              \
    RETURN_FAKE_RESULT(FUNCNAME)                                                                                                                   \
  }                                                                                                                                                \
  DEFINE_RESET_FUNCTION(FUNCNAME)

#define FAKE_VALUE_FUNC6(RETURN_TYPE, FUNCNAME, ARG0_TYPE, ARG1_TYPE, ARG2_TYPE, ARG3_TYPE, ARG4_TYPE, ARG5_TYPE)   \
  DECLARE_FAKE_VALUE_FUNC6(RETURN_TYPE, FUNCNAME, ARG0_TYPE, ARG1_TYPE, ARG2_TYPE, ARG3_TYPE, ARG4_TYPE, ARG5_TYPE) \
  DEFINE_FAKE_VALUE_FUNC6(RETURN_TYPE, FUNCNAME, ARG0_TYPE, ARG1_TYPE, ARG2_TYPE, ARG3_TYPE, ARG4_TYPE, ARG5_TYPE)

#define DECLARE_FAKE_VALUE_FUNC7(RETURN_TYPE, FUNCNAME, ARG0_TYPE, ARG1_TYPE, ARG2_TYPE, ARG3_TYPE, ARG4_TYPE, ARG5_TYPE, ARG6_TYPE)          \
  typedef struct FUNCNAME##_Fake                                                                                                              \
  {                                                                                                                                           \
    DECLARE_ARG(ARG0_TYPE, 0, FUNCNAME)                                                                                                       \
    DECLARE_ARG(ARG1_TYPE, 1, FUNCNAME)                                                                                                       \
    DECLARE_ARG(ARG2_TYPE, 2, FUNCNAME)                                                                                                       \
    DECLARE_ARG(ARG3_TYPE, 3, FUNCNAME)                                                                                                       \
    DECLARE_ARG(ARG4_TYPE, 4, FUNCNAME)                                                                                                       \
    DECLARE_ARG(ARG5_TYPE, 5, FUNCNAME)                                                                                                       \
    DECLARE_ARG(ARG6_TYPE, 6, FUNCNAME)                                                                                                       \
    DECLARE_ALL_FUNC_COMMON                                                                                                                   \
    DECLARE_VALUE_FUNCTION_VARIABLES(RETURN_TYPE)                                                                                             \
    DECLARE_RETURN_VALUE_HISTORY(RETURN_TYPE)                                                                                                 \
    DECLARE_CUSTOM_FAKE_SEQ_VARIABLES                                                                                                         \
    CUSTOM_EFF_FUNCTION_TEMPLATE(RETURN_TYPE, custom_fake, ARG0_TYPE, ARG1_TYPE, ARG2_TYPE, ARG3_TYPE, ARG4_TYPE, ARG5_TYPE, ARG6_TYPE);      \
    CUSTOM_EFF_FUNCTION_TEMPLATE(RETURN_TYPE, *custom_fake_seq, ARG0_TYPE, ARG1_TYPE, ARG2_TYPE, ARG3_TYPE, ARG4_TYPE, ARG5_TYPE, ARG6_TYPE); \
  } FUNCNAME##_Fake;                                                                                                                          \
  extern FUNCNAME##_Fake FUNCNAME##_fake;                                                                                                     \
  void FUNCNAME##_reset(void);                                                                                                                \
  RETURN_TYPE EFF_GCC_FUNCTION_ATTRIBUTES FUNCNAME(ARG0_TYPE arg0, ARG1_TYPE arg1, ARG2_TYPE arg2, ARG3_TYPE arg3, ARG4_TYPE arg4, ARG5_TYPE arg5, ARG6_TYPE arg6);

#define DEFINE_FAKE_VALUE_FUNC7(RETURN_TYPE, FUNCNAME, ARG0_TYPE, ARG1_TYPE, ARG2_TYPE, ARG3_TYPE, ARG4_TYPE, ARG5_TYPE, ARG6_TYPE)                                \
  FUNCNAME##_Fake FUNCNAME##_fake;                                                                                                                                 \
  RETURN_TYPE EFF_GCC_FUNCTION_ATTRIBUTES FUNCNAME(ARG0_TYPE arg0, ARG1_TYPE arg1, ARG2_TYPE arg2, ARG3_TYPE arg3, ARG4_TYPE arg4, ARG5_TYPE arg5, ARG6_TYPE arg6) \
  {                                                                                                                                                                \
    SAVE_ARG(FUNCNAME, 0);                                                                                                                                         \
    SAVE_ARG(FUNCNAME, 1);                                                                                                                                         \
    SAVE_ARG(FUNCNAME, 2);                                                                                                                                         \
    SAVE_ARG(FUNCNAME, 3);                                                                                                                                         \
    SAVE_ARG(FUNCNAME, 4);                                                                                                                                         \
    SAVE_ARG(FUNCNAME, 5);                                                                                                                                         \
    SAVE_ARG(FUNCNAME, 6);                                                                                                                                         \
    if (ROOM_FOR_MORE_HISTORY(FUNCNAME))                                                                                                                           \
    {                                                                                                                                                              \
      SAVE_ARG_HISTORY(FUNCNAME, 0);                                                                                                                               \
      SAVE_ARG_HISTORY(FUNCNAME, 1);                                                                                                                               \
      SAVE_ARG_HISTORY(FUNCNAME, 2);                                                                                                                               \
      SAVE_ARG_HISTORY(FUNCNAME, 3);                                                                                                                               \
      SAVE_ARG_HISTORY(FUNCNAME, 4);                                                                                                                               \
      SAVE_ARG_HISTORY(FUNCNAME, 5);                                                                                                                               \
      SAVE_ARG_HISTORY(FUNCNAME, 6);                                                                                                                               \
    }                                                                                                                                                              \
    else                                                                                                                                                           \
    {                                                                                                                                                              \
      HISTORY_DROPPED(FUNCNAME);                                                                                                                                   \
    }                                                                                                                                                              \
    INCREMENT_CALL_COUNT(FUNCNAME);                                                                                                                                \
    REGISTER_CALL(FUNCNAME);                                                                                                                                       \
    if (FUNCNAME##_fake.custom_fake_seq_len)                                                                                                                       \
    {                                                                                                                                                              \
      if (FUNCNAME##_fake.custom_fake_seq_idx < FUNCNAME##_fake.custom_fake_seq_len)                                                                               \
      {                                                                                                                                                            \
        RETURN_TYPE ret = FUNCNAME##_fake.custom_fake_seq[FUNCNAME##_fake.custom_fake_seq_idx++](arg0, arg1, arg2, arg3, arg4, arg5, arg6);                        \
        SAVE_RET_HISTORY(FUNCNAME, ret);                                                                                                                           \
        return ret;                                                                                                                                                \
      }                                                                                                                                                            \
      else                                                                                                                                                         \
      {                                                                                                                                                            \
        RETURN_TYPE ret = FUNCNAME##_fake.custom_fake_seq[FUNCNAME##_fake.custom_fake_seq_len - 1](arg0, arg1, arg2, arg3, arg4, arg5, arg6);                      \
        SAVE_RET_HISTORY(FUNCNAME, ret);                                                                                                                           \
        return ret;                                                                                                                                                \
      }                                                                                                                                                            \
    }                                                                                                                                                              \
    if (FUNCNAME##_fake.custom_fake != NULL)                                                                                                                       \
    {                                                                                                                                                              \
      RETURN_TYPE ret = FUNCNAME##_fake.custom_fake(arg0, arg1, arg2, arg3, arg4, arg5, arg6);                                                                     \
      SAVE_RET_HISTORY(FUNCNAME, ret);                                                                                                                             \
      return ret;                                                                                                                                                  \
    }                                                                                                                                                              \
    RETURN_FAKE_RESULT(FUNCNAME)                                                                                                                                   \
  }                                                                                                                                                                \
  DEFINE_RESET_FUNCTION(FUNCNAME)

#define FAKE_VALUE_FUNC7(RETURN_TYPE, FUNCNAME, ARG0_TYPE, ARG1_TYPE, ARG2_TYPE, ARG3_TYPE, ARG4_TYPE, ARG5_TYPE, ARG6_TYPE)   \
  DECLARE_FAKE_VALUE_FUNC7(RETURN_TYPE, FUNCNAME, ARG0_TYPE, ARG1_TYPE, ARG2_TYPE, ARG3_TYPE, ARG4_TYPE, ARG5_TYPE, ARG6_TYPE) \
  DEFINE_FAKE_VALUE_FUNC7(RETURN_TYPE, FUNCNAME, ARG0_TYPE, ARG1_TYPE, ARG2_TYPE, ARG3_TYPE, ARG4_TYPE, ARG5_TYPE, ARG6_TYPE)

#define DECLARE_FAKE_VALUE_FUNC8(RETURN_TYPE, FUNCNAME, ARG0_TYPE, ARG1_TYPE, ARG2_TYPE, ARG3_TYPE, ARG4_TYPE, ARG5_TYPE, ARG6_TYPE, ARG7_TYPE)          \
  typedef struct FUNCNAME##_Fake                                                                                                                         \
  {                                                                                                                                                      \
    DECLARE_ARG(ARG0_TYPE, 0, FUNCNAME)                                                                                                                  \
    DECLARE_ARG(ARG1_TYPE, 1, FUNCNAME)                                                                                                                  \
    DECLARE_ARG(ARG2_TYPE, 2, FUNCNAME)                                                                                                                  \
    DECLARE_ARG(ARG3_TYPE, 3, FUNCNAME)                                                                                                                  \
    DECLARE_ARG(ARG4_TYPE, 4, FUNCNAME)                                                                                                                  \
    DECLARE_ARG(ARG5_TYPE, 5, FUNCNAME)                                                                                                                  \
    DECLARE_ARG(ARG6_TYPE, 6, FUNCNAME)                                                                                                                  \
    DECLARE_ARG(ARG7_TYPE, 7, FUNCNAME)                                                                                                                  \
    DECLARE_ALL_FUNC_COMMON                                                                                                                              \
    DECLARE_VALUE_FUNCTION_VARIABLES(RETURN_TYPE)                                                                                                        \
    DECLARE_RETURN_VALUE_HISTORY(RETURN_TYPE)                                                                                                            \
    DECLARE_CUSTOM_FAKE_SEQ_VARIABLES                                                                                                                    \
    CUSTOM_EFF_FUNCTION_TEMPLATE(RETURN_TYPE, custom_fake, ARG0_TYPE, ARG1_TYPE, ARG2_TYPE, ARG3_TYPE, ARG4_TYPE, ARG5_TYPE, ARG6_TYPE, ARG7_TYPE);      \
    CUSTOM_EFF_FUNCTION_TEMPLATE(RETURN_TYPE, *custom_fake_seq, ARG0_TYPE, ARG1_TYPE, ARG2_TYPE, ARG3_TYPE, ARG4_TYPE, ARG5_TYPE, ARG6_TYPE, ARG7_TYPE); \
  } FUNCNAME##_Fake;                                                                                                                                     \
  extern FUNCNAME##_Fake FUNCNAME##_fake;                                                                                                                \
  void FUNCNAME##_reset(void);                                                                                                                           \
  RETURN_TYPE EFF_GCC_FUNCTION_ATTRIBUTES FUNCNAME(ARG0_TYPE arg0, ARG1_TYPE arg1, ARG2_TYPE arg2, ARG3_TYPE arg3, ARG4_TYPE arg4, ARG5_TYPE arg5, ARG6_TYPE arg6, ARG7_TYPE arg7);

#define DEFINE_FAKE_VALUE_FUNC8(RETURN_TYPE, FUNCNAME, ARG0_TYPE, ARG1_TYPE, ARG2_TYPE, ARG3_TYPE, ARG4_TYPE, ARG5_TYPE, ARG6_TYPE, ARG7_TYPE)                                     \
  FUNCNAME##_Fake FUNCNAME##_fake;                                                                                                                                                 \
  RETURN_TYPE EFF_GCC_FUNCTION_ATTRIBUTES FUNCNAME(ARG0_TYPE arg0, ARG1_TYPE arg1, ARG2_TYPE arg2, ARG3_TYPE arg3, ARG4_TYPE arg4, ARG5_TYPE arg5, ARG6_TYPE arg6, ARG7_TYPE arg7) \
  {                                                                                                                                                                                \
    SAVE_ARG(FUNCNAME, 0);                                                                                                                                                         \
    SAVE_ARG(FUNCNAME, 1);                                                                                                                                                         \
    SAVE_ARG(FUNCNAME, 2);                                                                                                                                                         \
    SAVE_ARG(FUNCNAME, 3);                                                                                                                                                         \
    SAVE_ARG(FUNCNAME, 4);                                                                                                                                                         \
    SAVE_ARG(FUNCNAME, 5);                                                                                                                                                         \
    SAVE_ARG(FUNCNAME, 6);                                                                                                                                                         \
    SAVE_ARG(FUNCNAME, 7);                                                                                                                                                         \
    if (ROOM_FOR_MORE_HISTORY(FUNCNAME))                                                                                                                                           \
    {                                                                                                                                                                              \
      SAVE_ARG_HISTORY(FUNCNAME, 0);                                                                                                                                               \
      SAVE_ARG_HISTORY(FUNCNAME, 1);                                                                                                                                               \
      SAVE_ARG_HISTORY(FUNCNAME, 2);                                                                                                                                               \
      SAVE_ARG_HISTORY(FUNCNAME, 3);                                                                                                                                               \
      SAVE_ARG_HISTORY(FUNCNAME, 4);                                                                                                                                               \
      SAVE_ARG_HISTORY(FUNCNAME, 5);                                                                                                                                               \
      SAVE_ARG_HISTORY(FUNCNAME, 6);                                                                                                                                               \
      SAVE_ARG_HISTORY(FUNCNAME, 7);                                                                                                                                               \
    }                                                                                                                                                                              \
    else                                                                                                                                                                           \
    {                                                                                                                                                                              \
      HISTORY_DROPPED(FUNCNAME);                                                                                                                                                   \
    }                                                                                                                                                                              \
    INCREMENT_CALL_COUNT(FUNCNAME);                                                                                                                                                \
    REGISTER_CALL(FUNCNAME);                                                                                                                                                       \
    if (FUNCNAME##_fake.custom_fake_seq_len)                                                                                                                                       \
    {                                                                                                                                                                              \
      if (FUNCNAME##_fake.custom_fake_seq_idx < FUNCNAME##_fake.custom_fake_seq_len)                                                                                               \
      {                                                                                                                                                                            \
        RETURN_TYPE ret = FUNCNAME##_fake.custom_fake_seq[FUNCNAME##_fake.custom_fake_seq_idx++](arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7);                                  \
        SAVE_RET_HISTORY(FUNCNAME, ret);                                                                                                                                           \
        return ret;                                                                                                                                                                \
      }                                                                                                                                                                            \
      else                                                                                                                                                                         \
      {                                                                                                                                                                            \
        RETURN_TYPE ret = FUNCNAME##_fake.custom_fake_seq[FUNCNAME##_fake.custom_fake_seq_len - 1](arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7);                                \
        SAVE_RET_HISTORY(FUNCNAME, ret);                                                                                                                                           \
        return ret;                                                                                                                                                                \
      }                                                                                                                                                                            \
    }                                                                                                                                                                              \
    if (FUNCNAME##_fake.custom_fake != NULL)                                                                                                                                       \
    {                                                                                                                                                                              \
      RETURN_TYPE ret = FUNCNAME##_fake.custom_fake(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7);                                                                               \
      SAVE_RET_HISTORY(FUNCNAME, ret);                                                                                                                                             \
      return ret;                                                                                                                                                                  \
    }                                                                                                                                                                              \
    RETURN_FAKE_RESULT(FUNCNAME)                                                                                                                                                   \
  }                                                                                                                                                                                \
  DEFINE_RESET_FUNCTION(FUNCNAME)

#define FAKE_VALUE_FUNC8(RETURN_TYPE, FUNCNAME, ARG0_TYPE, ARG1_TYPE, ARG2_TYPE, ARG3_TYPE, ARG4_TYPE, ARG5_TYPE, ARG6_TYPE, ARG7_TYPE)   \
  DECLARE_FAKE_VALUE_FUNC8(RETURN_TYPE, FUNCNAME, ARG0_TYPE, ARG1_TYPE, ARG2_TYPE, ARG3_TYPE, ARG4_TYPE, ARG5_TYPE, ARG6_TYPE, ARG7_TYPE) \
  DEFINE_FAKE_VALUE_FUNC8(RETURN_TYPE, FUNCNAME, ARG0_TYPE, ARG1_TYPE, ARG2_TYPE, ARG3_TYPE, ARG4_TYPE, ARG5_TYPE, ARG6_TYPE, ARG7_TYPE)

#define DECLARE_FAKE_VALUE_FUNC9(RETURN_TYPE, FUNCNAME, ARG0_TYPE, ARG1_TYPE, ARG2_TYPE, ARG3_TYPE, ARG4_TYPE, ARG5_TYPE, ARG6_TYPE, ARG7_TYPE, ARG8_TYPE)          \
  typedef struct FUNCNAME##_Fake                                                                                                                                    \
  {                                                                                                                                                                 \
    DECLARE_ARG(ARG0_TYPE, 0, FUNCNAME)                                                                                                                             \
    DECLARE_ARG(ARG1_TYPE, 1, FUNCNAME)                                                                                                                             \
    DECLARE_ARG(ARG2_TYPE, 2, FUNCNAME)                                                                                                                             \
    DECLARE_ARG(ARG3_TYPE, 3, FUNCNAME)                                                                                                                             \
    DECLARE_ARG(ARG4_TYPE, 4, FUNCNAME)                                                                                                                             \
    DECLARE_ARG(ARG5_TYPE, 5, FUNCNAME)                                                                                                                             \
    DECLARE_ARG(ARG6_TYPE, 6, FUNCNAME)                                                                                                                             \
    DECLARE_ARG(ARG7_TYPE, 7, FUNCNAME)                                                                                                                             \
    DECLARE_ARG(ARG8_TYPE, 8, FUNCNAME)                                                                                                                             \
    DECLARE_ALL_FUNC_COMMON                                                                                                                                         \
    DECLARE_VALUE_FUNCTION_VARIABLES(RETURN_TYPE)                                                                                                                   \
    DECLARE_RETURN_VALUE_HISTORY(RETURN_TYPE)                                                                                                                       \
    DECLARE_CUSTOM_FAKE_SEQ_VARIABLES                                                                                                                               \
    CUSTOM_EFF_FUNCTION_TEMPLATE(RETURN_TYPE, custom_fake, ARG0_TYPE, ARG1_TYPE, ARG2_TYPE, ARG3_TYPE, ARG4_TYPE, ARG5_TYPE, ARG6_TYPE, ARG7_TYPE, ARG8_TYPE);      \
    CUSTOM_EFF_FUNCTION_TEMPLATE(RETURN_TYPE, *custom_fake_seq, ARG0_TYPE, ARG1_TYPE, ARG2_TYPE, ARG3_TYPE, ARG4_TYPE, ARG5_TYPE, ARG6_TYPE, ARG7_TYPE, ARG8_TYPE); \
  } FUNCNAME##_Fake;                                                                                                                                                \
  extern FUNCNAME##_Fake FUNCNAME##_fake;                                                                                                                           \
  void FUNCNAME##_reset(void);                                                                                                                                      \
  RETURN_TYPE EFF_GCC_FUNCTION_ATTRIBUTES FUNCNAME(ARG0_TYPE arg0, ARG1_TYPE arg1, ARG2_TYPE arg2, ARG3_TYPE arg3, ARG4_TYPE arg4, ARG5_TYPE arg5, ARG6_TYPE arg6, ARG7_TYPE arg7, ARG8_TYPE arg8);

#define DEFINE_FAKE_VALUE_FUNC9(RETURN_TYPE, FUNCNAME, ARG0_TYPE, ARG1_TYPE, ARG2_TYPE, ARG3_TYPE, ARG4_TYPE, ARG5_TYPE, ARG6_TYPE, ARG7_TYPE, ARG8_TYPE)                                          \
  FUNCNAME##_Fake FUNCNAME##_fake;                                                                                                                                                                 \
  RETURN_TYPE EFF_GCC_FUNCTION_ATTRIBUTES FUNCNAME(ARG0_TYPE arg0, ARG1_TYPE arg1, ARG2_TYPE arg2, ARG3_TYPE arg3, ARG4_TYPE arg4, ARG5_TYPE arg5, ARG6_TYPE arg6, ARG7_TYPE arg7, ARG8_TYPE arg8) \
  {                                                                                                                                                                                                \
    SAVE_ARG(FUNCNAME, 0);                                                                                                                                                                         \
    SAVE_ARG(FUNCNAME, 1);                                                                                                                                                                         \
    SAVE_ARG(FUNCNAME, 2);                                                                                                                                                                         \
    SAVE_ARG(FUNCNAME, 3);                                                                                                                                                                         \
    SAVE_ARG(FUNCNAME, 4);                                                                                                                                                                         \
    SAVE_ARG(FUNCNAME, 5);                                                                                                                                                                         \
    SAVE_ARG(FUNCNAME, 6);                                                                                                                                                                         \
    SAVE_ARG(FUNCNAME, 7);                                                                                                                                                                         \
    SAVE_ARG(FUNCNAME, 8);                                                                                                                                                                         \
    if (ROOM_FOR_MORE_HISTORY(FUNCNAME))                                                                                                                                                           \
    {                                                                                                                                                                                              \
      SAVE_ARG_HISTORY(FUNCNAME, 0);                                                                                                                                                               \
      SAVE_ARG_HISTORY(FUNCNAME, 1);                                                                                                                                                               \
      SAVE_ARG_HISTORY(FUNCNAME, 2);                                                                                                                                                               \
      SAVE_ARG_HISTORY(FUNCNAME, 3);                                                                                                                                                               \
      SAVE_ARG_HISTORY(FUNCNAME, 4);                                                                                                                                                               \
      SAVE_ARG_HISTORY(FUNCNAME, 5);                                                                                                                                                               \
      SAVE_ARG_HISTORY(FUNCNAME, 6);                                                                                                                                                               \
      SAVE_ARG_HISTORY(FUNCNAME, 7);                                                                                                                                                               \
      SAVE_ARG_HISTORY(FUNCNAME, 8);                                                                                                                                                               \
    }                                                                                                                                                                                              \
    else                                                                                                                                                                                           \
    {                                                                                                                                                                                              \
      HISTORY_DROPPED(FUNCNAME);                                                                                                                                                                   \
    }                                                                                                                                                                                              \
    INCREMENT_CALL_COUNT(FUNCNAME);                                                                                                                                                                \
    REGISTER_CALL(FUNCNAME);                                                                                                                                                                       \
    if (FUNCNAME##_fake.custom_fake_seq_len)                                                                                                                                                       \
    {                                                                                                                                                                                              \
      if (FUNCNAME##_fake.custom_fake_seq_idx < FUNCNAME##_fake.custom_fake_seq_len)                                                                                                               \
      {                                                                                                                                                                                            \
        RETURN_TYPE ret = FUNCNAME##_fake.custom_fake_seq[FUNCNAME##_fake.custom_fake_seq_idx++](arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);                                            \
        SAVE_RET_HISTORY(FUNCNAME, ret);                                                                                                                                                           \
        return ret;                                                                                                                                                                                \
      }                                                                                                                                                                                            \
      else                                                                                                                                                                                         \
      {                                                                                                                                                                                            \
        RETURN_TYPE ret = FUNCNAME##_fake.custom_fake_seq[FUNCNAME##_fake.custom_fake_seq_len - 1](arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);                                          \
        SAVE_RET_HISTORY(FUNCNAME, ret);                                                                                                                                                           \
        return ret;                                                                                                                                                                                \
      }                                                                                                                                                                                            \
    }                                                                                                                                                                                              \
    if (FUNCNAME##_fake.custom_fake != NULL)                                                                                                                                                       \
    {                                                                                                                                                                                              \
      RETURN_TYPE ret = FUNCNAME##_fake.custom_fake(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8);                                                                                         \
      SAVE_RET_HISTORY(FUNCNAME, ret);                                                                                                                                                             \
      return ret;                                                                                                                                                                                  \
    }                                                                                                                                                                                              \
    RETURN_FAKE_RESULT(FUNCNAME)                                                                                                                                                                   \
  }                                                                                                                                                                                                \
  DEFINE_RESET_FUNCTION(FUNCNAME)

#define FAKE_VALUE_FUNC9(RETURN_TYPE, FUNCNAME, ARG0_TYPE, ARG1_TYPE, ARG2_TYPE, ARG3_TYPE, ARG4_TYPE, ARG5_TYPE, ARG6_TYPE, ARG7_TYPE, ARG8_TYPE)   \
  DECLARE_FAKE_VALUE_FUNC9(RETURN_TYPE, FUNCNAME, ARG0_TYPE, ARG1_TYPE, ARG2_TYPE, ARG3_TYPE, ARG4_TYPE, ARG5_TYPE, ARG6_TYPE, ARG7_TYPE, ARG8_TYPE) \
  DEFINE_FAKE_VALUE_FUNC9(RETURN_TYPE, FUNCNAME, ARG0_TYPE, ARG1_TYPE, ARG2_TYPE, ARG3_TYPE, ARG4_TYPE, ARG5_TYPE, ARG6_TYPE, ARG7_TYPE, ARG8_TYPE)

#define DECLARE_FAKE_VALUE_FUNC10(RETURN_TYPE, FUNCNAME, ARG0_TYPE, ARG1_TYPE, ARG2_TYPE, ARG3_TYPE, ARG4_TYPE, ARG5_TYPE, ARG6_TYPE, ARG7_TYPE, ARG8_TYPE, ARG9_TYPE)         \
  typedef struct FUNCNAME##_Fake                                                                                                                                               \
  {                                                                                                                                                                            \
    DECLARE_ARG(ARG0_TYPE, 0, FUNCNAME)                                                                                                                                        \
    DECLARE_ARG(ARG1_TYPE, 1, FUNCNAME)                                                                                                                                        \
    DECLARE_ARG(ARG2_TYPE, 2, FUNCNAME)                                                                                                                                        \
    DECLARE_ARG(ARG3_TYPE, 3, FUNCNAME)                                                                                                                                        \
    DECLARE_ARG(ARG4_TYPE, 4, FUNCNAME)                                                                                                                                        \
    DECLARE_ARG(ARG5_TYPE, 5, FUNCNAME)                                                                                                                                        \
    DECLARE_ARG(ARG6_TYPE, 6, FUNCNAME)                                                                                                                                        \
    DECLARE_ARG(ARG7_TYPE, 7, FUNCNAME)                                                                                                                                        \
    DECLARE_ARG(ARG8_TYPE, 8, FUNCNAME)                                                                                                                                        \
    DECLARE_ARG(ARG9_TYPE, 9, FUNCNAME)                                                                                                                                        \
    DECLARE_ALL_FUNC_COMMON                                                                                                                                                    \
    DECLARE_VALUE_FUNCTION_VARIABLES(RETURN_TYPE)                                                                                                                              \
    DECLARE_RETURN_VALUE_HISTORY(RETURN_TYPE)                                                                                                                                  \
    DECLARE_CUSTOM_FAKE_SEQ_VARIABLES                                                                                                                                          \
    CUSTOM_EFF_FUNCTION_TEMPLATE(RETURN_TYPE, custom_fake, ARG0_TYPE, ARG1_TYPE, ARG2_TYPE, ARG3_TYPE, ARG4_TYPE, ARG5_TYPE, ARG6_TYPE, ARG7_TYPE, ARG8_TYPE, ARG9_TYPE);      \
    CUSTOM_EFF_FUNCTION_TEMPLATE(RETURN_TYPE, *custom_fake_seq, ARG0_TYPE, ARG1_TYPE, ARG2_TYPE, ARG3_TYPE, ARG4_TYPE, ARG5_TYPE, ARG6_TYPE, ARG7_TYPE, ARG8_TYPE, ARG9_TYPE); \
  } FUNCNAME##_Fake;                                                                                                                                                           \
  extern FUNCNAME##_Fake FUNCNAME##_fake;                                                                                                                                      \
  void FUNCNAME##_reset(void);                                                                                                                                                 \
  RETURN_TYPE EFF_GCC_FUNCTION_ATTRIBUTES FUNCNAME(ARG0_TYPE arg0, ARG1_TYPE arg1, ARG2_TYPE arg2, ARG3_TYPE arg3, ARG4_TYPE arg4, ARG5_TYPE arg5, ARG6_TYPE arg6, ARG7_TYPE arg7, ARG8_TYPE arg8, ARG9_TYPE arg9);

#define DEFINE_FAKE_VALUE_FUNC10(RETURN_TYPE, FUNCNAME, ARG0_TYPE, ARG1_TYPE, ARG2_TYPE, ARG3_TYPE, ARG4_TYPE, ARG5_TYPE, ARG6_TYPE, ARG7_TYPE, ARG8_TYPE, ARG9_TYPE)                                              \
  FUNCNAME##_Fake FUNCNAME##_fake;                                                                                                                                                                                 \
  RETURN_TYPE EFF_GCC_FUNCTION_ATTRIBUTES FUNCNAME(ARG0_TYPE arg0, ARG1_TYPE arg1, ARG2_TYPE arg2, ARG3_TYPE arg3, ARG4_TYPE arg4, ARG5_TYPE arg5, ARG6_TYPE arg6, ARG7_TYPE arg7, ARG8_TYPE arg8, ARG9_TYPE arg9) \
  {                                                                                                                                                                                                                \
    SAVE_ARG(FUNCNAME, 0);                                                                                                                                                                                         \
    SAVE_ARG(FUNCNAME, 1);                                                                                                                                                                                         \
    SAVE_ARG(FUNCNAME, 2);                                                                                                                                                                                         \
    SAVE_ARG(FUNCNAME, 3);                                                                                                                                                                                         \
    SAVE_ARG(FUNCNAME, 4);                                                                                                                                                                                         \
    SAVE_ARG(FUNCNAME, 5);                                                                                                                                                                                         \
    SAVE_ARG(FUNCNAME, 6);                                                                                                                                                                                         \
    SAVE_ARG(FUNCNAME, 7);                                                                                                                                                                                         \
    SAVE_ARG(FUNCNAME, 8);                                                                                                                                                                                         \
    SAVE_ARG(FUNCNAME, 9);                                                                                                                                                                                         \
    if (ROOM_FOR_MORE_HISTORY(FUNCNAME))                                                                                                                                                                           \
    {                                                                                                                                                                                                              \
      SAVE_ARG_HISTORY(FUNCNAME, 0);                                                                                                                                                                               \
      SAVE_ARG_HISTORY(FUNCNAME, 1);                                                                                                                                                                               \
      SAVE_ARG_HISTORY(FUNCNAME, 2);                                                                                                                                                                               \
      SAVE_ARG_HISTORY(FUNCNAME, 3);                                                                                                                                                                               \
      SAVE_ARG_HISTORY(FUNCNAME, 4);                                                                                                                                                                               \
      SAVE_ARG_HISTORY(FUNCNAME, 5);                                                                                                                                                                               \
      SAVE_ARG_HISTORY(FUNCNAME, 6);                                                                                                                                                                               \
      SAVE_ARG_HISTORY(FUNCNAME, 7);                                                                                                                                                                               \
      SAVE_ARG_HISTORY(FUNCNAME, 8);                                                                                                                                                                               \
      SAVE_ARG_HISTORY(FUNCNAME, 9);                                                                                                                                                                               \
    }                                                                                                                                                                                                              \
    else                                                                                                                                                                                                           \
    {                                                                                                                                                                                                              \
      HISTORY_DROPPED(FUNCNAME);                                                                                                                                                                                   \
    }                                                                                                                                                                                                              \
    INCREMENT_CALL_COUNT(FUNCNAME);                                                                                                                                                                                \
    REGISTER_CALL(FUNCNAME);                                                                                                                                                                                       \
    if (FUNCNAME##_fake.custom_fake_seq_len)                                                                                                                                                                       \
    {                                                                                                                                                                                                              \
      if (FUNCNAME##_fake.custom_fake_seq_idx < FUNCNAME##_fake.custom_fake_seq_len)                                                                                                                               \
      {                                                                                                                                                                                                            \
        RETURN_TYPE ret = FUNCNAME##_fake.custom_fake_seq[FUNCNAME##_fake.custom_fake_seq_idx++](arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9);                                                      \
        SAVE_RET_HISTORY(FUNCNAME, ret);                                                                                                                                                                           \
        return ret;                                                                                                                                                                                                \
      }                                                                                                                                                                                                            \
      else                                                                                                                                                                                                         \
      {                                                                                                                                                                                                            \
        RETURN_TYPE ret = FUNCNAME##_fake.custom_fake_seq[FUNCNAME##_fake.custom_fake_seq_len - 1](arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9);                                                    \
        SAVE_RET_HISTORY(FUNCNAME, ret);                                                                                                                                                                           \
        return ret;                                                                                                                                                                                                \
      }                                                                                                                                                                                                            \
    }                                                                                                                                                                                                              \
    if (FUNCNAME##_fake.custom_fake != NULL)                                                                                                                                                                       \
    {                                                                                                                                                                                                              \
      RETURN_TYPE ret = FUNCNAME##_fake.custom_fake(arg0, arg1, arg2, arg3, arg4, arg5, arg6, arg7, arg8, arg9);                                                                                                   \
      SAVE_RET_HISTORY(FUNCNAME, ret);                                                                                                                                                                             \
      return ret;                                                                                                                                                                                                  \
    }                                                                                                                                                                                                              \
    RETURN_FAKE_RESULT(FUNCNAME)                                                                                                                                                                                   \
  }                                                                                                                                                                                                                \
  DEFINE_RESET_FUNCTION(FUNCNAME)

#define FAKE_VALUE_FUNC10(RETURN_TYPE, FUNCNAME, ARG0_TYPE, ARG1_TYPE, ARG2_TYPE, ARG3_TYPE, ARG4_TYPE, ARG5_TYPE, ARG6_TYPE, ARG7_TYPE, ARG8_TYPE, ARG9_TYPE)   \
  DECLARE_FAKE_VALUE_FUNC10(RETURN_TYPE, FUNCNAME, ARG0_TYPE, ARG1_TYPE, ARG2_TYPE, ARG3_TYPE, ARG4_TYPE, ARG5_TYPE, ARG6_TYPE, ARG7_TYPE, ARG8_TYPE, ARG9_TYPE) \
  DEFINE_FAKE_VALUE_FUNC10(RETURN_TYPE, FUNCNAME, ARG0_TYPE, ARG1_TYPE, ARG2_TYPE, ARG3_TYPE, ARG4_TYPE, ARG5_TYPE, ARG6_TYPE, ARG7_TYPE, ARG8_TYPE, ARG9_TYPE)
/** @} */ // End of FakeFunctionMacros

/**
 * @brief Expands a macro argument.
 *
 * Used to force macro expansion, especially needed to work around
 * expansion limitations in MSVC's preprocessor.
 *
 * @param x The macro argument to expand.
 */
#define EXPAND(x) x

/**
 * @defgroup PP_CountArgumentMacros Macros for counting variadic arguments
 * @brief Count the number of variadic arguments, optionally subtracting 1 or 2.
 *
 * @{
 */
#define PP_NARG_MINUS2(...) EXPAND(PP_NARG_MINUS2_(__VA_ARGS__, PP_RSEQ_N_MINUS2()))
#define PP_NARG_MINUS2_(...) EXPAND(PP_ARG_MINUS2_N(__VA_ARGS__))
#define PP_ARG_MINUS2_N(returnVal, _0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, N, ...) N
#define PP_RSEQ_N_MINUS2() 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
#define PP_NARG_MINUS1(...) EXPAND(PP_NARG_MINUS1_(__VA_ARGS__, PP_RSEQ_N_MINUS1()))
#define PP_NARG_MINUS1_(...) EXPAND(PP_ARG_MINUS1_N(__VA_ARGS__))
#define PP_ARG_MINUS1_N(_0, _1, _2, _3, _4, _5, _6, _7, _8, _9, _10, N, ...) N
#define PP_RSEQ_N_MINUS1() 10, 9, 8, 7, 6, 5, 4, 3, 2, 1, 0
/** @} */ // End of PP_CountArgumentMacros

/**
 * @defgroup FakeFunctionDispatch Internal dispatch helpers
 * @brief Internal macros to expand fake functions based on argument count.
 * @{
 */
#define FUNC_VALUE_(N, ...) EXPAND(FUNC_VALUE_N(N, __VA_ARGS__))
#define FUNC_VALUE_N(N, ...) EXPAND(FAKE_VALUE_FUNC##N(__VA_ARGS__))
#define FUNC_VOID_(N, ...) EXPAND(FUNC_VOID_N(N, __VA_ARGS__))
#define FUNC_VOID_N(N, ...) EXPAND(FAKE_VOID_FUNC##N(__VA_ARGS__))
/** @} */ // End of FakeFunctionDispatch

/**
 * @defgroup FakeFunctionDeclarationDispatch Internal declaration dispatch helpers
 * @brief Internal macros for dispatching fake function declarations by argument count.
 * @{
 */
#define DECLARE_FUNC_VALUE_(N, ...) EXPAND(DECLARE_FUNC_VALUE_N(N, __VA_ARGS__))
#define DECLARE_FUNC_VALUE_N(N, ...) EXPAND(DECLARE_FAKE_VALUE_FUNC##N(__VA_ARGS__))
#define DECLARE_FUNC_VOID_(N, ...) EXPAND(DECLARE_FUNC_VOID_N(N, __VA_ARGS__))
#define DECLARE_FUNC_VOID_N(N, ...) EXPAND(DECLARE_FAKE_VOID_FUNC##N(__VA_ARGS__))
/** @} */ // End of FakeFunctionDeclarationDispatch

/**
 * @defgroup FakeFunctionDefinitionDispatch Internal definition dispatch helpers
 * @brief Internal macros for dispatching fake function definitions by argument count.
 * @{
 */
#define DEFINE_FUNC_VALUE_(N, ...) EXPAND(DEFINE_FUNC_VALUE_N(N, __VA_ARGS__))
#define DEFINE_FUNC_VALUE_N(N, ...) EXPAND(DEFINE_FAKE_VALUE_FUNC##N(__VA_ARGS__))
#define DEFINE_FUNC_VOID_(N, ...) EXPAND(DEFINE_FUNC_VOID_N(N, __VA_ARGS__))
#define DEFINE_FUNC_VOID_N(N, ...) EXPAND(DEFINE_FAKE_VOID_FUNC##N(__VA_ARGS__))
/** @} */ // End of FakeFunctionDefinitionDispatch

// clang-format on

/*******************************************************************************
 * PUBLIC TYPEDEFS
 ******************************************************************************/

/**
 * @brief Generic function pointer type used for tracking calls.
 */
typedef void (*eff_function_t)(void);

/**
 * @brief Structure holding global call history information.
 *
 * Stores an array of function pointers representing the call history of
 * fake functions, along with the current index.
 */
typedef struct
{
  eff_function_t call_history[EFF_CALL_HISTORY_LEN]; /**< Array of called
                                                          functions */
  unsigned int call_history_idx;                     /**< call_history index*/
} eff_globals_t;

/*******************************************************************************
 * PUBLIC VARIABLES
 ******************************************************************************/

/**
 * @brief Global instance tracking call history of all fake functions.
 *
 * Shall be defined with DEFINE_EFF_GLOBALS macro
 */
extern eff_globals_t eff;

/*******************************************************************************
 * PUBLIC FUNCTIONS
 ******************************************************************************/

#endif /* EFF_H */
