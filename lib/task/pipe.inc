;;;;;;;;;;;;;;;;;;
; pipe task object
;;;;;;;;;;;;;;;;;;

(import "sys/lisp.inc")
(import "class/lisp.inc")

(defclass Pipe (cmds &optional flag) nil
	; (Pipe cmds &optional flag) -> pipe | nil
	(defq cmdpipe (list) args (map (lambda (cmd)
		(defq cmd (split cmd " "))
		(push cmdpipe (cat "cmd/" (elem 0 cmd) ".lisp"))
		(slice 0 -2 (apply cat (map (lambda (_) (cat _ " ")) cmd)))) (split cmds "|")))
	(cond
		;error with pipe element ?
		((some (lambda (_) (= 0 (get-long _ 0))) (defq mboxs (open-pipe cmdpipe)))
			;send abort to any started pipe elements
			(each (lambda (_) (if (/= 0 (get-long _ 0)) (mail-send _ ""))) mboxs)
			(setq this nil))
		(t	;wire up pipe and send args
			(defq select (list) streams (list))
			(each (lambda (_)
				(push streams (in-stream))
				(push select (in-mbox (elem -2 streams)))) mboxs)
			(defq stdout (in-stream) stdout_mbox (in-mbox stdout) ack_mbox (mail-alloc-mbox))
			(each-rev (lambda (mbox arg stderr_mbox)
				(mail-send mbox (setf-> (cat (str-alloc +stdio_msg_init_size) arg)
					(+stdio_msg_init_stdout stdout_mbox)
					(+stdio_msg_init_stderr stderr_mbox)
					(+stdio_msg_init_ack ack_mbox)))
				(setq stdout_mbox (getf (mail-read ack_mbox) +stdio_msg_init_stdout))) mboxs args select)
			(push streams stdout (out-stream stdout_mbox))
			(push select (in-mbox stdout) (if flag (to-net-id (pad "" (const (* +net_id_size 2)) "a")) (task-mailbox)))
			(mail-free-mbox ack_mbox)
			(def this :select select :streams streams :state t)))

	(defmethod :read (this)
		; (. pipe :read) -> nil | t | data
		(cond
			;nil if pipe closed
			((get :state this)
				;pipe is open
				(cond
					;t if task mailbox
					((= (- (defq idx (mail-select (get :select this)))
							(length (get :select this))) -1))
					(t	;must be stdout or one of the stderr's
						(if (/= (in-get-state (in-next-msg
									(defq msg_in (elem idx (get :streams this)))))
								+stream_mail_state_started)
							(def this :state nil))
						(read-avail msg_in))))))

	(defmethod :write (this string)
		; (. pipe :write string) -> pipe
		(stream-flush (write (elem -2 (get :streams this)) string))
		this)

	(defmethod :close (this)
		; (. pipe :close) -> pipe
		;clear the stdin stream, which will send stopping and stopped
		(pop (get :select this))
		(pop (get :streams this))
		;wait for stdout and stderr streams to stop
		(while (/= (length (get :select this)) 0)
			(if (= (in-get-state (in-next-msg (elem (defq idx (mail-select (get :select this))) (get :streams this))))
					+stream_mail_state_stopped)
				(def this
					:streams (erase (get :streams this) idx (inc idx))
					:select (erase (get :select this) idx (inc idx)))))
		this)
	)

(defun Pipe-run (cmdline &optional out)
	; (Pipe-run cmdline &optional outfun)
	(setd out print)
	(defq cmd (Pipe cmdline t))
	(while cmd
		(defq data (. cmd :read))
		(cond
			((eql data nil)
				;pipe is closed
				(. cmd :close)
				(setq cmd nil))
			(t	;string from pipe
				(out data)))))