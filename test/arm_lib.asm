.global arg_return_test
.global context_switch

arg_return_test:
    @mov r0, #23
    mov r1, #0x0100000
    @mov r4, r0
    str r0, [r1]
    mov r0, #23

    bx lr



