;;;;;;;;;;;;;;
; VP Registers
;;;;;;;;;;;;;;

(defq +vp_regs ''(:r0 :r1 :r2 :r3 :r4 :r5 :r6 :r7 :r8 :r9 :r10 :r11 :r12 :r13 :r14 :rsp)
	+vp_regs_0_14 ''(:r0 :r1 :r2 :r3 :r4 :r5 :r6 :r7 :r8 :r9 :r10 :r11 :r12 :r13 :r14))
(defun vp-reg? (_) (find _ +vp_regs))
(each (# (defcvar %0 %0)) +vp_regs)

(defmacro vp-def (_ &optional l)
	(if (> (length (defq v (filter (lambda (_)
			(and (def? _ *compile_env*) (not (find _ *func_syms*)))) _))) 0)
		(throw "VP redefine !" v))
	(setq l (apply cat (map (lambda (x y)
		(list (list 'quote x) y)) _ (if l (merge-obj (cat (eval l)) +vp_regs_0_14) +vp_regs_0_14))))
	`(progn (defcvar ~l) (apply push (cat (list *func_syms*) ',_))))

;;;;;;;;;;;;;
; Emit Buffer
;;;;;;;;;;;;;

(defmacro emit (&rest _)
	`(push *emit_list* ~_))

(defun emit-print (_)
	(unless (eql (elem 0 _) 'lambda) (print _)))

(defun emit-vp-code (emit_list)
	;remove redundant labels and ops, plus rename labels
	(defq out (push (clear '()) 'progn) labels (env 31)
		last_e nil pc 0 pc_sym nil)
	(each (lambda (op)
		(defq e (elem 0 op))
		(unless (eql e 'lambda)
			(cond
				((eql e 'emit-label)
					(bind '(_ (_ label)) op)
					(cond
						((eql e last_e)
							(def labels label pc_sym))
						(t	(def labels label (setq pc_sym (sym (str "_" pc))))
							(push out op)
							(setq pc (inc pc)))))
				((push out op))))
		(setq last_e e)) emit_list)
	(walk-list out
		(lambda (e)
			(and (sym? e) (defq e (def? e labels))
				(elem-set _ _l e)))
		(lambda (l) nil)
		(lambda (l)))
	out)

;;;;;;;;;;;;;;
; VP optimizer
;;;;;;;;;;;;;;

(include "lib/asm/vpopt.inc")

;;;;;;;;;;;;;;;;;
; VP Instructions
;;;;;;;;;;;;;;;;;

(defmacro type-sym (_) `(sym (str ,_ "_t")))

(defun vp-type-t (_)
	(if (setq _ (find-rev (elem 0 _) "bBsSiI"))
		(elem _ '(b ub s us i ui)) ""))

(defun vp-type (o)
	(case (pop (type-of o))
		(:sym
			(cond
				((vp-reg? o) :r)
				((eql (elem 0 o) ":") :k)
				(t :c)))
		(:num :c)
		(:list
			(defq i (elem 0 o))
			(cond
				((vp-reg? i)
					(setq i (elem 1 o))
					(cond
						((vp-reg? i)
							(if (= 2 (length o)) :d
								(sym (cat :d (elem 2 o)))))
						((/= 2 (length o))
							(sym (cat :i (elem 2 o))))
						((num? i) :i)
						((setq i (get (type-sym i)))
							(sym (cat :i (vp-type-t i))))
						(t :i)))
				((eql i '@) :@)
				((eql i '$) :$)
				((eql i '&)
					(if (vp-reg? (elem 1 o))
						(if (vp-reg? (elem 2 o)) :&d :&i) nil))
				(t :c)))
		(:str :s)
		(t nil)))

(defun label-sym (s) (sym (cat "_l_" s)))
(defun vp-label (l) (setq l (label-sym l)) (emit (list 'emit-label (list 'quote l))))
(defun vp-align (a &optional b) (emit (list 'emit-align a b)))
(defun vp-string (s) (emit (list 'emit-string s)))
(defun vp-byte (&rest b) (emit (cat '(emit-byte) b)))
(defun vp-short (&rest b) (emit (cat '(emit-short) b)))
(defun vp-int (&rest b) (emit (cat '(emit-int) b)))
(defun vp-long (&rest b) (emit (cat '(emit-long) b)))
(defun vp-cstr (s) (vp-string s)(vp-byte 0))

(defmacro vp-bcr (_)
	`(defun ,(sym (cat "vp-" _ "-cr")) (c d l)
		(emit (list 'emit-cmp-cr (eval c) (get d))
			(list ',(sym (cat "emit-" _)) (label-sym l)))))
(defmacro vp-brr (_)
	`(defun ,(sym (cat "vp-" _ "-rr")) (s d l)
		(emit (list 'emit-cmp-rr (get s) (get d))
			(list ',(sym (cat "emit-" _)) (label-sym l)))))
(defmacro vp-op-cr (_)
	`(defun ,(sym (cat "vp-" _ "-cr")) (c d)
		(emit (list ',(sym (cat "emit-" _ "-cr")) (eval c) (get d)))))
(defmacro vp-op-rr (_)
	`(defun ,(sym (cat "vp-" _ "-rr")) (s d)
		(emit (list ',(sym (cat "emit-" _ "-rr")) (get s) (get d)))))
(defmacro vp-mem-dr (&optional _)
	(setq _ (if _ (cat "-" _) ""))
	`(defun ,(sym (cat "vp-cpy-dr" _)) (b i d)
		(emit (list ',(sym (cat "emit-cpy-dr" _)) (get b) (get i) (get d)))))
(defmacro vp-mem-rd (&optional _)
	(setq _ (if _ (cat "-" _) ""))
	`(defun ,(sym (cat "vp-cpy-rd" _)) (s b i)
		(emit (list ',(sym (cat "emit-cpy-rd" _)) (get s) (get b) (get i)))))
(defmacro vp-mem-ir (&optional _)
	(setq _ (if _ (cat "-" _) ""))
	`(defun ,(sym (cat "vp-cpy-ir" _)) (b i d)
		(emit (list ',(sym (cat "emit-cpy-ir" _)) (get b) (eval i) (get d)))))
(defmacro vp-mem-ri (&optional _)
	(setq _ (if _ (cat "-" _) ""))
	`(defun ,(sym (cat "vp-cpy-ri" _)) (s b i)
		(emit (list ',(sym (cat "emit-cpy-ri" _)) (get s) (get b) (eval i)))))

(defun vp-push (&rest b) (emit (cat '(emit-push) (map get b))))
(defun vp-pop (&rest b) (emit (cat '(emit-pop) (map get b))))
(defun vp-alloc (c) (emit (list 'emit-alloc (eval c))))
(defun vp-free (c) (emit (list 'emit-free (eval c))))
(defun vp-ret () (emit (list 'emit-ret)))
(defun vp-brk (&optional n) (emit (list 'emit-brk (opt n 0))))
(defun vp-stack-init (s f x) (emit (list 'emit-stack-init (get s) (get f) (get x))))

(defun vp-call-abi (r b i n x) (emit (cat (list 'emit-call-abi (get r) (get b) i n) (map get x))))
(defun vp-call (l) (emit (list 'emit-call (label-sym l))))
(defun vp-call-r (d) (emit (list 'emit-call-r (get d))))
(defun vp-call-i (b i) (emit (list 'emit-call-i (get b) (eval i))))
(defun vp-call-p (l) (emit (list 'emit-call-p (label-sym l))))

(defun vp-jmp (l) (emit (list 'emit-jmp (label-sym l))))
(defun vp-jmp-r (d) (emit (list 'emit-jmp-r (get d))))
(defun vp-jmp-i (b i) (emit (list 'emit-jmp-i (get b) (eval i))))
(defun vp-jmp-p (l) (emit (list 'emit-jmp-p (label-sym l))))

(defun vp-lea-i (b i d) (emit (list 'emit-lea-i (get b) (eval i) (get d))))
(defun vp-lea-d (b i d) (emit (list 'emit-lea-d (get b) i (get d))))
(defun vp-lea-p (l r) (emit (list 'emit-lea-p (label-sym l) (get r))))
(defun vp-cpy-pr (l d) (emit (list 'emit-cpy-pr (label-sym l) (get d))))

(defun vp-div-rrr (d r q) (emit (list 'emit-div-rrr (get d) (get r) (get q))))
(defun vp-div-rrr-u (d r q) (emit (list 'emit-div-rrr-u (get d) (get r) (get q))))

(vp-bcr beq) (vp-bcr bne) (vp-bcr blt) (vp-bcr bgt) (vp-bcr ble) (vp-bcr bge)
(vp-brr beq) (vp-brr bne) (vp-brr blt) (vp-brr bgt) (vp-brr ble) (vp-brr bge)
(vp-op-cr seq) (vp-op-cr sne) (vp-op-cr slt) (vp-op-cr sgt) (vp-op-cr sle) (vp-op-cr sge)
(vp-op-rr seq) (vp-op-rr sne) (vp-op-rr slt) (vp-op-rr sgt) (vp-op-rr sle) (vp-op-rr sge)

(vp-mem-dr b) (vp-mem-dr i) (vp-mem-dr s) (vp-mem-dr ub) (vp-mem-dr ui)
(vp-mem-dr us) (vp-mem-dr) (vp-mem-ir b) (vp-mem-ir i) (vp-mem-ir s)
(vp-mem-ir ub) (vp-mem-ir ui) (vp-mem-ir us) (vp-mem-ir)

(vp-mem-rd b) (vp-mem-rd i) (vp-mem-rd s) (vp-mem-rd) (vp-mem-ri b)
(vp-mem-ri i) (vp-mem-ri s) (vp-mem-ri)

(vp-op-cr cpy) (vp-op-cr add) (vp-op-cr sub) (vp-op-cr mul) (vp-op-cr and)
(vp-op-cr or) (vp-op-cr xor) (vp-op-cr shl) (vp-op-cr shr) (vp-op-cr asr)
(vp-op-rr cpy) (vp-op-rr add) (vp-op-rr sub) (vp-op-rr mul) (vp-op-rr and)
(vp-op-rr or) (vp-op-rr xor) (vp-op-rr shl) (vp-op-rr shr) (vp-op-rr asr)
(vp-op-rr lnot) (vp-op-rr land) (vp-op-rr swp) (vp-op-rr ext)

(defun vp-op4 (op a b c d x y z w) (op a x) (op b y) (op c z) (op d w))
(defun vp-op3 (op a b c x y z) (op a x) (op b y) (op c z))
(defun vp-op2 (op a b x y) (op a x) (op b y))
(defun vp-min (a b) (vpif (list a '< b)) (vp-cpy-rr a b) (endif))
(defun vp-max (a b) (vpif (list a '> b)) (vp-cpy-rr a b) (endif))