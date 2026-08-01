/*******************************************************************************
 * @brief Dummy system calls implementation
 *
 * Required to eliminate warnings.
 *
 * @copyright
 * Copyright (c) 2026 FBS93.
 * See the LICENSE file of this project for license details.
 * This notice shall be retained in all copies or substantial portions
 * of the software.
 *
 * @warning
 * This software is provided "as is", without any express or implied warranty.
 * The user assumes all responsibility for its use and any consequences.
 ******************************************************************************/

/*******************************************************************************
 * INCLUDES
 ******************************************************************************/

/* -----------------------------------------------------------------------------
 * System library headers
 * -------------------------------------------------------------------------- */
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <errno.h>
#include <stddef.h>

/* -----------------------------------------------------------------------------
 * External library headers
 * -------------------------------------------------------------------------- */

/* -----------------------------------------------------------------------------
 * Project-specific headers
 * -------------------------------------------------------------------------- */
#include "emf.h"

/*******************************************************************************
 * PRIVATE MACROS
 ******************************************************************************/

/*******************************************************************************
 * PRIVATE TYPEDEFS
 ******************************************************************************/

/*******************************************************************************
 * PRIVATE VARIABLES
 ******************************************************************************/

/*******************************************************************************
 * PUBLIC VARIABLES
 ******************************************************************************/

/*******************************************************************************
 * PRIVATE FUNCTIONS
 ******************************************************************************/

/* -----------------------------------------------------------------------------
 * Private function declarations
 * -------------------------------------------------------------------------- */

/* -----------------------------------------------------------------------------
 * Private function definitions
 * -------------------------------------------------------------------------- */

/*******************************************************************************
 * PUBLIC FUNCTIONS
 ******************************************************************************/

// Required by the newlib syscall ABI.
// NOLINTNEXTLINE(bugprone-reserved-identifier)
ssize_t _write(int fd, const void* buf, size_t count)
{
  EMF_UTILS_UNUSED_PARAM(fd);
  EMF_UTILS_UNUSED_PARAM(buf);
  EMF_UTILS_UNUSED_PARAM(count);

  errno = ENOSYS;
  return -1;
}

// Required by the newlib syscall ABI.
// NOLINTNEXTLINE(bugprone-reserved-identifier)
ssize_t _read(int fd, void* buf, size_t count)
{
  EMF_UTILS_UNUSED_PARAM(fd);
  EMF_UTILS_UNUSED_PARAM(buf);
  EMF_UTILS_UNUSED_PARAM(count);

  errno = ENOSYS;
  return -1;
}

// Required by the newlib syscall ABI.
// NOLINTNEXTLINE(bugprone-reserved-identifier)
void* _sbrk(ptrdiff_t incr)
{
  EMF_UTILS_UNUSED_PARAM(incr);

  errno = ENOMEM;
  return (void*)-1;
}

// Required by the newlib syscall ABI.
// NOLINTNEXTLINE(bugprone-reserved-identifier)
int _close(int fd)
{
  EMF_UTILS_UNUSED_PARAM(fd);

  errno = ENOSYS;
  return -1;
}

// Required by the newlib syscall ABI.
// NOLINTNEXTLINE(bugprone-reserved-identifier)
off_t _lseek(int fd, off_t offset, int whence)
{
  EMF_UTILS_UNUSED_PARAM(fd);
  EMF_UTILS_UNUSED_PARAM(offset);
  EMF_UTILS_UNUSED_PARAM(whence);

  errno = ENOSYS;
  return -1;
}

// Required by the newlib syscall ABI.
// NOLINTNEXTLINE(bugprone-reserved-identifier)
int _fstat(int fd, struct stat* st)
{
  EMF_UTILS_UNUSED_PARAM(fd);
  EMF_UTILS_UNUSED_PARAM(st);

  errno = ENOSYS;
  return -1;
}

// Required by the newlib syscall ABI.
// NOLINTNEXTLINE(bugprone-reserved-identifier)
int _isatty(int fd)
{
  EMF_UTILS_UNUSED_PARAM(fd);

  return 0;
}

// Required by the newlib syscall ABI.
// NOLINTNEXTLINE(bugprone-reserved-identifier)
void _exit(int status)
{
  EMF_UTILS_UNUSED_PARAM(status);

  while (1)
  {
    // For-ever.
  }
}

// Required by the newlib syscall ABI.
// NOLINTNEXTLINE(bugprone-reserved-identifier)
int _kill(pid_t pid, int sig)
{
  EMF_UTILS_UNUSED_PARAM(pid);
  EMF_UTILS_UNUSED_PARAM(sig);

  errno = ENOSYS;
  return -1;
}

// Required by the newlib syscall ABI.
// NOLINTNEXTLINE(bugprone-reserved-identifier)
pid_t _getpid(void)
{
  return 1;
}
