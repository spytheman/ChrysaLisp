(import "sys/lisp.inc")
(import "class/lisp.inc")
(import "gui/lisp.inc")
(import "lib/anaphoric/anaphoric.inc")
(import "apps/minefield/board.inc")

(structure '+event 0
	(byte 'close+ 'beginner+ 'intermediate+ 'expert+ 'click+))

(ui-window mywindow ()
	(ui-flow window_flow (:flow_flags +flow_down_fill+)
		(ui-title-bar window_title "Minefield" (0xea19) +event_close+)
		(ui-flow view (:flow_flags +flow_flag_align_hcenter+)
			(ui-flow across (:flow_flags +flow_right_fill+)
					(ui-label left_pad (:text "" :min_width 25))
					(ui-flow center (:flow_flags +flow_down_fill+)
						(ui-label top_pad (:text "" :min_height 15))
						(ui-label game_title (:text "Minefield" :min_width 100 :font (create-font "fonts/OpenSans-Regular.ctf" 20) :flow_flags
								(logior +flow_flag_align_hcenter+ +flow_flag_align_vcenter+)))
						(ui-label _ (:text "" :font *env_window_font*))
						(component-connect (ui-button beginner (:text "Beginner" :min_width 125 :min_height 35)) +event_beginner+)
						(ui-label _ (:text ""))
						(component-connect (ui-button medium (:text "Intermediate" :min_width 125 :min_height 35)) +event_intermediate+)
						(ui-label _ (:text ""))
						(component-connect (ui-button expert (:text "Expert" :min_width 125 :min_height 35)) +event_expert+)
						(ui-label bottom_pad (:text "" :min_height 35)))
					(ui-label right_pad (:text "" :min_width 25))))
		(ui-label status_bar (:text "mouse: 00"))))

(defun clicked-blank (cell)
	(defq work (list cell))
	(while (defq cell (pop work))
		(unless (eql (elem cell game_map) "r")
			(elem-set cell game_map "r")
			(aeach (elem cell game_adj)
				(cond
					((= (elem it game_board) 0) (push work it))
					((< 0 (elem it game_board) 9) (elem-set it game_map "r"))))))
	(rebuild-board))

(defun clicked-flag (cell)
	(elem-set cell game_map "b")
	(rebuild-board))

(defun right-clicked-button (cell)
	(elem-set cell game_map "f")
	(rebuild-board)
	(is-game-over))

(defun clicked-value (cell)
	(elem-set cell game_map "r")
	(rebuild-board))

(defun clicked-mine (cell)
	(elem-set cell game_map "r")
	(rebuild-board))

(defun is-game-over (&optional lost)
	(defq message "")
	(cond 
		(game_over (setq message "You Lost!"))
		((= (length (filter (# (or (eql %0 "f") (eql %0 "b"))) game_map)) (last difficulty))
			(setq message "You Won!" game_over t))
		(t nil))
	(set status_bar :text message)
	(view-dirty status_bar))

(defun colorize (value)
	(elem value '(+argb_black+ 0x000000ff 0x00006600 0x00ff0000 +argb_magenta+ 
		+argb_black+ 0x00700000 +argb_grey1+ 0x0002bbdd +argb_black+)))

(defun board-layout ((gw gh nm))
	(view-sub across)
	(setq game_grid (Grid))
	(defq gwh (* gw gh))
		; (ui-grid game_grid (:grid_width 1 :grid_height 5)
	(each (lambda (_)
		(component-connect (defq mc (Button)) (+ _ +event_click+))
		(def mc :text "" :border 1 :flow_flags 
			(logior +flow_flag_align_hcenter+ +flow_flag_align_vcenter+) :min_width 32 :min_height 32)
		(view-add-child game_grid mc)) (range 0 gwh))
	(def game_grid :grid_width gw :grid_height gh :color (const *env_toolbar_col*) :font *env_window_font*)
	(bind '(w h) (view-pref-size game_grid))
	(view-change game_grid 0 0 w h)
	(def view :min_width w :min_height h)
	(view-add-child view game_grid)
	(bind '(x y w h) (apply view-fit (cat (view-get-pos mywindow) (view-pref-size mywindow))))
	(view-change-dirty mywindow x y w h))

(defun rebuild-board ()
	(bind '(gw gh nm) difficulty)
	(view-sub game_grid)
	(setq game_grid (Grid))
	(defq gwh (* gw gh))
		; (ui-grid game_grid (:grid_width 1 :grid_height 5)
	(each (lambda (_)
		(defq value nil)
		(cond 
			((eql (elem _ game_map) "f")
				(component-connect (defq mc (Button)) (+ +event_click+ _))
				(def mc :text "F" :border 1 :flow_flags 
					(logior +flow_flag_align_hcenter+ +flow_flag_align_vcenter+) :min_width 32 :min_height 32)
				(view-add-child game_grid mc))			
			((eql (elem _ game_map) "b")
				(component-connect (defq mc (Button)) (+ +event_click+ _))
				(def mc :text "" :border 1 :flow_flags 
					(logior +flow_flag_align_hcenter+ +flow_flag_align_vcenter+) :min_width 32 :min_height 32)
				(view-add-child game_grid mc))
			((eql (elem _ game_map) "r")
				(if (< 0 (elem _ game_board) 9) 
					(component-connect (defq mc (Label)) (+ +event_click+ _))
					(defq mc (Label)))
				(def mc :text 
					(cond 
						((= (defq value (elem _ game_board)) 0) "")
						((< 0 value 9) (str value))
						((= value 9) "X"))
					:flow_flags (logior +flow_flag_align_hcenter+ +flow_flag_align_vcenter+) 
					:border 0 :ink_color (colorize value) :color (if (= value 9) +argb_red+ *env_toolbar_col*) :min_width 32 :min_height 32)
				(view-add-child game_grid mc))
			(t nil))) (range 0 gwh))
	(def game_grid :grid_width gw :grid_height gh :color (const *env_toolbar_col*) :font *env_window_font*)
	(bind '(w h) (view-pref-size game_grid))
	(view-change game_grid 0 0 w h)
	(def view :min_width w :min_height h)
	(view-add-child view game_grid)
	(bind '(x y w h) (apply view-fit (cat (view-get-pos mywindow) (view-pref-size mywindow))))
	(view-change-dirty mywindow x y w h))

(defun main ()
	(defq started nil game (list) game_board (list) game_adj (list) game_map (list) 
		first_click t difficulty (list) game_grid nil click_offset 4 game_over nil mouse_down 0)
	(bind '(x y w h) (apply view-locate (view-pref-size mywindow)))
	(gui-add (view-change mywindow x y w h))
	(while (cond
		((= (defq id (get-long (defq msg (mail-read (task-mailbox))) ev_msg_target_id)) +event_close+)
			nil)
		((= id +event_beginner+) (setq started t) (board-layout (setq difficulty beginner_settings)))
		((= id +event_intermediate+) (setq started t) (board-layout (setq difficulty intermediate_settings)))
		((= id +event_expert+) (setq started t) (board-layout (setq difficulty expert_settings)))
		((and started (not game_over)
			(<= +event_click+ id (+ +event_click+ (dec (* (elem 0 difficulty) (elem 1 difficulty))))))
			(defq cid (- id click_offset))
			(bind '(gw gh gn) difficulty)
			(when first_click (setq first_click nil game (create-game gw gh gn cid) game_board (elem 0 game) 
				game_map (elem 1 game) game_adj (elem 2 game)))
			(cond 
				((= mouse_down 1)
					(cond
						((eql (elem cid game_map) "f") (clicked-flag cid))
						((= (elem cid game_board) 9) (clicked-mine cid) (setq game_over t))
						((= (elem cid game_board) 0) (clicked-blank cid))
						((< 0 (elem cid game_board) 9) (clicked-value cid))
						(t nil)))
				((/= mouse_down 1)
					(cond
						((eql (elem cid game_map) "b") (right-clicked-button cid))
						((eql (elem cid game_map) "f") (clicked-flag cid))
						(t nil)))
				(t nil))
			(is-game-over))
		(t
			(and (= (get-long msg (const ev_msg_type)) (const ev_type_mouse))
				(/= 0 (get-int msg (const ev_msg_mouse_buttons)))
				(setq mouse_down (get-int msg (const ev_msg_mouse_buttons))))
			(. mywindow :event msg))))
	(view-hide mywindow))
