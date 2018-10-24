#include <stdio.h>
#include <stdint.h>

#include "motherboard.h"
#include "8088.h"

int main(int argc, char *argv[])
{
    struct motherboard *mb = mb_create();
    mb_load_bios_rom(mb, "BIOS_5150_24APR81_U33.BIN");
    mb_powerup(mb);
    mb_run(mb);
}
