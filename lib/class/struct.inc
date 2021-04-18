;;;;;;;;;;;;;;;;;;;;;;;
; structures and fields
;;;;;;;;;;;;;;;;;;;;;;;

(defun structure (type size alignment)
	(setq base (align base alignment))
	(push syms (sym (cat name "_" field)) (sym (cat name "_" field "_t")))
	(push values base type)
	(setq base (+ base size)))

(defun _structure (base line)
	(case (elem 0 line)
		(align
			(if (> (length line) 1)
				(setq base (align base (eval (elem 1 line))))
				(setq base (align base +long_size))))
		(offset
			(push syms (sym (cat name "_" (elem 1 line))))
			(push values base))
		(struct
			(push syms (sym (cat name "_" (elem 1 line))) (sym (cat name "_" (elem 1 line) "_t")))
			(push values base nil)
			(setq base (+ base (eval (elem 2 line)))))
		(union
			(defq union_base base max_base base)
			(each! 1 -1 (lambda (line)
				(setq max_base (max max_base (_structure union_base line)))) (list line))
			(setq base max_base))
		(t	(each! 1 -1 (lambda (field)
				(case (elem 0 line)
					(byte (structure "b" +byte_size +byte_size))
					(ubyte (structure "B" +byte_size +byte_size))
					(short (structure "s" +short_size +short_size))
					(ushort (structure "S" +short_size +short_size))
					(int (structure "i" +int_size +int_size))
					(uint (structure "I" +int_size +int_size))
					(long (structure "l" +long_size +long_size))
					(ulong (structure "L" +long_size +long_size))
					(ptr (structure "p" +ptr_size +ptr_size))
					(pptr (structure "pp" +ptr_size +ptr_size))
					(pubyte (structure "pB" +ptr_size +ptr_size))
					(pushort (structure "pS" +ptr_size +ptr_size))
					(puint (structure "pI" +ptr_size +ptr_size))
					(pulong (structure "pL" +ptr_size +ptr_size))
					(pbyte (structure "pb" +ptr_size +ptr_size))
					(pshort (structure "ps" +ptr_size +ptr_size))
					(pint (structure "pi" +ptr_size +ptr_size))
					(plong (structure "pl" +ptr_size +ptr_size))
					(nodeid (structure "no" +node_id_size +byte_size))
					(netid (structure "ne" +net_id_size +byte_size))))
				(list line)))) base)

(defmacro structure (name base &rest lines)
	; (structure name base [(byte field ...)] ...)
	(if (get (sym (cat name "_size"))) (throw "Structure redefined !" name))
	(defq syms (list) values (list) base (eval base))
	(each (lambda (line)
		(setq base (_structure base line))) lines)
	(push syms (sym (cat name "_size")))
	(push values base)
	`(bind ',syms ',values))

(defmacro getf (s field &optional o)
	; (getf obj field [offset]) -> value
	(defq type (eval (sym (cat field "_t"))) field (eval field) o (opt o +str_data))
	(case (sym type)
		(b `(get-field ,s ,(+ o field) +byte_size))
		(s `(get-field ,s ,(+ o field) +short_size))
		(i `(get-field ,s ,(+ o field) +int_size))
		(l `(get-field ,s ,(+ o field) +long_size))
		(B `(get-field ,s ,(+ o field) ,(neg +byte_size)))
		(S `(get-field ,s ,(+ o field) ,(neg +short_size)))
		(I `(get-field ,s ,(+ o field) ,(neg +int_size)))
		(L `(get-field ,s ,(+ o field) ,(neg +long_size)))
		((p pp pb pB ps pS pi pI pl pL) `(get-field ,s ,(+ o field) 0))
		((no) `(get-field ,s ,(+ o field) +node_id_size))
		((ne) `(get-field ,s ,(+ o field) +net_id_size))))

(defmacro setf (s field v &optional o)
	; (setf obj field value [offset]) -> obj
	(defq type (eval (sym (cat field "_t"))) field (eval field) o (opt o +str_data))
	(case (sym type)
		(b `(set-field ,s ,(+ o field) +byte_size ,v))
		(s `(set-field ,s ,(+ o field) +short_size ,v))
		(i `(set-field ,s ,(+ o field) +int_size ,v))
		(l `(set-field ,s ,(+ o field) +long_size ,v))
		(B `(set-field ,s ,(+ o field) ,(neg +byte_size) ,v))
		(S `(set-field ,s ,(+ o field) ,(neg +short_size) ,v))
		(I `(set-field ,s ,(+ o field) ,(neg +int_size) ,v))
		(L `(set-field ,s ,(+ o field) ,(neg +long_size) ,v))
		((p pp pb pB ps pS pi pI pl pL) `(set-field ,s ,(+ o field) 0 ,v))
		((no) `(set-field ,s ,(+ o field) +node_id_size ,v))
		((ne) `(set-field ,s ,(+ o field) +net_id_size ,v))))

(defmacro setf-> (res &rest frm)
	; (setf-> msg form ...)
	(reduce (lambda (res frm)
		(if (list? frm)
			(insert frm 0 (list 'setf res))
			(list 'setf res frm))) frm res))

;;;;;;;
; enums
;;;;;;;

(defun enums (size)
	(push syms (sym (cat name "_" field)))
	(push values base)
	(setq base (+ base size)))

(defmacro enums (name base &rest lines)
	; (enums name base [(enum field ...)] ...)
	(if (get (sym (cat name "_size"))) (throw "Enums redefined !" name))
	(defq syms (list) values (list) base (eval base))
	(each (lambda (line)
		(each! 1 -1 (lambda (field)
			(case (elem 0 line)
				(enum (enums +byte_size))))
			(list line))) lines)
	(push syms (sym (cat name "_size")))
	(push values base)
	`(bind ',syms ',values))

;;;;;;
; bits
;;;;;;

(defun bits ()
	(push syms (sym (cat name "_" field)))
	(push values (<< 1 base))
	(setq base (inc base)))

(defmacro bits (name base &rest lines)
	; (bits name base [(bit field ...)] ...)
	(if (get (sym (cat name "_size"))) (throw "Bits redefined !" name))
	(defq syms (list) values (list) base (eval base))
	(each (lambda (line)
		(each! 1 -1 (lambda (field)
			(case (elem 0 line)
				(bit (bits))))
			(list line))) lines)
	(push syms (sym (cat name "_size")))
	(push values base)
	`(bind ',syms ',values))