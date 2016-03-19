%include 'inc/func.inc'
%include 'inc/mail.inc'

;;;;;;;;;;;
; test code
;;;;;;;;;;;

	TEST_SIZE equ 1000

	fn_function tests/test12
		;gloabl task started by test11

		;wait a bit
		vp_cpy 1000000, r0
		fn_call sys/math_random
		vp_add 1000000, r0
		static_call task, sleep

		;read 10 mail commands
		for r14, 0, 10, 1
			static_call mail, mymail
			for r15, 0, TEST_SIZE, 1
				if r15, !=, [r0 + (r15 * 8) + ml_msg_data]
					fn_debug Failed to verify data !
					vp_ret
				endif
			next
			static_call mem, free
		next

		fn_debug Hello from global worker !
		vp_ret

	fn_function_end
