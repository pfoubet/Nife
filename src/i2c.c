/* Copyright (C) 2011-2022  Patrick H. E. Foubet - S.E.R.I.A.N.E.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or any
later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>
*******************************************************************/
/* i2c.c */

#ifdef HAVE_CONFIG_H
#include "../config.h"
#endif

#ifdef HAVE_LINUX_I2C_DEV_H
#include <linux/i2c-dev.h>
#ifdef I2C_FUNC_I2C
#define _WITH_I2C
#endif
#endif

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef _WITH_I2C
/*
    This part is a merge of Linux i2c-tools code and Nife code.
    i2c-tools are user-space programs to detect, scan I2C devices, 
    and read and/or write an I2C register.
    Copyright (C) 2005-2012  Jean Delvare <jdelvare@suse.de>

    Copyright (C) 2001-2003  Frodo Looijaard <frodol@dds.nl>, and
                             Mark D. Studebaker <mdsxyz123@yahoo.com>
    Copyright (C) 2004-2005  Jean Delvare

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
*/
#include "i2cbusses.c"  /* add code for I2C on Linux */

#define MODE_AUTO       0
#define MODE_QUICK      1
#define MODE_READ       2
#define MODE_FUNC       3

/*
 * Print the installed i2c busses. The format is those of Linux 2.4's
 * /proc/bus/i2c for historical compatibility reasons.
 */
static void print_i2c_busses(void)
{
        struct i2c_adap *adapters;
        int count;

        adapters = gather_i2c_busses();
        if (adapters == NULL) {
                fprintf(stderr, "Error: Out of memory!\n");
                return;
        }
        printf("i2c-id\tfunction  \tadapter name\t\t\t\talgorithm\n");
        for (count = 0; adapters[count].name; count++) {
                printf("%d\t%-10s\t%-32s\t%s\n",
                        adapters[count].nr, adapters[count].funcs,
                        adapters[count].name, adapters[count].algo);
        }
        free_adapters(adapters);
}

static int scan_i2c_bus(int file, int mode, int first, int last)
{
int i, j;
int res;
        printf("     0  1  2  3  4  5  6  7  8  9  a  b  c  d  e  f\n");
        for (i = 0; i < 128; i += 16) {
                printf("%02x: ", i);
                for(j = 0; j < 16; j++) {
                        fflush(stdout);

                        /* Skip unwanted addresses */
                        if (i+j < first || i+j > last) {
                                printf("   ");
                                continue;
                        }

                        /* Set slave address */
                        if (ioctl(file, I2C_SLAVE, i+j) < 0) {
                                if (errno == EBUSY) {
                                        printf("UU ");
                                        continue;
                                } else {
                                        fprintf(stderr, "Error: Could not set "
                                                "address to 0x%02x: %s\n", i+j,
                                                strerror(errno));
                                        return -1;
                                }
                        }

                        /* Probe this address */
                        switch (mode) {
                        case MODE_QUICK:
                                /* This is known to corrupt the Atmel AT24RF08
                                   EEPROM */
                                res = i2c_smbus_write_quick(file,
                                      I2C_SMBUS_WRITE);
                                break;
                        case MODE_READ:
                                /* This is known to lock SMBus on various
                                   write-only chips (mainly clock chips) */
                                res = i2c_smbus_read_byte(file);
                                break;
                        default:
                                if ((i+j >= 0x30 && i+j <= 0x37)
                                 || (i+j >= 0x50 && i+j <= 0x5F))
                                        res = i2c_smbus_read_byte(file);
                                else
                                        res = i2c_smbus_write_quick(file,
                                              I2C_SMBUS_WRITE);
                        }
                        if (res < 0)
                                printf("-- ");
                        else
                                printf("%02x ", i+j);
                }
                printf("\n");
        }
        return 0;
}

static int check_write_funcs(int file, int size, int pec)
{
unsigned long funcs;
        /* check adapter functionality */
        if (ioctl(file, I2C_FUNCS, &funcs) < 0) {
                fprintf(stderr, "Error: Could not get the adapter "
                        "functionality matrix: %s\n", strerror(errno));
                return -1;
        }
        switch (size) {
        case I2C_SMBUS_BYTE:
                if (!(funcs & I2C_FUNC_SMBUS_WRITE_BYTE)) {
                        fprintf(stderr, MISSING_FUNC_FMT, "SMBus send byte");
                        return -1;
                }
                break;
        case I2C_SMBUS_BYTE_DATA:
                if (!(funcs & I2C_FUNC_SMBUS_WRITE_BYTE_DATA)) {
                        fprintf(stderr, MISSING_FUNC_FMT, "SMBus write byte");
                        return -1;
                }
                break;
        case I2C_SMBUS_WORD_DATA:
                if (!(funcs & I2C_FUNC_SMBUS_WRITE_WORD_DATA)) {
                        fprintf(stderr, MISSING_FUNC_FMT, "SMBus write word");
                        return -1;
                }
                break;
        case I2C_SMBUS_BLOCK_DATA:
                if (!(funcs & I2C_FUNC_SMBUS_WRITE_BLOCK_DATA)) {
                        fprintf(stderr, MISSING_FUNC_FMT, "SMBus block write");
                        return -1;
                }
                break;
        case I2C_SMBUS_I2C_BLOCK_DATA:
                if (!(funcs & I2C_FUNC_SMBUS_WRITE_I2C_BLOCK)) {
                        fprintf(stderr, MISSING_FUNC_FMT, "I2C block write");
                        return -1;
                }
                break;
        }
        if (pec && !(funcs & (I2C_FUNC_SMBUS_PEC | I2C_FUNC_I2C))) {
                fprintf(stderr, "Warning: Adapter does "
                        "not seem to support PEC\n");
        }
        return 0;
}

static int check_read_funcs(int file, int size, int daddress, int pec)
{
unsigned long funcs;
        /* check adapter functionality */
        if (ioctl(file, I2C_FUNCS, &funcs) < 0) {
                fprintf(stderr, "Error: Could not get the adapter "
                        "functionality matrix: %s\n", strerror(errno));
                return -1;
        }
        switch (size) {
        case I2C_SMBUS_BYTE:
                if (!(funcs & I2C_FUNC_SMBUS_READ_BYTE)) {
                        fprintf(stderr, MISSING_FUNC_FMT, "SMBus receive byte");
                        return -1;
                }
                if (daddress >= 0
                 && !(funcs & I2C_FUNC_SMBUS_WRITE_BYTE)) {
                        fprintf(stderr, MISSING_FUNC_FMT, "SMBus send byte");
                        return -1;
                }
                break;
        case I2C_SMBUS_BYTE_DATA:
                if (!(funcs & I2C_FUNC_SMBUS_READ_BYTE_DATA)) {
                        fprintf(stderr, MISSING_FUNC_FMT, "SMBus read byte");
                        return -1;
                }
                break;
        case I2C_SMBUS_WORD_DATA:
                if (!(funcs & I2C_FUNC_SMBUS_READ_WORD_DATA)) {
                        fprintf(stderr, MISSING_FUNC_FMT, "SMBus read word");
                        return -1;
                }
                break;
        }
        if (pec
         && !(funcs & (I2C_FUNC_SMBUS_PEC | I2C_FUNC_I2C))) {
                fprintf(stderr, "Warning: Adapter does "
                        "not seem to support PEC\n");
        }
        return 0;
}

#endif  /* for _WITH_I2C */
/* *********************************************************************/

#include "stackN.h"
#include "i2c.h"
#include "err.h"
#include "debug.h"

void IF_listI2C (void)
{
#ifdef _WITH_I2C
    print_i2c_busses();
#else
    messErr(49);
#endif
}

void IFD_listI2C (void)
{
    _IFD_BEGIN_
    IF_listI2C();
    _IFD_END_
}

void IF_showI2C (void)
{
long n;
    if (!getParLong(&n)) return;

#ifdef _WITH_I2C
int i2cbus, file;
int mode=MODE_AUTO;
int first = 0x03, last = 0x77;
unsigned long funcs;
char Bus[10], filename[20];
     sprintf(Bus, "%ld",n);
     i2cbus = lookup_i2c_bus(Bus);
     if (i2cbus < 0) messErr(50);
     else {
        file = open_i2c_dev(i2cbus, filename, sizeof(filename), 0);
        if (file < 0) messErr(51);
        else {
           if (ioctl(file, I2C_FUNCS, &funcs) < 0)  messErr(52);
           else scan_i2c_bus(file, mode, first, last);
           close(file);
        }
     }
#else
     messErr(49);
#endif
}

void IFD_showI2C (void)
{
    _IFD_BEGIN_
    IF_showI2C();
    _IFD_END_
}

static void i2c_read(int id, int add, int off)
{
#ifdef _WITH_I2C
int i2cbus, file, address, res;
int size = I2C_SMBUS_BYTE_DATA;
int pec = 0,force = 0;
char Buf[10], filename[20];
     sprintf(Buf, "%ld",id);
     i2cbus = lookup_i2c_bus(Buf);
     if (i2cbus < 0) messErr(50);
     else {
        sprintf(Buf, "%ld",add);
        address = parse_i2c_address(Buf);
        if (address < 0) messErr(53);
        else {
            if (off == -1) size = I2C_SMBUS_BYTE;
            if (off < -1 || off > 0xff) messErr(54);
            else {
               file = open_i2c_dev(i2cbus, filename, sizeof(filename), 0);
               if (file < 0
                   || check_read_funcs(file, size, off, pec)
                   || set_slave_addr(file, address, force)) messErr(51);
               else {
                  if (size == I2C_SMBUS_BYTE)
                     res = i2c_smbus_read_byte(file);
                  else
                     res = i2c_smbus_read_byte_data(file, off);
                  close(file);
                  if (res < 0) messErr(57);
                  else putLong((long long)res);
               }
            }
        }
     }
#else
    messErr(49);
#endif
}

void IF_I2CRead (void)
{
long id, add, off;
    if (!getParLong(&id)) return;  /* I2C ID */
    if (!getParLong(&add)) return; /* CHIP ADDRESS */
    if (!getParLong(&off)) return; /* DATA ADDRESS (OFFSET) */
    i2c_read(id, add, off);
}

static void i2c_write(int id, int add, int off, int val)
{
#ifdef _WITH_I2C
int i2cbus, file, address, res;
int size = I2C_SMBUS_BYTE_DATA;
int pec = 0,force = 0;
char Buf[10], filename[20];
     sprintf(Buf, "%ld",id);
     i2cbus = lookup_i2c_bus(Buf);
     if (i2cbus < 0) messErr(50);
     else {
        sprintf(Buf, "%ld",add);
        address = parse_i2c_address(Buf);
        if (address < 0) messErr(53);
        else {
            if (off < 0 || off > 0xff) messErr(54);
            else {
               if (val > 0xff) messErr(55);
               else {
                  file = open_i2c_dev(i2cbus, filename, sizeof(filename), 0);
                  if (file < 0
                      || check_write_funcs(file, size, pec)
                      || set_slave_addr(file, address, force)) messErr(51);
                  else {
                     res = i2c_smbus_write_byte_data(file, off, val);
                     close(file);
                     if (res < 0) messErr(56);
                  }
               }
            }
        }
     }
#else
    messErr(49);
#endif
}

void IF_I2CWrite (void)
{
long id, add, off, val;
    if (!getParLong(&id)) return;  /* I2C ID */
    if (!getParLong(&add)) return; /* CHIP ADDRESS */
    if (!getParLong(&off)) return; /* DATA ADDRESS (OFFSET) */
    if (!getParLong(&val)) return; /* DATA BYTE */
    i2c_write(id, add, off, val);
    return;
}


