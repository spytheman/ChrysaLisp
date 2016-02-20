%include "func.inc"

;;;;;;;;;;;
; test code
;;;;;;;;;;;

	fn_function "tests/test12"
		;pipe task started by test11

		;wait a bit
		vp_cpy 12000000, r0
		fn_call sys/task_sleep

		;read 10 mail commands
		for r14, 0, 10, 1
			fn_call sys/mail_read_mymail
			fn_call sys/mem_free
		next

		;print Hello and return
		vp_lea [rel hello], r0
		vp_cpy 1, r1
		fn_jmp sys/write_string

	hello:
		db "Hello from global worker !", 10, 0

	fn_function_end
