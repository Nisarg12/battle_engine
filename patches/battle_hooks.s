// actualy wild grass hook  
.org 0x0807F76C
    ldr r1, = battle_hook|1
    bx r1
.pool
    
// exit option hook
.org 0x083A7378
    .word init_battle_elements|1
.pool

