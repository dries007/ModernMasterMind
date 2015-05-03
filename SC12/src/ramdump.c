/*
 * ramdump.c
 *
 *  Created on: 5-mrt.-2015
 *      Author: Dries007
 */

#include "ramdump.h"

void manualram()
{
    char buf[64];
    printf("\nAddress? ");
    gets(buf);
    address addr = strtol(buf, NULL, 0);
    printf("\nValue? ");
    gets(buf);
    byte val = strtol(buf, NULL, 0);

    byte bank = addr >> 8;
    printf("\nWriting 0x%02x to 0x%04x (Bank %d, Real address 0x%04x)\n", val, addr, bank, (addr & 0x0FF) | 0x100);
    pfe_enable_pio(2, bank & 0x01 ? 4 : 5);
    pfe_enable_pio(3, bank & 0x02 ? 4 : 5);
    hal_write_bus((addr & 0x0FF) | 0x100, val, 0xFFFF, 0x0000);
}

void ramdump()
{
    printf("##############################\n");
    printf("########## RAM DUMP ##########\n");
    printf("##############################\n");

    pfe_enable_bus(0xFF, 1);
    pfe_enable_pcs(1);

    for (byte bank = 0; bank < 4; bank++)
    {
        printf("########## BANK %d ##########\n", bank);
        pfe_enable_pio(2, bank & 0x01 ? 4 : 5);
        pfe_enable_pio(3, bank & 0x02 ? 4 : 5);

        printf("     | 0xx0 | 0xx1 | 0xx2 | 0xx3 | 0xx4 | 0xx5 | 0xx6 | 0xx7 | 0xx8 | 0xx9 | 0xxA | 0xxB | 0xxC | 0xxD | 0xxE | 0xxF |\n");
        printf("-----+------+------+------+------+------+------+------+------+------+------+------+------+------+------+------+------+\n");
        address addr = 0x100;
        while (addr < 0x200)
        {
            printf("0x%1xx |", (addr & 0xF0) >> 4);
            do
            {
                printf(" 0x%02x |", hal_read_bus(addr, 0xFFFF, 0x0000));
                addr++;
            }
            while (addr % 0x10 != 0);
            printf("\n");
        }
    }

    printf("##############################\n");
    printf("##########   DONE   ##########\n");
    printf("##############################\n");
}
