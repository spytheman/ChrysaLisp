%include 'inc/func.inc'
%include 'inc/task.inc'

	fn_function sys/task_stack_depth, no_debug_enter
		;outputs
		;r0 = stack depth (in bytes)
		;trashes
		;r0

		class_bind task, statics, r0
		vp_cpy [r0 + tk_statics_current_tcb], r0
		vp_add tk_node_size, r0
		vp_sub r4, r0
		if r0, >, tk_stack_size
			;must be kernel or stack extended
			vp_xor r0, r0
		endif
		vp_ret

	fn_function_end
