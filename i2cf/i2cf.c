/*
The I2C driver seems to enforce an arbitrary minimum limit of 10 kHz on the
clock.  See if we can get down to 4 kHz.

Tested with RPi Zero W, verified 10 kHz and 3815 Hz with a scope.
*/

#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>



#define  CORE_CLK  (250*1000*1000)

// "BSC" is Broadcom Serial Controller, which means I2C controller
#define  BSC_SIZE  0x20
#define  BSC1_BASE  0x20804000
#define  BSC_DIV_OFFSET   0x14


uint32_t  *registers = NULL;

#define  DIV  (registers [BSC_DIV_OFFSET / sizeof (*registers)])



int  main (int argc, char *argv[])
{
  int  mem_fd = 0;

  if ((mem_fd = open ("/dev/mem", O_RDWR)) < 0)
  {
    perror ("open /dev/mem");
  }
  else if ((registers = (uint32_t *) mmap (NULL, BSC_SIZE, PROT_READ|PROT_WRITE,
            MAP_SHARED, mem_fd, BSC1_BASE)) == MAP_FAILED)
  {
    perror ("mmap");
  }
  else if (close (mem_fd) < 0)
  {
    perror ("close");
  }
  else {
    // SCL frequency is CORE_CLK / DIV
    // If no parameter is given then show the current frequency
    if (argc != 2)
    {
      printf ("DIV: %08x (%u Hz)\n", DIV, CORE_CLK / DIV);
    }
    // If a single parameter is given then it should be interpreted as the
    // desired frequency
    else {
      // Even though DIV is a 32-bit register, the MS 16 bits should be zero, so
      // the divider ranges from 65535 down to 0 (which is interpreted as
      // 32,768).  The range of frequencies should then be CORE_CLK / 1 down to
      // CORE_CLK / 65535
      #define  FREQ_MAX  (CORE_CLK / 1)
      #define  FREQ_MIN  (CORE_CLK / 65535 + 1)
      uint32_t  f = atoi (argv [1]);
      if (f < FREQ_MIN  ||  FREQ_MAX < f)
      {
        fprintf (stderr, "Frequency must be in the range %u to %u\n", FREQ_MIN, FREQ_MAX);
        exit (1);
      }
      else {
        uint32_t  new_div = CORE_CLK / f;
        printf ("f: %u  DIV: %08x\n", f, new_div);
        DIV = new_div;
      }
    }

    if (munmap (registers, BSC_SIZE) < 0)
    {
      perror ("munmap");
    }
  }
}

